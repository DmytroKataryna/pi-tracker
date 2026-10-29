#include "pi_tracker/imu.h"
#include "pi_tracker/serial_port.h"
#include "pi_tracker/tf_luna.h"
#include "pi_tracker/thread_safe_queue.h"
#include "pi_tracker/udp_sender.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include <variant>

#include <pthread.h>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

namespace
{

    struct LidarReading
    {
        Clock::time_point timestamp;
        pi_tracker::TfLunaFrame frame;
    };

    struct SensorError
    {
        std::string source;
        std::string message;
    };

    using SensorMessage = std::variant<pi_tracker::ImuReading, LidarReading, SensorError>;

    template <typename... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };

    // Стежить, щоб дані від датчика приходили не рідше, ніж раз на timeout
    class Watchdog
    {
    public:
        Watchdog(const char *name, Clock::duration timeout) : m_name{name}, m_timeout{timeout}, m_lastKick{Clock::now()}, m_alive{true} {};

        void kick(Clock::time_point timestamp)
        {
            m_lastKick = timestamp;
        };

        void check(Clock::time_point now)
        {
            const bool alive = now - m_lastKick <= m_timeout;
            if (alive != m_alive)
            {
                m_alive = alive;
                std::cerr << std::format("[watchdog] {}: {}\n", m_name, alive ? "restored" : "LOST");
            }
        }

        bool isAlive() const { return m_alive; }

    private:
        const char *m_name;
        Clock::duration m_timeout;
        Clock::time_point m_lastKick;
        bool m_alive{true};
    };

    static_assert(std::atomic<bool>::is_always_lock_free); // інакше використовувати його в обробнику сигналу не можна
    std::atomic<bool> g_stopRequested{false};

    void onSignal(int)
    {
        g_stopRequested = true; // в обробнику сигналу — лише встановити прапорець і нічого більше
    }

    void report(const Watchdog &w)
    {
        if (!w.isAlive())
        {
            std::cerr << "lost\n";
        }
    }

} // namespace

namespace
{
    int run(int argc, char *argv[])
    {
        const char *host = argc > 1 ? argv[1] : "127.0.0.1";
        const auto port = static_cast<std::uint16_t>(argc > 2 ? std::atoi(argv[2]) : 5005);

        std::signal(SIGINT, onSignal);  // Ctrl+C
        std::signal(SIGTERM, onSignal); // kill, systemctl stop

        // Порядок оголошення важливий: потоки йдуть ОСТАННІМИ, тож знищуються (зупиняються) ПЕРШИМИ
        pi_tracker::ThreadSafeQueue<SensorMessage> queue;
        pi_tracker::UdpSender sender{host, port};
        pi_tracker::SerialPort lidarPort{"/dev/ttyAMA0", 115200};

        pi_tracker::Imu imu{"/dev/i2c-1", 0x68, pi_tracker::AccelRange::G4, pi_tracker::GyroRange::Dps500};
        std::cerr << "sensord: calibrating gyro, keep still...\n";
        imu.calibrateGyro(200);

        const auto start = Clock::now();

        std::jthread imuThread{[&](std::stop_token stop)
                               {
                                   pthread_setname_np(pthread_self(), "sd-imu"); // ім'я видно в top -H і gdb (до 15 символів)
                                   try
                                   {
                                       auto next = Clock::now();
                                       while (!stop.stop_requested())
                                       {
                                           queue.push(imu.read());
                                           next += 10ms;
                                           std::this_thread::sleep_until(next);
                                       }
                                   }
                                   catch (const std::exception &e)
                                   {
                                       queue.push(SensorError{"imu", e.what()}); // виняток не може покинути потік — передаємо як повідомлення
                                   }
                               }};

        std::jthread lidarThread{[&](std::stop_token stop)
                                 {
                                     pthread_setname_np(pthread_self(), "sd-lidar");
                                     try
                                     {
                                         pi_tracker::TfLunaParser parser;
                                         std::array<std::uint8_t, 64> buffer{};
                                         while (!stop.stop_requested())
                                         {
                                             const std::size_t n = lidarPort.read(buffer); // чекає не довше 0.1 с
                                             for (std::size_t i = 0; i < n; ++i)
                                             {
                                                 if (auto frame = parser.feed(buffer[i]))
                                                 {
                                                     queue.push(LidarReading{Clock::now(), *frame});
                                                 }
                                             }
                                         }
                                     }
                                     catch (const std::exception &e)
                                     {
                                         queue.push(SensorError{"lidar", e.what()});
                                     }
                                 }};

        auto ms = [start](Clock::time_point t)
        { return std::chrono::duration_cast<std::chrono::milliseconds>(t - start).count(); };

        Watchdog imuWatchdog{"imu", 100ms};
        Watchdog lidarWatchdog{"lidar", 500ms};
        int imuCount = 0;
        int lidarCount = 0;
        int sendErrors = 0;
        std::size_t maxQueue = 0;
        auto nextReport = Clock::now() + 1s;

        std::cerr << std::format("sensord: sending to {}:{}\n", host, port);
        while (!g_stopRequested)
        {
            if (auto message = queue.popFor(100ms))
            {
                const std::string line = std::visit(
                    Overloaded{
                        [&](const pi_tracker::ImuReading &r)
                        {
                            ++imuCount;
                            imuWatchdog.kick(r.timestamp);
                            return std::format("imu,{},{:.3f},{:.3f},{:.3f},{:.2f},{:.2f},{:.2f}\n", ms(r.timestamp),
                                               r.accel[0], r.accel[1], r.accel[2], r.gyro[0], r.gyro[1], r.gyro[2]);
                        },
                        [&](const LidarReading &r)
                        {
                            ++lidarCount;
                            lidarWatchdog.kick(r.timestamp);
                            return std::format("lidar,{},{},{}\n", ms(r.timestamp), r.frame.distanceCm, r.frame.amplitude);
                        },
                        [](const SensorError &e)
                        {
                            std::cerr << std::format("sensord: {} failed: {}\n", e.source, e.message);
                            return std::string{};
                        },
                    },
                    *message);

                if (!line.empty() && !sender.send(line))
                {
                    ++sendErrors;
                }
                maxQueue = std::max(maxQueue, queue.size());
            }

            const auto now = Clock::now();
            imuWatchdog.check(now);
            lidarWatchdog.check(now);
            if (now >= nextReport)
            {
                std::cerr << std::format("imu {}/s, lidar {}/s, send errors {}, queue max {}\n",
                                         imuCount, lidarCount, sendErrors, maxQueue);
                imuCount = lidarCount = sendErrors = 0;
                maxQueue = 0;
                nextReport += 1s;
            }
        }

        std::cerr << "sensord: stopping\n";
        return EXIT_SUCCESS;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        return run(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << "sensord: fatal: " << e.what() << '\n';
        return EXIT_FAILURE; // systemd побачить status=1/FAILURE і перезапустить
    }
}
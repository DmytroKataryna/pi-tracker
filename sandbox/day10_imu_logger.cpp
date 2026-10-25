#include "pi_tracker/imu.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <optional>
#include <thread>

using namespace std::chrono_literals;
using Clock = std::chrono::steady_clock;

int main()
{
    pi_tracker::Imu imu{"/dev/i2c-1", 0x68, pi_tracker::AccelRange::G4, pi_tracker::GyroRange::Dps500};

    // Діагностика → stderr, дані → stdout (пояснення нижче)
    std::cerr << "Calibrating gyro, keep still...\n";
    imu.calibrateGyro(200);
    const auto &bias = imu.gyroBias();
    std::cerr << std::format("gyro bias [°/s]: {:+.3f} {:+.3f} {:+.3f}\n", bias[0], bias[1], bias[2]);

    std::cout << "timestamp_ms,ax,ay,az,gx,gy,gz\n";

    const auto start = Clock::now();
    auto next = start;
    Clock::duration maxReadTime{};
    Clock::duration totalReadTime{};
    std::optional<Clock::time_point> previous;
    double yawDeg = 0.0;
    int count = 0;

    while (Clock::now() - start < 10s)
    {
        next += 10ms;
        std::this_thread::sleep_until(next);

        const auto t0 = Clock::now();
        const pi_tracker::ImuReading r = imu.read();
        const auto readTime = Clock::now() - t0;
        maxReadTime = std::max(maxReadTime, readTime);
        totalReadTime += readTime;
        ++count;

        // Інтегрування: кут += кутова швидкість × час між вимірюваннями
        if (previous)
        {
            const double dt = std::chrono::duration<double>(r.timestamp - *previous).count();
            yawDeg += r.gyro[2] * dt;
        }
        previous = r.timestamp;

        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(r.timestamp - start).count();
        std::cout << std::format("{},{:.4f},{:.4f},{:.4f},{:.3f},{:.3f},{:.3f}\n",
                                 ms, r.accel[0], r.accel[1], r.accel[2], r.gyro[0], r.gyro[1], r.gyro[2]);
    }

    std::cerr << std::format("samples: {}, read avg {} us, max {} us, yaw {:+.2f} deg\n",
                             count,
                             std::chrono::duration_cast<std::chrono::microseconds>(totalReadTime).count() / count,
                             std::chrono::duration_cast<std::chrono::microseconds>(maxReadTime).count(),
                             yawDeg);
}
#include "pi_tracker/ring_buffer.h"
#include "pi_tracker/thread_safe_queue.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <optional>
#include <random>
#include <thread>

using namespace std::chrono_literals; // дозволяє писати 10ms, 3s
using Clock = std::chrono::steady_clock;

struct Sample
{
    Clock::time_point timestamp;
    double accelZ{};
};

double average(const pi_tracker::RingBuffer<double, 10> &buffer)
{
    double sum{};
    for (std::size_t i = 0; i < buffer.size(); ++i)
    {
        sum += buffer.at(i);
    }
    return sum / static_cast<double>(buffer.size());
}

int main()
{
    pi_tracker::ThreadSafeQueue<Sample> queue;

    // «Датчик»: 100 Гц, поки не попросять зупинитись
    std::jthread producer{[&queue](std::stop_token stop)
                          {
                              std::mt19937 rng{42};
                              std::normal_distribution<double> noise{0.0, 0.05};
                              auto next = Clock::now();
                              while (!stop.stop_requested())
                              {
                                  next += 10ms;
                                  std::this_thread::sleep_until(next); // sleep_until, а не sleep_for: похибка не накопичується
                                  queue.push(Sample{Clock::now(), 9.81 + noise(rng)});
                              }
                              queue.close(); // сигнал споживачу: даних більше не буде
                          }};

    // «Обробка»
    std::jthread consumer{[&queue]
                          {
                              pi_tracker::RingBuffer<double, 10> window;
                              int count = 0;
                              std::optional<Clock::time_point> previous;
                              Clock::duration maxJitter{};

                              while (auto sample = queue.pop()) // nullopt → черга закрита → вихід з циклу
                              {
                                  window.push(sample->accelZ);
                                  ++count;
                                  if (previous)
                                  {
                                      const auto period = sample->timestamp - *previous;
                                      maxJitter = std::max(maxJitter, std::chrono::abs(period - 10ms));
                                  }
                                  previous = sample->timestamp;

                                  if (count % 100 == 0)
                                  {
                                      std::cout << "sample " << count << ", avg10 = " << average(window) << '\n';
                                  }
                              }
                              std::cout << "total samples: " << count << '\n';
                              std::cout << "max jitter: "
                                        << std::chrono::duration_cast<std::chrono::microseconds>(maxJitter).count()
                                        << " us\n";
                          }};

    std::this_thread::sleep_for(3s);
    producer.request_stop();
} // jthread-и самі зроблять join у деструкторах

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

constexpr int kIterations = 1'000'000;

template <typename Work>
void runTwoThreads(const std::string &name, Work work)
{
    const auto start = std::chrono::steady_clock::now();
    std::thread t1{work};
    std::thread t2{work};
    t1.join();
    t2.join();
    const auto elapsed = std::chrono::steady_clock::now() - start;
    std::cout << name << ": "
              << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms\n";
}

int main()
{
    int counter = 0;
    runTwoThreads("no sync", [&counter]
                  {
        for (int i = 0; i < kIterations; ++i)
        {
            ++counter;
        } });
    std::cout << "  counter = " << counter << " (expected " << 2 * kIterations << ")\n";

    int lockedCounter = 0;
    std::mutex mutex;
    runTwoThreads("mutex  ", [&lockedCounter, &mutex]
                  {
                      for (int i = 0; i < kIterations; ++i)
                      {
                          std::lock_guard lock{mutex}; 
                          ++lockedCounter;
                      } });
    std::cout << "  counter = " << lockedCounter << " (expected " << 2 * kIterations << ")\n";

    std::atomic<int> atomicCounter = 0;
    runTwoThreads("atomic", [&atomicCounter]
                  {
        for (int i = 0; i < kIterations; ++i)
        {
            ++atomicCounter;
        } });
    std::cout << "  counter = " << atomicCounter << " (expected " << 2 * kIterations << ")\n";
}
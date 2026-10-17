#include <algorithm>
#include <cmath>
#include <iostream>
#include "pi_tracker/ring_buffer.h"

// Середнє значення по буферу будь-якого розміру N
template <std::size_t N>
double average(const pi_tracker::RingBuffer<double, N> &buffer)
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
    // 1. Перевірка логіки: ємність 4, додаємо 1..6
    pi_tracker::RingBuffer<int, 4> rb;
    for (int i = 1; i <= 6; ++i)
    {
        rb.push(i);
    }
    std::cout << "contents (oldest first): ";
    for (std::size_t i = 0; i < rb.size(); ++i)
    {
        std::cout << rb.at(i) << ' ';
    }
    std::cout << '\n';

    // 2. Ковзне середнє по 10 вимірюваннях з даними з Дня 2
    pi_tracker::RingBuffer<double, 10> window;
    double maxRawDev{};
    double maxAvgDev{};
    double v{};
    while (std::cin >> v)
    {
        window.push(v);
        maxRawDev = std::max(maxRawDev, std::abs(v - 9.81));
        if (window.full())
        {
            maxAvgDev = std::max(maxAvgDev, std::abs(average(window) - 9.81));
        }
    }
    std::cout << "max deviation raw:   " << maxRawDev << '\n';
    std::cout << "max deviation avg10: " << maxAvgDev << '\n';
}
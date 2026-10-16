#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <cassert>

// T — тип елементів, Capacity — розмір (відомий під час компіляції)
// Тіла методів шаблону пишуть прямо в класі: компілятор має бачити код, коли підставляє T
template <typename T, std::size_t Capacity>
class RingBuffer
{
public:
    void push(T value)
    {
        m_data[m_head] = std::move(value);
        m_head = (m_head + 1) % Capacity;
        if (!full())
        {
            m_size = m_size + 1;
        }
    }

    std::size_t size() const { return m_size; }
    bool full() const { return m_size == Capacity; }

    const T &at(std::size_t i) const
    {
        assert(i < m_size && "RingBuffer::at: index out of range");
        const std::size_t oldest = (m_head + Capacity - m_size) % Capacity;
        return m_data[(oldest + i) % Capacity];
    }

private:
    std::array<T, Capacity> m_data{};
    std::size_t m_head{}; // індекс, куди запишеться НАСТУПНИЙ елемент
    std::size_t m_size{}; // скільки елементів зараз у буфері
};

// Середнє значення по буферу будь-якого розміру N
template <std::size_t N>
double average(const RingBuffer<double, N> &buffer)
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
    RingBuffer<int, 4> rb;
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
    RingBuffer<double, 10> window;
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
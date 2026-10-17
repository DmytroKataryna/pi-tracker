#pragma once

#include <array>     // std::array
#include <cassert>   // assert
#include <cstddef>   // std::size_t
#include <utility>   // std::move

namespace pi_tracker
{
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
}
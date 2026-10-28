#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>
#include <limits>
#include <stdexcept>
#include <chrono>
#include <cstddef>

namespace pi_tracker
{

    // Потокобезпечна черга: одні потоки кладуть (push), інші забирають (pop)
    template <typename T>
    class ThreadSafeQueue
    {
    public:
        explicit ThreadSafeQueue(const std::size_t capacity) : m_capacity{capacity}
        {
            if (m_capacity == 0)
            {
                throw std::invalid_argument("ThreadSafeQueue: capacity must be positive");
            }
        };

        explicit ThreadSafeQueue() : m_capacity{std::numeric_limits<std::size_t>::max()} {};

        // push
        void push(T value)
        {
            {
                std::lock_guard lock{m_mutex}; // RAII: розблокується на закриваючій }

                if (m_queue.size() >= m_capacity)
                {
                    ++m_dropped;
                    m_queue.pop();
                }

                m_queue.push(std::move(value));
            }
            m_cv.notify_one(); // будимо одного споживача (вже без блокування)
        }

        std::optional<T> pop()
        {
            std::unique_lock lock{m_mutex};

            m_cv.wait(lock,
                      [this]
                      { return !m_queue.empty() || m_closed; });

            return takeFront();
        }

        std::optional<T> popFor(std::chrono::milliseconds timeout)
        {
            std::unique_lock l{m_mutex};

            m_cv.wait_for(l, timeout, [this]
                          { return !m_queue.empty() || m_closed; });

            return takeFront();
        }

        void close()
        {
            {
                std::lock_guard lock{m_mutex};
                m_closed = true;
            }
            m_cv.notify_all();
        }

        std::size_t size() const
        {
            std::lock_guard l{m_mutex};
            return m_queue.size();
        }

        std::size_t dropped() const
        {
            std::lock_guard l{m_mutex};
            return m_dropped;
        }

    private:
        mutable std::mutex m_mutex;
        std::condition_variable m_cv;
        std::queue<T> m_queue;
        bool m_closed{false};
        std::size_t m_dropped{};
        std::size_t m_capacity{};

        // Викликати лише під захопленим m_mutex
        std::optional<T> takeFront()
        {
            if (m_queue.empty())
            {
                return std::nullopt;
            }
            T value = std::move(m_queue.front());
            m_queue.pop();
            return value;
        }
    };

}
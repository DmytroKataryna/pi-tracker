#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace pi_tracker
{

    // Потокобезпечна черга: одні потоки кладуть (push), інші забирають (pop)
    template <typename T>
    class ThreadSafeQueue
    {
    public:
        // push
        void push(T value)
        {
            {
                std::lock_guard lock{m_mutex}; // RAII: розблокується на закриваючій }
                m_queue.push(std::move(value));
            }
            m_cv.notify_one(); // будимо одного споживача (вже без блокування)
        }

        std::optional<T> pop()
        {
            std::unique_lock lock{m_mutex};

            m_cv.wait(lock, [this]
                      { return !m_queue.empty() || m_closed; });

            if (m_queue.empty())
            {
                return std::nullopt;
            }

            T value = std::move(m_queue.front());
            m_queue.pop();
            return value;
        }

        void close()
        {
            {
                std::lock_guard lock{m_mutex};
                m_closed = true;
            }
            m_cv.notify_all();
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::queue<T> m_queue;
        bool m_closed{false};
    };

}
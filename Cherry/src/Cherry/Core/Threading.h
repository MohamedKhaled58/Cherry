
// Cherry/src/Cherry/Core/Threading.h - Fixed version
#pragma once
#include "CHpch.h"
#include <thread>
#include <future>
#include <queue>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <type_traits>

namespace Cherry {
    class ThreadPool {
    public:
        ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
        ~ThreadPool();

        template<typename F, typename... Args>
        auto Submit(F&& f, Args&&... args)
            -> std::future<typename std::invoke_result<F, Args...>::type> {

            using return_type = typename std::invoke_result<F, Args...>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> result = task->get_future();

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                if (m_Stop) {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }

                m_Tasks.emplace([task]() { (*task)(); });
            }

            m_Condition.notify_one();
            return result;
        }

    private:
        std::vector<std::thread> m_Workers;
        std::queue<std::function<void()>> m_Tasks;

        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;
        bool m_Stop;
    };

    template<typename T>
    class ThreadSafeQueue {
    public:
        ThreadSafeQueue() = default;

        void Push(T item) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Queue.push(std::move(item));
            m_Condition.notify_one();
        }

        bool TryPop(T& item) {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Queue.empty()) {
                return false;
            }
            item = std::move(m_Queue.front());
            m_Queue.pop();
            return true;
        }

        void WaitAndPop(T& item) {
            std::unique_lock<std::mutex> lock(m_Mutex);
            while (m_Queue.empty()) {
                m_Condition.wait(lock);
            }
            item = std::move(m_Queue.front());
            m_Queue.pop();
        }

        bool Empty() const {
            std::lock_guard<std::mutex> lock(m_Mutex);
            return m_Queue.empty();
        }

    private:
        mutable std::mutex m_Mutex;
        std::queue<T> m_Queue;
        std::condition_variable m_Condition;
    };
}
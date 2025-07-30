#pragma once
#include <thread>
#include <future>
#include <queue>
namespace Cherry {
    class ThreadPool {
    public:
        ThreadPool(size_t numThreads = std::thread::hardware_concurrency());

        template<typename F, typename... Args>
        auto Submit(F&& f, Args&&... args)
            -> std::future<typename std::result_of<F(Args...)>::type>;

    private:
        std::vector<std::thread> m_Workers;
        ThreadSafeQueue<std::function<void()>> m_Tasks;
    };

    template<typename T>
    class ThreadSafeQueue {
    public:
        void Push(T item);
        bool TryPop(T& item);
        void WaitAndPop(T& item);
        bool Empty() const;

    private:
        mutable std::mutex m_Mutex;
        std::queue<T> m_Queue;
        std::condition_variable m_Condition;
    };
}
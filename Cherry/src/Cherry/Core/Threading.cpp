#include "CHpch.h"
#include "Threading.h"

namespace Cherry {

    ThreadPool::ThreadPool(size_t numThreads) : m_Stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            m_Workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                        this->m_Condition.wait(lock, [this] {
                            return this->m_Stop || !this->m_Tasks.empty();
                            });

                        if (this->m_Stop && this->m_Tasks.empty()) {
                            return;
                        }

                        task = std::move(this->m_Tasks.front());
                        this->m_Tasks.pop();
                    }

                    task();
                }
                });
        }
    }

    ThreadPool::~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }

        m_Condition.notify_all();

        for (std::thread& worker : m_Workers) {
            worker.join();
        }
    }

} // namespace Cherry
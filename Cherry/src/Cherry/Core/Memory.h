#pragma once

namespace Cherry {
    template<typename T>
    class ObjectPool {
    public:
        template<typename... Args>
        T* Acquire(Args&&... args);
        void Release(T* object);

    private:
        std::stack<std::unique_ptr<T>> m_Available;
        std::vector<std::unique_ptr<T>> m_Pool;
    };

    class MemoryProfiler {
    public:
        static void BeginFrame();
        static void EndFrame();
        static MemoryStats GetStats();

    private:
        static thread_local size_t s_AllocationsThisFrame;
        static thread_local size_t s_DeallocationsThisFrame;
    };
}
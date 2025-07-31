#pragma once
#include <stack>
#include <vector>
#include <memory>
#include <cstddef>

namespace Cherry {
    // Memory statistics structure
    struct MemoryStats {
        size_t TotalAllocations = 0;
        size_t TotalDeallocations = 0;
        size_t CurrentAllocations = 0;
        size_t PeakAllocations = 0;
        size_t TotalBytesAllocated = 0;
        size_t CurrentBytesAllocated = 0;
        size_t PeakBytesAllocated = 0;
    };

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
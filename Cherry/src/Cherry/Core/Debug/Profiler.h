// Cherry/src/Cherry/Debug/Profiler.h
#pragma once
#include "Cherry/Core/Core.h"
#include <chrono>
#include <string>

namespace Cherry {

    struct ProfileResult {
        std::string Name;
        float Time;
    };

    struct FrameData {
        std::vector<ProfileResult> Results;
        float TotalTime;
    };

    class ScopedTimer {
    public:
        ScopedTimer(const std::string& name) : m_Name(name) {
            m_StartTime = std::chrono::high_resolution_clock::now();
        }

        ~ScopedTimer() {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime);
            float ms = duration.count() * 0.001f;
            // Store result somewhere if needed
        }

    private:
        std::string m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
    };

    class Profiler {
    public:
        static void BeginSample(const std::string& name) {}
        static void EndSample() {}
        static FrameData GetLastFrameData() { return {}; }
    };
}

// Profiling macros
#ifdef CH_PROFILE
#define CH_PROFILE_SCOPE(name) auto timer_##__LINE__ = Cherry::ScopedTimer(name)
#define CH_PROFILE_FUNCTION() CH_PROFILE_SCOPE(__FUNCTION__)
#else
#define CH_PROFILE_SCOPE(name)
#define CH_PROFILE_FUNCTION()
#endif
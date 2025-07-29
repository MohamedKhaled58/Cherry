#pragma once

#include "Cherry/Core/Debug/Profiler.h"

#ifdef CH_PLATFORM_WINDOWS

extern Cherry::Application* Cherry::CreateApplication();

int main(int argc, char** argv)
{
    // Initialize logging system
    Cherry::Log::Init();

    // Initialize profiler (always enabled in debug builds)
#ifdef CH_PROFILE
    CH_CORE_INFO("Profiler initialized - Real-time profiling enabled");
    Cherry::Profiler::Get().SetEnabled(true);
#endif

    // Create application instance
    {
        CH_PROFILE_SCOPE("Application Creation");
        auto app = Cherry::CreateApplication();

        // Run the main application loop
        {
            CH_PROFILE_SCOPE("Application Runtime");
            app->Run();
        }

        // Clean shutdown
        {
            CH_PROFILE_SCOPE("Application Shutdown");
            delete app;
        }
    }

#ifdef CH_PROFILE
    CH_CORE_INFO("Application terminated - Profiler data available in ImGui");
#endif

    return 0;
}

#endif // CH_PLATFORM_WINDOWS
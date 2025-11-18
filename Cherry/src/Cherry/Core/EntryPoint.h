#pragma once
#include "Cherry/Debug/Instrumentor.h"

#ifdef CH_PLATFORM_WINDOWS
extern Cherry::Application* Cherry::CreateApplication();

int main(int argc, char** argv)
{
    // Initialize logging system
    Cherry::Log::Init();

	CH_PROFILE_BEGIN_SESSION("Startup", "CherryProfile-Startup.jason");
	auto app = Cherry::CreateApplication();
	CH_PROFILE_END_SESSION();

	CH_PROFILE_BEGIN_SESSION("Runtime", "CherryProfile-Runtime.jason");
	app->Run();
	CH_PROFILE_END_SESSION();

	CH_PROFILE_BEGIN_SESSION("Shutdown", "CherryProfile-Shutdown.jason");
	delete app;
	CH_PROFILE_END_SESSION();

    return 0;
}
#endif // CH_PLATFORM_WINDOWS
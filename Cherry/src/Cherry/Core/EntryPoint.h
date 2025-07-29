#pragma once
#ifdef CH_PLATFORM_WINDOWS

extern Cherry::Application* Cherry::CreateApplication();

int main(int argc, char** argv)
{
    // Initialize logging system
    Cherry::Log::Init();

        auto app = Cherry::CreateApplication();

            app->Run();

            delete app;

    return 0;
}

#endif // CH_PLATFORM_WINDOWS
// MyShell/src/MMOApplication.cpp - Updated main application
#include "Cherry.h"
#include "Cherry/Core/EntryPoint.h"
#include "MMOGameLayer.h"

class MMOApplication : public Cherry::Application {
public:
    MMOApplication() {
        // Initialize all systems
        Cherry::EnhancedPackageManager::Get().Initialize();
        Cherry::EnhancedAnimationSystem::Get().Initialize();
        Cherry::SpriteManager::Get().Initialize();

        // Add the main game layer
        PushLayer(new Cherry::MMOGameLayer());
    }

    ~MMOApplication() {
        // Cleanup systems
        Cherry::SpriteManager::Get().Shutdown();
        Cherry::EnhancedAnimationSystem::Get().Shutdown();
        Cherry::EnhancedPackageManager::Get().Shutdown();
    }
};

Cherry::Application* Cherry::CreateApplication() {
    CH_CLIENT_INFO("Creating MMO Application");
    return new MMOApplication();
}
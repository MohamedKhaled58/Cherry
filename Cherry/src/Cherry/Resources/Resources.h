#pragma once
#include <future>
#include <Cherry/Core/Core.h>
#include <Cherry/Core/Threading.h>
#include "Cherry/Resources/ResourceManager.h"

namespace Cherry {
    class ResourceManager {
    public:
        static ResourceManager& Get();

        // Modern resource loading with async support
        template<typename T>
        std::future<REF(T)> LoadAsync(const std::string& path);

        template<typename T>
        REF(T) Load(const std::string& path);

        // Package file support (.wdf, .dnp equivalents)
        bool LoadPackage(const std::string& packagePath);
        void UnloadPackage(const std::string& packagePath);

        // Hot-reload support
        void EnableHotReload(bool enable = true);

    private:
        std::unordered_map<std::string, std::unique_ptr<PackageFile>> m_Packages;
        std::unordered_map<std::string, std::weak_ptr<Resource>> m_ResourceCache;
        ThreadPool m_LoadingThreadPool;
    };
}
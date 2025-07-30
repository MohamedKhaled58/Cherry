#pragma once
#include <filesystem>
#include <unordered_map>
namespace Cherry {
    class AssetManager {
    public:
        // Asset importing with metadata
        AssetHandle ImportAsset(const std::filesystem::path& path);

        // Asset dependencies
        std::vector<AssetHandle> GetDependencies(AssetHandle handle);

        // Asset hot-reloading
        void ReloadAsset(AssetHandle handle);

        // Memory management
        void UnloadUnusedAssets();
        size_t GetMemoryUsage() const;

    private:
        std::unordered_map<AssetHandle, AssetMetadata> m_AssetRegistry;
        std::unordered_map<AssetHandle, std::unique_ptr<Asset>> m_LoadedAssets;
    };
}
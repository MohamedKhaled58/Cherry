#pragma once
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace Cherry {
    // Asset system type definitions
    using AssetHandle = uint64_t;
    static constexpr AssetHandle INVALID_ASSET_HANDLE = 0;

    enum class AssetType {
        None = 0,
        Texture2D,
        Model,
        Audio,
        Shader,
        Scene,
        Script,
        Material
    };

    struct AssetMetadata {
        AssetHandle Handle = INVALID_ASSET_HANDLE;
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;
        std::string Name;
        size_t FileSize = 0;
        uint64_t LastModified = 0;
        bool IsLoaded = false;
    };

    // Base class for all assets
    class Asset {
    public:
        virtual ~Asset() = default;
        virtual AssetType GetType() const = 0;
        virtual const std::string& GetName() const = 0;
        virtual bool IsValid() const = 0;

        AssetHandle GetHandle() const { return m_Handle; }

    protected:
        AssetHandle m_Handle = INVALID_ASSET_HANDLE;
        friend class AssetManager;
    };

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
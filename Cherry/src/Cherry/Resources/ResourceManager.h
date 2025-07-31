#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Core/Threading.h"
#include "PackageManager.h"
#include <future>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

namespace Cherry {

    // Forward declarations
    class Resource;
    class FileWatcher;

    enum class ResourceType {
        Unknown = 0,
        Texture,
        Audio,
        Shader,
        Animation,
        Model,
        Font,
        Config
    };

    struct ResourceMetadata {
        std::string Path;
        ResourceType Type;
        size_t FileSize;
        std::filesystem::file_time_type LastModified;
        std::vector<std::string> Dependencies;
        bool IsPackaged = false;
        std::string PackageName;
    };

    class Resource {
    public:
        Resource(const std::string& path, ResourceType type)
            : m_Path(path), m_Type(type), m_RefCount(0) {
        }

        virtual ~Resource() = default;

        const std::string& GetPath() const { return m_Path; }
        ResourceType GetType() const { return m_Type; }

        void AddRef() { ++m_RefCount; }
        void Release() { --m_RefCount; }
        uint32_t GetRefCount() const { return m_RefCount; }

        virtual bool Load() = 0;
        virtual void Unload() = 0;
        virtual size_t GetMemoryUsage() const = 0;

    protected:
        std::string m_Path;
        ResourceType m_Type;
        std::atomic<uint32_t> m_RefCount;
    };

    // PackageFile is now defined in PackageManager.h

    class ResourceManager {
    public:
        static ResourceManager& Get() {
            static ResourceManager instance;
            return instance;
        }

        // Delete copy constructor and assignment operator
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

        // Initialization
        void Initialize();
        void Shutdown();

        // Synchronous loading
        template<typename T>
        REF(T) Load(const std::string& path) {
            static_assert(std::is_base_of_v<Resource, T>, "T must inherit from Resource");

            auto it = m_ResourceCache.find(path);
            if (it != m_ResourceCache.end()) {
                if (auto resource = it->second.lock()) {
                    resource->AddRef();
                    return std::static_pointer_cast<T>(resource);
                }
                else {
                    m_ResourceCache.erase(it);
                }
            }

            auto resource = LoadResourceInternal<T>(path);
            if (resource && resource->Load()) {
                resource->AddRef();
                m_ResourceCache[path] = resource;
                return resource;
            }

            return nullptr;
        }

        // Asynchronous loading
        template<typename T>
        std::future<REF(T)> LoadAsync(const std::string& path) {
            return m_ThreadPool.Submit([this, path]() {
                return Load<T>(path);
                });
        }

        // Package management
        bool LoadPackage(const std::string& packagePath);
        void UnloadPackage(const std::string& packagePath);
        bool IsPackageLoaded(const std::string& packagePath) const;

        // Resource management
        void UnloadResource(const std::string& path);
        void UnloadUnusedResources();
        void ReloadResource(const std::string& path);

        // Hot reloading
        void EnableHotReload(bool enable = true);
        void CheckForChangedFiles();

        // Statistics
        size_t GetLoadedResourceCount() const;
        size_t GetTotalMemoryUsage() const;
        ResourceMetadata GetResourceMetadata(const std::string& path) const;

        // Resource discovery
        std::vector<std::string> FindResourcesOfType(ResourceType type) const;
        std::vector<std::string> GetResourcesInDirectory(const std::string& directory) const;

    private:
        ResourceManager() = default;
        ~ResourceManager() = default;

        template<typename T>
        REF(T) LoadResourceInternal(const std::string& path);

        std::vector<uint8_t> ReadFileData(const std::string& path);
        ResourceType DetermineResourceType(const std::string& path) const;

        // Resource cache with weak references for automatic cleanup
        std::unordered_map<std::string, std::weak_ptr<Resource>> m_ResourceCache;

        // Package files
        std::unordered_map<std::string, std::unique_ptr<PackageFile>> m_PackageFiles;

        // Thread pool for async loading
        ThreadPool m_ThreadPool;

        // Hot reload system
        std::unique_ptr<FileWatcher> m_FileWatcher;
        bool m_HotReloadEnabled = false;

        // Resource metadata cache
        std::unordered_map<std::string, ResourceMetadata> m_MetadataCache;

        // Thread safety
        mutable std::shared_mutex m_CacheMutex;
        mutable std::mutex m_PackageMutex;
    };

    // Resource factory registration
    template<typename T>
    class ResourceFactory {
    public:
        static REF(T) Create(const std::string& path, const std::vector<uint8_t>& data);
    };

    // Macro for easy resource factory registration
#define REGISTER_RESOURCE_FACTORY(ResourceType, Factory) \
        template<> \
        REF(ResourceType) ResourceFactory<ResourceType>::Create(const std::string& path, const std::vector<uint8_t>& data) { \
            return Factory::Create(path, data); \
        }

    // RAII Resource handle for automatic cleanup
    template<typename T>
    class ResourceHandle {
    public:
        ResourceHandle() = default;

        ResourceHandle(REF(T) resource) : m_Resource(resource) {
            if (m_Resource) m_Resource->AddRef();
        }

        ResourceHandle(const ResourceHandle& other) : m_Resource(other.m_Resource) {
            if (m_Resource) m_Resource->AddRef();
        }

        ResourceHandle& operator=(const ResourceHandle& other) {
            if (this != &other) {
                if (m_Resource) m_Resource->Release();
                m_Resource = other.m_Resource;
                if (m_Resource) m_Resource->AddRef();
            }
            return *this;
        }

        ResourceHandle(ResourceHandle&& other) noexcept : m_Resource(std::move(other.m_Resource)) {
            other.m_Resource = nullptr;
        }

        ResourceHandle& operator=(ResourceHandle&& other) noexcept {
            if (this != &other) {
                if (m_Resource) m_Resource->Release();
                m_Resource = std::move(other.m_Resource);
                other.m_Resource = nullptr;
            }
            return *this;
        }

        ~ResourceHandle() {
            if (m_Resource) m_Resource->Release();
        }

        T* Get() const { return m_Resource.get(); }
        T* operator->() const { return m_Resource.get(); }
        T& operator*() const { return *m_Resource; }
        bool IsValid() const { return m_Resource != nullptr; }

        void Reset() {
            if (m_Resource) {
                m_Resource->Release();
                m_Resource = nullptr;
            }
        }

    private:
        REF(T) m_Resource;
    };

} // namespace Cherry
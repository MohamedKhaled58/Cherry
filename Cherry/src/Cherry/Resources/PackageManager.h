#pragma once
#include "Cherry/Core/Core.h"
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <memory>
#include <future>
#include <queue>
#include <condition_variable>

namespace Cherry {

    // Forward declarations
    class FileWatcher;

    // String hashing utility (compatible with C3Engine)
    class StringHasher {
    public:
        static uint32_t StringToID(const char* str);
        static uint32_t GenerateID(const std::string& str);
        static uint32_t PackName(const std::string& filename);
        static uint32_t RealName(const std::string& filename);
    };

    // Base package file structures
    struct PackageFileHeader {
        char Version[8] = "PKG_1.0";    // Package version
        uint32_t FileCount = 0;         // Number of files
        uint32_t IndexOffset = 0;       // Offset to file index
        uint32_t Reserved[13] = { 0 };  // Reserved for future use
    };

    struct PackageFileEntry {
        uint32_t UID;           // String hash ID
        uint32_t Offset;        // File offset in package
        uint32_t Size;          // File size
        uint32_t IsCompressed;  // Compression flag
        uint32_t Reserved[4] = { 0 };
    };

    // Base PackageFile class
    class PackageFile {
    public:
        PackageFile();
        virtual ~PackageFile();

        virtual bool Open(const std::string& filename);
        virtual void Close();
        virtual void* LoadFile(uint32_t fileID, uint32_t& size);
        virtual bool FileExists(uint32_t fileID) const;

        bool IsOpen() const { return m_FileHandle != nullptr; }
        const std::string& GetPackagePath() const { return m_PackagePath; }
        uint32_t GetPackageID() const { return m_PackageID; }

    protected:
        virtual bool ReadHeader();
        virtual bool ReadIndex();
        PackageFileEntry* FindFile(uint32_t fileID);

        std::FILE* m_FileHandle;
        std::string m_PackagePath;
        uint32_t m_PackageID;
        PackageFileHeader m_Header;
        std::unordered_map<uint32_t, PackageFileEntry> m_FileIndex;
        mutable std::mutex m_AccessMutex;
    };

    // Base PackageManager class
    class PackageManager {
    public:
        static PackageManager& Get() {
            static PackageManager instance;
            return instance;
        }

        virtual ~PackageManager() = default;

        virtual bool OpenPackage(const std::string& packagePath);
        virtual void ClosePackage(const std::string& packagePath);
        virtual void CloseAllPackages();
        virtual void* LoadFile(const std::string& filename, uint32_t& size);
        virtual bool FileExists(const std::string& filename) const;

        // Cache management
        void SetCacheSize(size_t maxSize) { m_MaxCacheSize = maxSize; }
        size_t GetCacheSize() const { return m_CurrentCacheSize; }

        // Utility methods
        void BeforeUse();
        void AfterUse();
        size_t GetLoadedPackageCount() const;
        size_t GetTotalMemoryUsage() const;
        void PrintPackageInfo() const;

    protected:
        PackageManager() = default;
        PackageFile* FindPackageContaining(uint32_t fileID);

        static constexpr size_t MAX_PACKAGES = 16;
        std::vector<std::unique_ptr<PackageFile>> m_Packages;
        mutable std::mutex m_PackagesMutex;

        // Simple file cache
        std::unordered_map<std::string, std::vector<uint8_t>> m_FileCache;
        size_t m_CurrentCacheSize = 0;
        size_t m_MaxCacheSize = 64 * 1024 * 1024; // 64MB default
    };

    // Enhanced package format similar to C3Engine's WDF/DNP
    struct PackageFileHeader_V2 {
        char Signature[9] = "CHRYPKG2";  // Cherry Package v2 (need 9 chars for null terminator)
        uint32_t Version = 0x00020000;
        uint32_t FileCount = 0;
        uint32_t IndexOffset = 0;
        uint32_t Flags = 0;
        uint8_t Reserved[39] = { 0 };  // Adjusted to maintain struct size
    };

    struct PackageFileEntry_V2 {
        uint32_t UID;           // String hash ID
        uint32_t Offset;        // File offset in package
        uint32_t Size;          // Uncompressed size
        uint32_t CompressedSize; // Compressed size (0 if not compressed)
        uint32_t CRC32;         // File checksum
        uint32_t Flags;         // Compression type, encryption, etc.
        uint64_t Timestamp;     // File modification time
        uint8_t Reserved[8] = { 0 };
    };

    // Enhanced package file with async loading and streaming
    class EnhancedPackageFile : public PackageFile {
    public:
        EnhancedPackageFile(const std::string& packagePath);
        ~EnhancedPackageFile() override;

        // Async operations
        std::future<std::vector<uint8_t>> ReadFileAsync(const std::string& relativePath);
        std::future<bool> PreloadFilesAsync(const std::vector<std::string>& filePaths);

        // Streaming support for large files
        class FileStream {
        public:
            FileStream(EnhancedPackageFile* package, const std::string& filePath);
            ~FileStream();

            size_t Read(void* buffer, size_t size);
            bool Seek(size_t offset, int origin = SEEK_SET);
            size_t Tell() const;
            size_t Size() const;
            bool IsEOF() const;

        private:
            EnhancedPackageFile* m_Package;
            std::string m_FilePath;
            PackageFileEntry_V2 m_Entry;
            size_t m_Position = 0;
            mutable std::mutex m_StreamMutex;
        };

        std::unique_ptr<FileStream> CreateStream(const std::string& relativePath);

        // Memory mapping for large packages
        void EnableMemoryMapping(bool enable = true);
        bool IsMemoryMapped() const { return m_MemoryMapped; }

        // File system watching for development
        void EnableHotReload(bool enable = true);

    protected:
        bool ReadHeaderV2();
        bool ReadIndexV2();
        std::vector<uint8_t> DecompressFile(const PackageFileEntry_V2& entry, const std::vector<uint8_t>& data);

    private:
        bool m_MemoryMapped = false;
        void* m_MappedMemory = nullptr;
        size_t m_MappedSize = 0;

        // Async support
        std::thread m_BackgroundThread;
        std::atomic<bool> m_BackgroundRunning = false;
        mutable std::shared_mutex m_IndexMutex;

        // Hot reload
        std::unique_ptr<class FileWatcher> m_FileWatcher;
        std::atomic<bool> m_HotReloadEnabled = false;

        std::unordered_map<uint32_t, PackageFileEntry_V2> m_IndexV2;
    };

    // Enhanced package manager with better caching and performance
    class EnhancedPackageManager : public PackageManager {
    public:
        static EnhancedPackageManager& Get() {
            static EnhancedPackageManager instance;
            return instance;
        }

        // Enhanced loading with priority system
        enum class LoadPriority {
            Critical = 0,   // UI, essential game files
            High = 1,       // Player assets, immediate gameplay
            Medium = 2,     // Environment, NPCs
            Low = 3,        // Background music, optional content
            Background = 4  // Preloading, caching
        };

        std::future<void*> LoadFileAsync(const std::string& filename, LoadPriority priority = LoadPriority::Medium);
        void SetCachePolicy(size_t maxMemory, float retentionTime = 300.0f); // 5 minutes default

        // Batch loading for level transitions
        class BatchLoader {
        public:
            BatchLoader(EnhancedPackageManager* manager);
            ~BatchLoader();

            void AddFile(const std::string& filename, LoadPriority priority = LoadPriority::Medium);
            void AddDirectory(const std::string& directory, LoadPriority priority = LoadPriority::Medium);

            std::future<bool> Execute();
            float GetProgress() const;
            void Cancel();

        private:
            EnhancedPackageManager* m_Manager;
            std::vector<std::pair<std::string, LoadPriority>> m_Files;
            std::atomic<float> m_Progress = 0.0f;
            std::atomic<bool> m_Cancelled = false;
        };

        std::unique_ptr<BatchLoader> CreateBatchLoader();

        // Package management (inherits from base + enhanced features)
        bool LoadPackage(const std::string& packagePath) { return OpenPackage(packagePath); }
        void UnloadPackage(const std::string& packagePath) { ClosePackage(packagePath); }

        // Enhanced initialization
        void Initialize() {
            // Initialize base package manager
            m_ShutdownRequested = false;
            
            // Start worker threads
            SetWorkerThreadCount(std::thread::hardware_concurrency());
            
            // Start cache maintenance
            std::thread([this]() { CacheMaintenanceThread(); }).detach();
        }

        void Shutdown() {
            // Signal shutdown
            m_ShutdownRequested = true;
            m_QueueCondition.notify_all();
            
            // Wait for worker threads
            for (auto& thread : m_WorkerThreads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            
            // Close all packages
            CloseAllPackages();
        }

        // Statistics and monitoring
        struct PackageStats {
            size_t TotalPackages = 0;
            size_t TotalFiles = 0;
            size_t CacheHits = 0;
            size_t CacheMisses = 0;
            size_t MemoryUsed = 0;
            size_t MemoryLimit = 0;
            float AverageLoadTime = 0.0f;
        };

        PackageStats GetStatistics() const;
        void ResetStatistics();

        // Thread pool for async operations
        void SetWorkerThreadCount(size_t count);

    public:
        // Allow unique_ptr to call destructor
        ~EnhancedPackageManager() = default;
        
    private:
        EnhancedPackageManager() = default;

        // Priority queue for loading requests
        struct LoadRequest {
            std::string Filename;
            LoadPriority Priority;
            std::promise<void*> Promise;
            std::chrono::steady_clock::time_point RequestTime;

            bool operator<(const LoadRequest& other) const {
                return Priority > other.Priority; // Higher priority = lower number
            }
        };

        std::priority_queue<LoadRequest> m_LoadQueue;
        std::mutex m_QueueMutex;
        std::condition_variable m_QueueCondition;

        // Worker threads
        std::vector<std::thread> m_WorkerThreads;
        std::atomic<bool> m_ShutdownRequested = false;

        void WorkerThreadFunction();

        // Enhanced caching with LRU eviction
        struct CacheEntry {
            std::vector<uint8_t> Data;
            std::chrono::steady_clock::time_point LastAccess;
            std::chrono::steady_clock::time_point LoadTime;
            size_t AccessCount = 0;
        };

        std::unordered_map<std::string, CacheEntry> m_EnhancedCache;
        size_t m_MaxCacheMemory = 256 * 1024 * 1024; // 256MB default
        float m_CacheRetentionTime = 300.0f; // 5 minutes
        mutable std::shared_mutex m_CacheMutex;

        void EvictOldCacheEntries();
        void CacheMaintenanceThread();

        // Statistics
        mutable std::mutex m_StatsMutex;
        PackageStats m_Stats;
    };

    // Global instance declaration
    extern PackageManager& g_PackageManager;

    // Utility functions for compatibility with C3Engine
    namespace C3Compat {
        // String ID generation (compatible with C3Engine's algorithm)
        uint32_t GenerateStringID(const char* str);
        uint32_t PackName(const char* filename);
        uint32_t RealName(const char* filename);

        // Direct compatibility functions
        void* DataFileLoad(const char* filename, uint32_t& size);
        bool DataFileOpen(const char* filename);
        void DataFileClose();
    }

} // namespace Cherry
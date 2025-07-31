#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Resources/PackageManager.h"
#include <thread>
#include <atomic>
#include <shared_mutex>

namespace Cherry {

    // Enhanced package format similar to C3Engine's WDF/DNP
    struct PackageFileHeader_V2 {
        char Signature[8] = "CHRYPKG2";  // Cherry Package v2
        uint32_t Version = 0x00020000;
        uint32_t FileCount = 0;
        uint32_t IndexOffset = 0;
        uint32_t Flags = 0;
        uint8_t Reserved[40] = { 0 };
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

    private:
        EnhancedPackageManager() = default;
        ~EnhancedPackageManager() = default;

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
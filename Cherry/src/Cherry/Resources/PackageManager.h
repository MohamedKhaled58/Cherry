#pragma once
#include "Cherry/Core/Core.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <mutex>

namespace Cherry {

    // Forward declarations
    struct PackageFileEntry;
    class PackageFile;

    // C3Engine-style string ID generation
    class StringHasher {
    public:
        static uint32_t GenerateID(const std::string& str);
        static uint32_t PackName(const std::string& filename);
        static uint32_t RealName(const std::string& filename);

    private:
        static uint32_t StringToID(const char* str);
    };

    // Package file entry (similar to C3DataFileIndex)
    struct PackageFileEntry {
        uint32_t UID;
        uint32_t Offset;
        uint32_t Size;
        uint32_t CompressedSize;
        bool IsCompressed;

        PackageFileEntry()
            : UID(0), Offset(0), Size(0), CompressedSize(0), IsCompressed(false) {
        }

        PackageFileEntry(uint32_t uid, uint32_t offset, uint32_t size, uint32_t compSize = 0)
            : UID(uid), Offset(offset), Size(size), CompressedSize(compSize), IsCompressed(compSize > 0) {
        }
    };

    // Package file header (similar to C3DataFileHeader)
    struct PackageFileHeader {
        uint32_t ID;
        uint32_t FileCount;
        uint32_t IndexOffset;
        char Version[16];

        PackageFileHeader() : ID(0), FileCount(0), IndexOffset(0) {
            memset(Version, 0, sizeof(Version));
            strcpy_s(Version, "PKG_1.0");
        }
    };

    // Individual package file manager
    class PackageFile {
    public:
        PackageFile();
        ~PackageFile();

        bool Open(const std::string& filename);
        void Close();
        bool IsOpen() const { return m_FileHandle != nullptr; }

        void* LoadFile(uint32_t fileID, uint32_t& size);
        bool FileExists(uint32_t fileID) const;

        uint32_t GetPackageID() const { return m_PackageID; }
        const std::string& GetPackagePath() const { return m_PackagePath; }

    private:
        bool ReadHeader();
        bool ReadIndex();
        PackageFileEntry* FindFile(uint32_t fileID);
        std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& compressedData);

    private:
        std::string m_PackagePath;
        FILE* m_FileHandle;
        uint32_t m_PackageID;
        PackageFileHeader m_Header;
        std::unordered_map<uint32_t, PackageFileEntry> m_FileIndex;
        mutable std::mutex m_AccessMutex;
    };

    // Main package manager (singleton, C3Engine style)
    class PackageManager {
    public:
        static PackageManager& Get() {
            static PackageManager instance;
            return instance;
        }

        // Core functionality (similar to MyDataFile.h interface)
        bool OpenPackage(const std::string& packagePath);
        void ClosePackage(const std::string& packagePath);
        void CloseAllPackages();

        void* LoadFile(const std::string& filename, uint32_t& size);
        bool FileExists(const std::string& filename) const;

        // C3Engine compatibility
        bool OpenDataFile(const std::string& filename) { return OpenPackage(filename); }
        void* DataFileLoad(const std::string& filename, uint32_t& size) { return LoadFile(filename, size); }
        void DataFileClose() { CloseAllPackages(); }

        // Utility functions
        void BeforeUse();
        void AfterUse();

        // Statistics
        size_t GetLoadedPackageCount() const;
        size_t GetTotalMemoryUsage() const;
        void PrintPackageInfo() const;

    private:
        PackageManager() = default;
        ~PackageManager() { CloseAllPackages(); }

        // Prevent copying
        PackageManager(const PackageManager&) = delete;
        PackageManager& operator=(const PackageManager&) = delete;

        PackageFile* FindPackageContaining(uint32_t fileID);

    private:
        static constexpr size_t MAX_PACKAGES = 16;
        std::vector<std::unique_ptr<PackageFile>> m_Packages;
        mutable std::mutex m_PackagesMutex;

        // Memory management
        std::unordered_map<std::string, std::vector<uint8_t>> m_FileCache;
        size_t m_MaxCacheSize = 64 * 1024 * 1024; // 64MB cache
        size_t m_CurrentCacheSize = 0;
    };

    // Global access (C3Engine style)
    extern PackageManager& g_PackageManager;

} // namespace Cherry
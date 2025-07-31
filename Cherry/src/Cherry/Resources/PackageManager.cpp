#include "CHpch.h"
#include "PackageManager.h"
#include <algorithm>
#include <cstring>

namespace Cherry {

    // Global instance
    PackageManager& g_PackageManager = PackageManager::Get();

    // String hashing implementation (from C3Engine)
    uint32_t StringHasher::StringToID(const char* str) {
        int i;
        unsigned int v;
        static unsigned m[70];
        strncpy_s((char*)m, sizeof(m), str, 256);

        for (i = 0; i < 256 / 4 && m[i]; i++);
        m[i++] = 0x9BE74448;
        m[i++] = 0x66F42C48;
        v = 0xF4FA8928;

        uint32_t esi = 0x37A8470E; // x0
        uint32_t edi = 0x7758B42B; // y0
        uint32_t ecx = 0;

        while (ecx < (uint32_t)i) {
            uint32_t ebx = 0x267B0B11; // w
            v = (v << 1) | (v >> 31);   // rol v, 1
            uint32_t eax = m[ecx];
            ebx ^= v;
            esi ^= eax;
            edi ^= eax;

            uint32_t edx = ebx + edi;
            edx |= 0x2040801;          // a
            edx &= 0xBFEF7FDF;         // c

            eax = esi;
            uint64_t result = (uint64_t)eax * edx;
            eax = (uint32_t)result;
            edx = (uint32_t)(result >> 32);
            eax += edx;
            if (result & 0x100000000ULL) eax++;

            esi = eax;
            edx = ebx + esi;
            edx |= 0x804021;           // b
            edx &= 0x7DFEFBFF;         // d

            eax = edi;
            result = (uint64_t)eax * edx;
            edx = (uint32_t)(result >> 32);
            eax = (uint32_t)result;
            edx += edx;
            eax += edx;
            if (result & 0x80000000ULL) eax += 2;

            edi = eax;
            ecx++;
        }

        return esi ^ edi;
    }

    uint32_t StringHasher::GenerateID(const std::string& str) {
        std::string buffer(str);

        // Convert to lowercase and normalize path separators
        std::transform(buffer.begin(), buffer.end(), buffer.begin(), [](unsigned char c) {
            if (std::isupper(c)) return static_cast<char>(std::tolower(c));
            if (c == '\\') return '/';
            return static_cast<char>(c);
            });

        return StringToID(buffer.c_str());
    }

    uint32_t StringHasher::PackName(const std::string& filename) {
        std::string buffer;

        for (char c : filename) {
            if (c == '/') {
                buffer += ".pkg"; // Changed from .wdf to .pkg
                break;
            }
            if (c >= 'A' && c <= 'Z') {
                buffer += (c + 'a' - 'A');
            }
            else {
                buffer += c;
            }
        }

        return buffer.empty() ? 0 : StringToID(buffer.c_str());
    }

    uint32_t StringHasher::RealName(const std::string& filename) {
        return GenerateID(filename);
    }

    // PackageFile implementation
    PackageFile::PackageFile()
        : m_FileHandle(nullptr), m_PackageID(0) {
    }

    PackageFile::~PackageFile() {
        Close();
    }

    bool PackageFile::Open(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_AccessMutex);

        if (IsOpen()) {
            Close();
        }

        errno_t err = fopen_s(&m_FileHandle, filename.c_str(), "rb");
        if (err != 0 || !m_FileHandle) {
            CH_CORE_ERROR("Failed to open package file: {}", filename);
            return false;
        }

        m_PackagePath = filename;
        m_PackageID = StringHasher::GenerateID(filename);

        if (!ReadHeader() || !ReadIndex()) {
            Close();
            return false;
        }

        CH_CORE_INFO("Opened package: {} (ID: {}, Files: {})",
            filename, m_PackageID, m_Header.FileCount);
        return true;
    }

    void PackageFile::Close() {
        std::lock_guard<std::mutex> lock(m_AccessMutex);

        if (m_FileHandle) {
            fclose(m_FileHandle);
            m_FileHandle = nullptr;
        }

        m_FileIndex.clear();
        m_PackageID = 0;
        m_PackagePath.clear();
    }

    bool PackageFile::ReadHeader() {
        if (!m_FileHandle) return false;

        fseek(m_FileHandle, 0, SEEK_SET);
        size_t bytesRead = fread(&m_Header, sizeof(PackageFileHeader), 1, m_FileHandle);

        if (bytesRead != 1) {
            CH_CORE_ERROR("Failed to read package header from: {}", m_PackagePath);
            return false;
        }

        // Validate header
        if (strncmp(m_Header.Version, "PKG_1.0", 7) != 0) {
            CH_CORE_WARN("Package version mismatch in: {}", m_PackagePath);
            // Continue anyway - might be compatible
        }

        return true;
    }

    bool PackageFile::ReadIndex() {
        if (!m_FileHandle || m_Header.FileCount == 0) return false;

        fseek(m_FileHandle, m_Header.IndexOffset, SEEK_SET);

        for (uint32_t i = 0; i < m_Header.FileCount; ++i) {
            PackageFileEntry entry;
            size_t bytesRead = fread(&entry, sizeof(PackageFileEntry), 1, m_FileHandle);

            if (bytesRead != 1) {
                CH_CORE_ERROR("Failed to read file entry {} from: {}", i, m_PackagePath);
                return false;
            }

            m_FileIndex[entry.UID] = entry;
        }

        return true;
    }

    void* PackageFile::LoadFile(uint32_t fileID, uint32_t& size) {
        std::lock_guard<std::mutex> lock(m_AccessMutex);

        PackageFileEntry* entry = FindFile(fileID);
        if (!entry) {
            size = 0;
            return nullptr;
        }

        // Seek to file data
        fseek(m_FileHandle, entry->Offset, SEEK_SET);

        // Allocate buffer
        void* buffer = malloc(entry->Size);
        if (!buffer) {
            CH_CORE_ERROR("Failed to allocate {} bytes for file ID: {}", entry->Size, fileID);
            size = 0;
            return nullptr;
        }

        // Read data
        size_t bytesRead = fread(buffer, 1, entry->Size, m_FileHandle);
        if (bytesRead != entry->Size) {
            CH_CORE_ERROR("Failed to read file data for ID: {} (expected: {}, got: {})",
                fileID, entry->Size, bytesRead);
            free(buffer);
            size = 0;
            return nullptr;
        }

        // TODO: Implement decompression if entry->IsCompressed

        size = entry->Size;
        return buffer;
    }

    bool PackageFile::FileExists(uint32_t fileID) const {
        std::lock_guard<std::mutex> lock(m_AccessMutex);
        return m_FileIndex.find(fileID) != m_FileIndex.end();
    }

    PackageFileEntry* PackageFile::FindFile(uint32_t fileID) {
        auto it = m_FileIndex.find(fileID);
        return (it != m_FileIndex.end()) ? &it->second : nullptr;
    }

    // PackageManager implementation
    bool PackageManager::OpenPackage(const std::string& packagePath) {
        std::lock_guard<std::mutex> lock(m_PackagesMutex);

        // Check if already open
        for (const auto& pkg : m_Packages) {
            if (pkg && pkg->GetPackagePath() == packagePath) {
                CH_CORE_WARN("Package already open: {}", packagePath);
                return true;
            }
        }

        // Find empty slot or create new one
        std::unique_ptr<PackageFile> newPackage = std::make_unique<PackageFile>();
        if (!newPackage->Open(packagePath)) {
            return false;
        }

        // Remove oldest if we're at capacity
        if (m_Packages.size() >= MAX_PACKAGES) {
            CH_CORE_WARN("Maximum packages reached, closing oldest");
            m_Packages.erase(m_Packages.begin());
        }

        m_Packages.push_back(std::move(newPackage));
        return true;
    }

    void PackageManager::ClosePackage(const std::string& packagePath) {
        std::lock_guard<std::mutex> lock(m_PackagesMutex);

        auto it = std::find_if(m_Packages.begin(), m_Packages.end(),
            [&packagePath](const std::unique_ptr<PackageFile>& pkg) {
                return pkg && pkg->GetPackagePath() == packagePath;
            });

        if (it != m_Packages.end()) {
            m_Packages.erase(it);
            CH_CORE_INFO("Closed package: {}", packagePath);
        }
    }

    void PackageManager::CloseAllPackages() {
        std::lock_guard<std::mutex> lock(m_PackagesMutex);
        m_Packages.clear();
        m_FileCache.clear();
        m_CurrentCacheSize = 0;
        CH_CORE_INFO("Closed all packages and cleared cache");
    }

    void* PackageManager::LoadFile(const std::string& filename, uint32_t& size) {
        // Generate file IDs
        uint32_t packageID = StringHasher::PackName(filename);
        uint32_t fileID = StringHasher::RealName(filename);

        // Check cache first
        auto cacheIt = m_FileCache.find(filename);
        if (cacheIt != m_FileCache.end()) {
            size = static_cast<uint32_t>(cacheIt->second.size());
            void* buffer = malloc(size);
            if (buffer) {
                memcpy(buffer, cacheIt->second.data(), size);
                return buffer;
            }
        }

        // Find package containing the file
        PackageFile* package = FindPackageContaining(fileID);
        if (!package) {
            CH_CORE_WARN("File not found in any package: {}", filename);
            size = 0;
            return nullptr;
        }

        // Load from package
        void* data = package->LoadFile(fileID, size);
        if (data && size > 0) {
            // Add to cache if size is reasonable
            if (size <= 1024 * 1024 && m_CurrentCacheSize + size <= m_MaxCacheSize) {
                std::vector<uint8_t> cacheData(static_cast<uint8_t*>(data),
                    static_cast<uint8_t*>(data) + size);
                m_FileCache[filename] = std::move(cacheData);
                m_CurrentCacheSize += size;
            }
        }

        return data;
    }

    bool PackageManager::FileExists(const std::string& filename) const {
        uint32_t packageID = StringHasher::PackName(filename);
        uint32_t fileID = StringHasher::RealName(filename);

        // Check cache
        if (m_FileCache.find(filename) != m_FileCache.end()) {
            return true;
        }

        // Check packages
        std::lock_guard<std::mutex> lock(m_PackagesMutex);
        for (const auto& pkg : m_Packages) {
            if (pkg && pkg->IsOpen() && pkg->FileExists(fileID)) {
                return true;
            }
        }

        return false;
    }

    PackageFile* PackageManager::FindPackageContaining(uint32_t fileID) {
        std::lock_guard<std::mutex> lock(m_PackagesMutex);

        for (const auto& pkg : m_Packages) {
            if (pkg && pkg->IsOpen() && pkg->FileExists(fileID)) {
                return pkg.get();
            }
        }

        return nullptr;
    }

    void PackageManager::BeforeUse() {
        // Initialize any required resources
        CH_CORE_TRACE("PackageManager ready for use");
    }

    void PackageManager::AfterUse() {
        // Cleanup temporary resources but keep packages open
        // Clear cache if memory pressure is high
        if (m_CurrentCacheSize > m_MaxCacheSize * 0.8f) {
            m_FileCache.clear();
            m_CurrentCacheSize = 0;
            CH_CORE_INFO("Cleared package cache due to memory pressure");
        }
    }

    size_t PackageManager::GetLoadedPackageCount() const {
        std::lock_guard<std::mutex> lock(m_PackagesMutex);
        return m_Packages.size();
    }

    size_t PackageManager::GetTotalMemoryUsage() const {
        return m_CurrentCacheSize;
    }

    void PackageManager::PrintPackageInfo() const {
        std::lock_guard<std::mutex> lock(m_PackagesMutex);

        CH_CORE_INFO("=== Package Manager Status ===");
        CH_CORE_INFO("Loaded packages: {}/{}", m_Packages.size(), MAX_PACKAGES);
        CH_CORE_INFO("Cache size: {:.2f} MB / {:.2f} MB",
            m_CurrentCacheSize / (1024.0f * 1024.0f),
            m_MaxCacheSize / (1024.0f * 1024.0f));
        CH_CORE_INFO("Cached files: {}", m_FileCache.size());

        for (const auto& pkg : m_Packages) {
            if (pkg && pkg->IsOpen()) {
                CH_CORE_INFO("  Package: {} (ID: {})",
                    pkg->GetPackagePath(), pkg->GetPackageID());
            }
        }
    }

} // namespace Cherry
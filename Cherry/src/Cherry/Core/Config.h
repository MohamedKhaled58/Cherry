#pragma once
#include <string>
#include <filesystem>
#include "rapidjson/document.h"

namespace Cherry {
    
    // Simple FileWatcher class for configuration file monitoring
    class FileWatcher {
    public:
        FileWatcher() = default;
        ~FileWatcher() = default;
        
        void StartWatching(const std::string& filepath) {
            m_FilePath = filepath;
            m_IsWatching = true;
        }
        
        void StopWatching() {
            m_IsWatching = false;
        }
        
        bool HasChanged() const {
            if (!m_IsWatching || m_FilePath.empty()) return false;
            
            if (std::filesystem::exists(m_FilePath)) {
                auto lastModified = std::filesystem::last_write_time(m_FilePath);
                if (lastModified != m_LastCheck) {
                    m_LastCheck = lastModified;
                    return true;
                }
            }
            return false;
        }
        
    private:
        std::string m_FilePath;
        bool m_IsWatching = false;
        mutable std::filesystem::file_time_type m_LastCheck;
    };

    class ConfigManager {
    public:
        // Type-safe configuration access
        template<typename T>
        T Get(const std::string& key, const T& defaultValue = T{});

        template<typename T>
        void Set(const std::string& key, const T& value);

        // File operations
        bool LoadFromFile(const std::string& filePath);
        bool SaveToFile(const std::string& filePath);

        // Live reloading for development
        void EnableLiveReload(const std::string& configPath);

        // Validation
        bool ValidateConfig(const std::string& schemaPath);

    private:
        rapidjson::Document m_Config;
        std::filesystem::file_time_type m_LastModified;
        std::unique_ptr<FileWatcher> m_FileWatcher;
    };
}
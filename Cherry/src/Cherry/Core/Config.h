#pragma once
#include <string>
#include <filesystem>

namespace Cherry {
    class ConfigManager {
    public:
        // Type-safe configuration access
        template<typename T>
        T Get(const std::string& key, const T& defaultValue = T{});

        template<typename T>
        void Set(const std::string& key, const T& value);

        // Live reloading for development
        void EnableLiveReload(const std::string& configPath);

        // Validation
        bool ValidateConfig(const std::string& schemaPath);

    private:
        nlohmann::json m_Config;
        std::filesystem::file_time_type m_LastModified;
        FileWatcher m_FileWatcher;
    };
}
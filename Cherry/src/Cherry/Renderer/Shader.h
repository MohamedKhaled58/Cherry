#pragma once
#include <string>
#include <Cherry/Core.h>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <filesystem>

namespace Cherry {

    class Shader {
    public:
        virtual ~Shader() = default;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;
        virtual std::string GetName() const = 0;

        static REF(Shader) Create(const std::string& filepath);
        static REF(Shader) Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);

        static std::string ExtractShaderNameFromPath(const std::string& filepath);

    };

    class ShaderLibrary {
    public:
        ShaderLibrary() = default;
        ~ShaderLibrary() = default;

        // Non-copyable, movable
        ShaderLibrary(const ShaderLibrary&) = delete;
        ShaderLibrary& operator=(const ShaderLibrary&) = delete;
        ShaderLibrary(ShaderLibrary&&) = default;
        ShaderLibrary& operator=(ShaderLibrary&&) = default;

        // Shader management
        void Add(const std::string& name,const REF(Shader)& shader);
        void Add(const REF(Shader)& shader);
        REF(Shader) Load(const std::string& filepath);
        REF(Shader) Load(const std::string& shaderName,const std::string& filepath);
        REF(Shader) Get(const std::string& shaderName);

        bool Exists(const std::string& shaderName) const;
        void Remove(const std::string& shaderName);
        void Clear();
        std::vector<std::string> GetShaderNames() const;
        void ReloadShader(const std::string& shaderName);
        void ReloadAll();

    private:
        // 1. Core shader storage (you already have this)
        std::unordered_map<std::string, REF(Shader)> m_Shaders;

        // 2. REQUIRED: File paths for reload functionality
        // Maps shader name -> original file path for hot-reloading
        std::unordered_map<std::string, std::string> m_ShaderPaths;

        // 3. OPTIONAL BUT RECOMMENDED: Hot-reload support
        // Track file modification times for automatic reload detection
        std::unordered_map<std::string, std::filesystem::file_time_type> m_FileTimestamps;

        // 4. OPTIONAL: Hot-reload configuration
        bool m_HotReloadEnabled = false;

        // 5. OPTIONAL: Performance tracking
        // Track load times and compilation statistics
        struct ShaderMetrics {
            std::chrono::milliseconds loadTime{ 0 };
            std::chrono::milliseconds compileTime{ 0 };
            size_t reloadCount = 0;
            bool lastCompilationSuccess = false;
        };
        std::unordered_map<std::string, ShaderMetrics> m_ShaderMetrics;

        // 6. OPTIONAL: Error tracking
        // Store compilation errors for debugging
        std::unordered_map<std::string, std::string> m_LastErrors;

        // 7. OPTIONAL: Dependency tracking
        // Track which shaders depend on which include files
        std::unordered_map<std::string, std::vector<std::string>> m_ShaderDependencies;
        std::unordered_map<std::string, std::vector<std::string>> m_IncludeDependents;


    };
}
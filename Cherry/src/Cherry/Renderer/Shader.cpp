#include "CHpch.h"
#include "Shader.h"
#include "Cherry/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Cherry {

	REF(Shader) Shader::Create(const std::string& filepath)
	{

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); 
				return nullptr;
			case RendererAPI::API::OpenGL:	return std::make_shared<OpenGLShader>(filepath);
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
		return nullptr;

	}

	REF(Shader) Shader::Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:	CH_CLIENT_ASSERT(false, "RendererAPI::None is not Supported!"); return nullptr;
		case RendererAPI::API::OpenGL:	return std::make_shared<OpenGLShader>(name,vertexSource, fragmentSource);
		}

		CH_CLIENT_ASSERT(false, "UnKnown RendererAPI !");
		return nullptr;

	}

	void ShaderLibrary::Add(const std::string& name, const REF(Shader)& shader)
	{
		// Check for existing shader with same name
		if (m_Shaders.find(name) != m_Shaders.end()) {
			CH_CORE_WARN("Shader '{}' already exists in library. Replacing existing shader.", name);
		}

		// Add shader to library
		m_Shaders[name] = shader;
		CH_CORE_INFO("Added shader '{}' to library", name);
	}

	void ShaderLibrary::Add(const REF(Shader)& shader)
	{
		const std::string& shaderName = shader->GetName();
		if (shaderName.empty()) {
			CH_CORE_WARN("Attempted to add shader with empty name to library");
			return;
		}

		// Check for existing shader with same name
		if (m_Shaders.find(shaderName) != m_Shaders.end()) {
			CH_CORE_WARN("Shader '{}' already exists in library. Replacing existing shader.", shaderName);
		}

		// Add shader to library
		Add(shaderName, shader);
		CH_CORE_INFO("Added shader '{}' to library", shaderName);
	}

	REF(Shader) ShaderLibrary::Load(const std::string& shaderName, const std::string& filepath)
	{
		// Input validation
		if (shaderName.empty()) {
			CH_CORE_ERROR("Shader name cannot be empty");
			return nullptr;
		}

		if (filepath.empty()) {
			CH_CORE_ERROR("Shader filepath cannot be empty");
			return nullptr;
		}

		// Check if file exists
		if (!std::filesystem::exists(filepath)) {
			CH_CORE_ERROR("Shader file does not exist: '{}'", filepath);
			return nullptr;
		}

		// Check if shader already exists in library
		auto existingShader = m_Shaders.find(shaderName);
		if (existingShader != m_Shaders.end()) {
			CH_CORE_WARN("Shader '{}' already exists in library. Reloading from '{}'",
				shaderName, filepath);
		}

		// Create shader from file
		auto shader = Shader::Create(filepath);
		if (!shader) {
			CH_CORE_ERROR("Failed to create shader '{}' from file '{}'", shaderName, filepath);
			return nullptr;
		}

		// Store filepath for hot-reloading
		m_ShaderPaths[shaderName] = filepath;

		// ? FIX: Use the explicit name parameter, not shader's internal name
		Add(shaderName, shader);

		CH_CORE_INFO("Successfully loaded shader '{}' from '{}'", shaderName, filepath);
		return shader;
	}

	REF(Shader) ShaderLibrary::Load(const std::string& filepath)
	{
		auto shader = Shader::Create(filepath);
		if (!shader) {
			CH_CORE_ERROR("Failed to create shader from file '{}'", filepath);
			return nullptr;
		}

		std::string shaderName = Shader::ExtractShaderNameFromPath(filepath);
		if (shaderName.empty()) {
			CH_CORE_ERROR("Failed to extract shader name from filepath: '{}'", filepath);
			return nullptr;
		}

		// Use the explicit Add method for consistency
		Add(shaderName, shader);
		m_ShaderPaths[shaderName] = filepath;

		return shader;
	}
	

	REF(Shader) ShaderLibrary::Get(const std::string& shaderName)
	{
		if (shaderName.empty()) {
			CH_CORE_WARN("Attempted to get shader with empty name");
			return nullptr;
		}

		auto it = m_Shaders.find(shaderName);
		if (it == m_Shaders.end()) {
			CH_CORE_WARN("Shader '{}' not found in library", shaderName);
			return nullptr;
		}

		const auto& shader = it->second;
		if (!shader) {
			CH_CORE_WARN("Shader '{}' reference is null, removing from library", shaderName);
			m_Shaders.erase(it);
			m_ShaderPaths.erase(shaderName);
			return nullptr;
		}

		return shader;
	}


	std::string Shader::ExtractShaderNameFromPath(const std::string& filepath)
	{
		std::filesystem::path path(filepath);
		std::string filename = path.filename().string();

		// Remove all extensions like ".vert.glsl"
		while (path.has_extension()) {
			path = path.stem();
		}
		filename = path.string();

		// Known suffixes
		const std::vector<std::string> suffixes = {
			"_vert", "_frag", "_geom", "_comp", "_tesc", "_tese",
			".vert", ".frag", ".geom", ".comp", ".tesc", ".tese",
			"_vs", "_fs", "_gs", "_cs"
		};

		// Strip suffix
		for (const auto& suffix : suffixes)
		{
			if (filename.size() >= suffix.size() &&
				filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0)
			{
				filename = filename.substr(0, filename.size() - suffix.size());
				break;
			}
		}

		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
		return filename;
	}

	
	bool ShaderLibrary::Exists(const std::string& shaderName) const
	{
		return m_Shaders.find(shaderName) != m_Shaders.end();
	}


	void ShaderLibrary::Remove(const std::string& shaderName)
	{
		auto it = m_Shaders.find(shaderName);
		if (it != m_Shaders.end()) {
			CH_CORE_INFO("Removing shader '{}' from library", shaderName);
			m_Shaders.erase(it);
			m_ShaderPaths.erase(shaderName);
		}
	}

	void ShaderLibrary::Clear()
	{
		CH_CORE_INFO("Clearing shader library ({} shaders)", m_Shaders.size());
		m_Shaders.clear();
		m_ShaderPaths.clear();
	}

	std::vector<std::string> ShaderLibrary::GetShaderNames() const
	{
		std::vector<std::string> names;
		names.reserve(m_Shaders.size());

		for (const auto& [name, shader] : m_Shaders) {
			names.push_back(name);
		}

		std::sort(names.begin(), names.end());
		return names;
	}

	void ShaderLibrary::ReloadShader(const std::string& shaderName)
	{
		auto pathIt = m_ShaderPaths.find(shaderName);
		if (pathIt == m_ShaderPaths.end()) {
			CH_CORE_WARN("Cannot reload shader '{}': no source path stored", shaderName);
			return;
		}

		std::string filepath = pathIt->second;
		std::string name = shaderName; // Copy for Load method signature

		CH_CORE_INFO("Reloading shader '{}' from '{}'", shaderName, filepath);

		auto reloadedShader = Load(name, filepath);
		if (!reloadedShader) {
			CH_CORE_ERROR("Failed to reload shader '{}'", shaderName);
		}
	}

	void ShaderLibrary::ReloadAll()
	{
		CH_CORE_INFO("Reloading all shaders in library ({} shaders)", m_ShaderPaths.size());

		// Create a copy of shader names to avoid iterator invalidation
		std::vector<std::string> shaderNames;
		for (const auto& [name, path] : m_ShaderPaths) {
			shaderNames.push_back(name);
		}

		size_t successCount = 0;
		for (const auto& name : shaderNames) {
			try {
				ReloadShader(name);
				successCount++;
			}
			catch (const std::exception& e) {
				CH_CORE_ERROR("Exception while reloading shader '{}': {}", name, e.what());
			}
		}

		CH_CORE_INFO("Reloaded {}/{} shaders successfully", successCount, shaderNames.size());
	}
}

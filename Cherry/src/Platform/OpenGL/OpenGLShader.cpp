#include "CHpch.h"
#include "OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cherry {
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")   return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

		CH_CORE_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);

		ExtractShaderNameFromPath(filepath);
	}


	OpenGLShader::OpenGLShader(const std::string& name,const std::string& vertexSource, const std::string& fragmentSource)
		:m_Name(name)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSource;
		sources[GL_FRAGMENT_SHADER] = fragmentSource;
		Compile(sources);

	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	// ALTERNATIVE: More modern C++17 approach using filesystem
	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::error_code ec;

		// Check if file exists first
		if (!std::filesystem::exists(filepath, ec)) {
			CH_CORE_ERROR("File does not exist: '{0}'", filepath);
			return {};
		}

		// Get file size efficiently
		const auto size = std::filesystem::file_size(filepath, ec);
		if (ec) {
			CH_CORE_ERROR("Failed to get file size for '{0}': {1}", filepath, ec.message());
			return {};
		}

		// Open file
		std::ifstream in(filepath, std::ios::binary);
		if (!in) {
			CH_CORE_ERROR("Failed to open file '{0}'", filepath);
			return {};
		}

		// Read entire file
		std::string result(size, '\0');
		if (!in.read(result.data(), static_cast<std::streamsize>(size))) {
			CH_CORE_ERROR("Failed to read file '{0}'", filepath);
			return {};
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); // End of line
			CH_CORE_ASSERT(eol != std::string::npos,"Syntax error: no EOL after #type");

			size_t begin = pos + typeTokenLength + 1; // skip "#type "
			std::string type = source.substr(begin, eol - begin);
			CH_CORE_ASSERT(ShaderTypeFromString(type), "Invaled Shader Type Specified");


			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			CH_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error: no shader code after #type");
			pos = source.find(typeToken, nextLinePos); // Find next #type

			GLenum shaderType = ShaderTypeFromString(type);

			shaderSources[shaderType] = (pos == std::string::npos)
				? source.substr(nextLinePos)
				: source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();

		// Stack-allocated array for common cases (vertex + fragment = 2 shaders max)
		constexpr size_t MAX_SHADERS = 2; // Adjust based on your needs
		GLuint glShaderIDs[MAX_SHADERS];
		size_t glShaderIDIndex = 0;

		// Early check to prevent buffer overflow
		if (shaderSources.size() > MAX_SHADERS) {
			CH_CORE_ERROR("Too many shaders! Maximum supported: {0}", MAX_SHADERS);
			glDeleteProgram(program);
			return;
		}

		for (const auto& [type, source] : shaderSources) // C++17 structured binding
		{
			GLuint shader = glCreateShader(type);
			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);
			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				// Get error message length
				GLint logLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

				// Stack-allocated error message for small errors
				if (logLength <= 512) {
					char stackLog[512];
					glGetShaderInfoLog(shader, logLength, nullptr, stackLog);
					CH_CORE_ERROR("Shader compilation failed: {0}", stackLog);
				}
				else {
					// Fallback to heap for very large error messages
					std::vector<char> heapLog(logLength);
					glGetShaderInfoLog(shader, logLength, nullptr, heapLog.data());
					CH_CORE_ERROR("Shader compilation failed: {0}", heapLog.data());
				}

				// Cleanup
				glDeleteShader(shader);
				glDeleteProgram(program);
				for (size_t i = 0; i < glShaderIDIndex; ++i) {
					glDeleteShader(glShaderIDs[i]);
				}
				return;
			}

			glAttachShader(program, shader);
			glShaderIDs[glShaderIDIndex++] = shader;
		}

		// Link program
		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint logLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

			// Same stack/heap strategy for linking errors
			if (logLength <= 512) {
				char stackLog[512];
				glGetProgramInfoLog(program, logLength, nullptr, stackLog);
				CH_CORE_ERROR("Shader linking failed: {0}", stackLog);
			}
			else {
				std::vector<char> heapLog(logLength);
				glGetProgramInfoLog(program, logLength, nullptr, heapLog.data());
				CH_CORE_ERROR("Shader linking failed: {0}", heapLog.data());
			}

			// Cleanup
			glDeleteProgram(program);
			for (size_t i = 0; i < glShaderIDIndex; ++i) {
				glDeleteShader(glShaderIDs[i]);
			}
			return;
		}

		// Cleanup shaders (they're linked into the program now)
		for (size_t i = 0; i < glShaderIDIndex; ++i) {
			glDetachShader(program, glShaderIDs[i]);
			glDeleteShader(glShaderIDs[i]);
		}

		m_RendererID = program;
	} 


	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		UploadUniformFloat4(name, value);

	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		UploadUniformFloat3(name, value);
	}

	// Function to upload a 4x4 matrix uniform to the shader program.
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const {

		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1) {
			CH_CORE_ERROR("Uniform '{}' not found in shader!", name);
		}
		else {
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1) {
			CH_CORE_ERROR("Uniform '{}' not found in shader!", name);
		}
		else {
			glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}
	}

	void OpenGLShader::UploadUniformMat2(const std::string& name, const glm::mat2& matrix) const
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1) {
			CH_CORE_ERROR("Uniform '{}' not found in shader!", name);
		}
		else {
			glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1) {
			CH_CORE_ERROR("Uniform '{}' not found in shader!", name);
		}
		else {
			glUniform4f(location, value.x, value.y, value.z, value.w);
		}
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			return;
		}

		glUniform3f(location, value.x, value.y, value.z);
	}
	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			return;
		}

		glUniform2f(location, value.x, value.y);
	}
	void OpenGLShader::UploadUniformFloat(const std::string& name, const float& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			return;
		}

		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			return;
		}

		glUniform1i(location, value);
	}


	


}
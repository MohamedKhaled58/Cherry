#include "CHpch.h"
#include "Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cherry {
	Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource)
	{
			// Create an empty vertex shader handle
			GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const GLchar* source = vertexSource.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		// Compile the vertex shader
		glCompileShader(vertexShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			CH_CORE_ERROR("{0}", infoLog.data());
			CH_CORE_ASSERT(false,"Vertex Shader Compilation Failure!");
			return;
		}

		// Create an empty fragment shader handle
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		source = fragmentSource.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		// Compile the fragment shader
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragmentShader);
			// Either of them. Don't leak shaders.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			CH_CORE_ERROR("{0}", infoLog.data());
			CH_CORE_ASSERT(false, "Fragment Shader Compilation Failure!");
			return;
		}

		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
		m_RendererID = glCreateProgram();
		GLuint program = m_RendererID;
		// Attach our shaders to our program
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Use the infoLog as you see fit.
			CH_CORE_ERROR("{0}", infoLog.data());
			CH_CORE_ASSERT(false, "Shader Link Failure!");
			return;
		}

		// Always detach shaders after a successful link.
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_RendererID);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	// Function to upload a 4x4 matrix uniform to the shader program.
	void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const {

		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1) {
			CH_CORE_ERROR("Uniform '{}' not found in shader!", name);
		}
		else {
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}
	}

	void Shader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			// Optional: Log warning about uniform not found
			// std::cout << "Warning: Uniform '" << name << "' not found in shader!" << std::endl;
			return;
		}

		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::UploadUniformFloat3(const std::string& name, float x, float y, float z)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		if (location == -1)
		{
			// Optional: Log warning about uniform not found
			// std::cout << "Warning: Uniform '" << name << "' not found in shader!" << std::endl;
			return;
		}

		glUniform3f(location, x, y, z);
	}

}

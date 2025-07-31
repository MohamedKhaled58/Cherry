#pragma once
#include "Cherry/Renderer/Shader.h"
#include <glm/glm.hpp>

//TODO REMOVE!
typedef unsigned int GLenum;

namespace Cherry {
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		OpenGLShader(const std::string& name,const std::string& vertexSource, const std::string& fragmentSource);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetInt(const std::string& name, int value) override;

		virtual std::string GetName() const override { return m_Name; }


		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) const;
		void UploadUniformMat2(const std::string& name, const glm::mat2& matrix) const;

		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat(const std::string& name, const float& value);

		void UploadUniformInt(const std::string& name, int value);






	private:
		std::string ReadFile(const std::string& filepath );
		std::unordered_map<GLenum,std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
	private:
		uint32_t m_RendererID = 0;

		std::string m_Name;
	};
}
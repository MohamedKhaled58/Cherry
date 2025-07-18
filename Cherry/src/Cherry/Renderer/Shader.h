#pragma once
#include <string>
#include <glm/glm.hpp>

namespace Cherry {

	class Shader {

	public:
		Shader(const std::string& vertexSource, const std::string& fragmentSource);
		~Shader();

		void Bind() const;
		void Unbind() const;

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) const;
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat3(const std::string& name, float x, float y, float z);

	private:
		uint32_t m_RendererID = 0;

	};
}
#pragma once
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Cherry {

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const std::string& GetName() const = 0;

		static REF(Shader) Create(const std::string& filepath);
		static REF(Shader) Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const REF(Shader)& shader);
		void Add(const REF(Shader)& shader);
		REF(Shader) Load(const std::string& filepath);
		REF(Shader) Load(const std::string& name, const std::string& filepath);

		REF(Shader) Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, REF(Shader)> m_Shaders;
	};

}
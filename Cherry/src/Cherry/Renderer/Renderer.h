#pragma once

#include "Cherry/Renderer/RenderCommand.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/Shader.h"
#include <glm/glm.hpp>


namespace Cherry {

	class Renderer 
	{
	public:
		static void Init();
		static void BeginScene(OrthographicCamera& camera);		//TODO:: All Scene Params
		static void EndScene();

		static void Submit(const REF(Shader)& shader, const REF(VertexArray)& vertexArray, const glm::mat4& transform = glm::mat4 (1.0f));

		static void Flush();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData {
		public:
			glm::mat4 ViewProjectionMatrix;
		};


		static SceneData* m_SceneData;
	};

}
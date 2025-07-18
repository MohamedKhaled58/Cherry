#pragma once

#include "Cherry/Renderer/RenderCommand.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/Shader.h"
#include <glm/glm.hpp>


namespace Cherry {

	class Renderer 
	{
	public:
		static void BeginScene(OrthographicCamera& camera);		//TODO:: All Scene Params
		static void EndScene();

		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);

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
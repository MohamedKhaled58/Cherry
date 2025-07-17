#pragma once

#include "Cherry/Renderer/RenderCommand.h"

namespace Cherry {

	class Renderer 
	{
	public:
		static void BeginScene();		//TODO:: All Scene Params
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);
		static void Flush();

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};

}
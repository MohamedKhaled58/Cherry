#pragma once

#include "Cherry/Renderer/RendererAPI.h"

namespace Cherry {

	class RenderCommand
	{
	public:
		//Dispatch to s_RendererAPI
		inline static  void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }
		inline static  void Clear() { s_RendererAPI->Clear(); }

		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static RendererAPI* s_RendererAPI;

	};
}
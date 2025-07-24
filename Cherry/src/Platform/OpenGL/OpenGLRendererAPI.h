#pragma once
 
#include "Cherry/Renderer/RendererAPI.h"

namespace Cherry {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const REF(VertexArray)& vertexArray) override;
	private:

	};
}
#pragma once
#include "Cherry/Renderer/VertexArray.h"

namespace Cherry {

	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;


		virtual void AddVertexBuffer(const REF(VertexBuffer)& vertexBuffer) override;
		virtual void SetIndexBuffer(const REF(IndexBuffer)& indexBuffer) override;

		virtual const std::vector<REF(VertexBuffer)>& GetVertexBuffers() const override{ return m_VertexBuffers; }
		virtual const REF(IndexBuffer)& GetIndexBuffers() const override { return m_IndexBuffer; }


	private:
		std::vector< REF(VertexBuffer)> m_VertexBuffers;
		REF(IndexBuffer) m_IndexBuffer;

		uint32_t m_RendererID = 0;
		uint32_t m_VertexBufferIndex = 0;

	};
}
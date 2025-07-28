#pragma once
#include <memory>
#include "Cherry/Renderer/Buffer.h"

namespace Cherry {

	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;


		virtual void AddVertexBuffer(const REF(VertexBuffer)& vertexBuffer) = 0;
		virtual void SetIndexBuffer(const REF(IndexBuffer)& indexBuffer) = 0;

		virtual const std::vector< REF(VertexBuffer)>& GetVertexBuffers() const = 0;
		virtual const REF(IndexBuffer)& GetIndexBuffers() const = 0;


		static REF(VertexArray) Create();
	};
}
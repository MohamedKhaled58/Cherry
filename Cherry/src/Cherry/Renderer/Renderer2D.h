#pragma once
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/Texture.h"
#include <memory>

namespace Cherry {
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();
		static void BeginScene(const OrthographicCamera& camera);
		static void EndScene();

		// PRIMITIVES - Position-based rendering
		static void DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		
		// Transform matrix-based rendering (for ECS)
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture, 
			float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Circle rendering
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, 
			float thickness = 1.0f, float fade = 0.005f);

	private:
	};
}
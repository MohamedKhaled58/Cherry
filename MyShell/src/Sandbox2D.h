#pragma once
#include "Cherry/Core/Layer.h"
#include <Cherry/Renderer/VertexArray.h>
#include <Cherry/Renderer/Shader.h>
#include <Cherry/OrthographicCameraController.h>

namespace Cherry {

	class Sandbox2D : public Layer
	{
	public:
		Sandbox2D();
		virtual ~Sandbox2D() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(TimeStep timeStep) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		Cherry::OrthographicCameraController m_CameraController;

		//TEMP
		REF(Shader) m_FlatColorShader;
		REF(VertexArray) m_FlatColorVertexArray;


		glm::vec4 m_SquareColor = { 0.3f,0.1f,0.8f,1.0f };
	};
}
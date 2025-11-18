#pragma once
#include "Cherry.h"

class Sandbox2D : public Cherry::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Cherry::TimeStep timeStep) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Cherry::Event& event) override;
private:
	Cherry::OrthographicCameraController m_CameraController;

	//TEMP
	REF(Cherry::Shader) m_FlatColorShader;
	REF(Cherry::VertexArray) m_FlatColorVertexArray;
	REF(Cherry::Texture2D) m_CheckerboardTexture;
	glm::vec4 m_SquareColor = { 0.3f,0.1f,0.8f,1.0f };
};

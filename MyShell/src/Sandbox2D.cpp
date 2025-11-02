#include "Cherry.h"
#include "Sandbox2D.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <Cherry/Renderer/Renderer.h>

namespace Cherry {
    Sandbox2D::Sandbox2D()
        :Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
    {

    }

    void Sandbox2D::OnAttach()
    {
        m_CheckerboardTexture = Cherry::Texture2D::Create("assets/textures/Checkerboard.png");

    }

    void Sandbox2D::OnDetach()
    {

    }

    void Sandbox2D::OnUpdate(TimeStep timeStep)
    {
        //  UPDATE
        m_CameraController.OnUpdate(timeStep);

        //  RENDER
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();

        Renderer2D::BeginScene(m_CameraController.GetCamera());

        Renderer2D::DrawQuad({ -1.0f, 0.0f}, glm::radians(0.0f), { 0.5f, 0.5f },  m_SquareColor);
        Renderer2D::DrawQuad({ 0.5f, -0.5f}, glm::radians(0.0f), { 1.0f, 1.0f }, { 0.2f, 0.8f, 0.0f, 1.5f });
        Renderer2D::DrawQuad({ 0.0f, 0.0f , -0.1}, glm::radians(0.0f), { 10.0f, 10.0f }, m_CheckerboardTexture);

        Renderer2D::EndScene();
    }

    void Sandbox2D::OnImGuiRender()
    {
        // Keep your existing settings window
        ImGui::Begin("Render Settings");
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }

    void Sandbox2D::OnEvent(Event& e)
    {
        m_CameraController.OnEvent(e);
    }
}
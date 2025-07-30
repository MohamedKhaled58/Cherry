#include "Cherry.h"
#include "Sandbox2D.h"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Cherry {

    Sandbox2D::Sandbox2D()
        : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true) {
    }

    void Sandbox2D::OnAttach() {
    }

    void Sandbox2D::OnDetach() {
    }

    void Sandbox2D::OnUpdate(TimeStep timeStep) {
        m_CameraController.OnUpdate(timeStep);

        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();

        // Use EXISTING Renderer2D (not BatchRenderer2D)
        Renderer2D::BeginScene(m_CameraController.GetCamera());

        // Use your current Renderer2D methods:
        Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f });
        Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 1.0f, 1.0f }, m_SquareColor);

        Renderer2D::EndScene();
    }
    void Sandbox2D::OnImGuiRender() {
        ImGui::Begin("Settings");
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }

    void Sandbox2D::OnEvent(Event& e) {
        m_CameraController.OnEvent(e);
    }
}
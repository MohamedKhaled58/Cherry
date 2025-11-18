#include "Cherry.h"
#include "Sandbox2D.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>



Sandbox2D::Sandbox2D()
    :Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{

}

void Sandbox2D::OnAttach()
{
    CH_PROFILE_FUNCTION();

    m_CheckerboardTexture = Cherry::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
    CH_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Cherry::TimeStep timeStep)
{

    {
        CH_PROFILE_SCOPE("CameraController::OnUpdate");
        //Update
        m_CameraController.OnUpdate(timeStep);
    }

    {
        CH_PROFILE_SCOPE("RenderPrep");
        //Render
        Cherry::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Cherry::RenderCommand::Clear();
    }


    {
        CH_PROFILE_SCOPE("BeginScene");
        Cherry::Renderer2D::BeginScene(m_CameraController.GetCamera());
    }

    {
        CH_PROFILE_SCOPE("Draw");
        Cherry::Renderer2D::DrawQuad({ -1.0f, 0.0f }, glm::radians(0.0f), { 0.5f, 0.5f }, m_SquareColor);
        Cherry::Renderer2D::DrawQuad({ 0.5f, -0.5f }, glm::radians(0.0f), { 1.0f, 1.0f }, { 0.2f, 0.8f, 0.0f, 1.5f });
        Cherry::Renderer2D::DrawQuad({ 0.0f, 0.0f , -0.1 }, glm::radians(0.0f), { 10.0f, 10.0f }, m_CheckerboardTexture);
        Cherry::Renderer2D::EndScene();
    }

    
}

void Sandbox2D::OnImGuiRender()
{
    CH_PROFILE_FUNCTION();

    // Keep your existing settings window
    ImGui::Begin("Render Settings");
    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Cherry::Event& e)
{
    m_CameraController.OnEvent(e);
}

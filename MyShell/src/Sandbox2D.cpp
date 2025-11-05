#include "Cherry.h"
#include "Sandbox2D.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

template<typename Fn>
class Timer
{
public:
    Timer(const char* name ,Fn&& func)
	    :m_Name(name), m_Func(func), m_Stopped(false)
    {
        m_StartTimePoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        if (!m_Stopped)
            Stop();
    }

    void Stop()
    {
        auto endTimePoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

        m_Stopped = true;
        
        float duration = (end - start) * 0.001f;
        m_Func({m_Name,duration});
    }



private:
    const char* m_Name;
    Fn m_Func;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimePoint;
    bool m_Stopped;
};


#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult) { m_ProfileResults.push_back(profileResult); })

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

void Sandbox2D::OnUpdate(Cherry::TimeStep timeStep)
{
    PROFILE_SCOPE("Sandbox2D::OnUpdate");

    {
        PROFILE_SCOPE("CameraController::OnUpdate");
        //Update
        m_CameraController.OnUpdate(timeStep);
    }

    {
        PROFILE_SCOPE("RenderPrep");
        //Render
        Cherry::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Cherry::RenderCommand::Clear();
    }


    {
        PROFILE_SCOPE("Draw");
        Cherry::Renderer2D::BeginScene(m_CameraController.GetCamera());
        Cherry::Renderer2D::DrawQuad({ -1.0f, 0.0f }, glm::radians(0.0f), { 0.5f, 0.5f }, m_SquareColor);
        Cherry::Renderer2D::DrawQuad({ 0.5f, -0.5f }, glm::radians(0.0f), { 1.0f, 1.0f }, { 0.2f, 0.8f, 0.0f, 1.5f });
        Cherry::Renderer2D::DrawQuad({ 0.0f, 0.0f , -0.1 }, glm::radians(0.0f), { 10.0f, 10.0f }, m_CheckerboardTexture);
		Cherry::Renderer2D::EndScene();
    }
}

void Sandbox2D::OnImGuiRender()
{
    // Keep your existing settings window
    ImGui::Begin("Render Settings");
    ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));

    for (auto& result : m_ProfileResults)
    {
        char label[50];
        strcpy_s(label,"%.3f ms ");
        strcat_s(label, result.Name);
        ImGui::Text(label, result.Time);
    }
    m_ProfileResults.clear();
    ImGui::End();
}

void Sandbox2D::OnEvent(Cherry::Event& e)
{
    m_CameraController.OnEvent(e);
}

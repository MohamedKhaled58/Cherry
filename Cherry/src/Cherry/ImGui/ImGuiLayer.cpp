#include "CHpch.h"
#include "ImGuiLayer.h"
#include "Cherry/Application.h"

#include "imgui.h"
#include "Cherry/Platform/OpenGL/ImGuiOpenGlRenderer.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Cherry {
	ImGuiLayer::ImGuiLayer()
		:Layer("ImGuiLayer")
	{
	}
	ImGuiLayer::~ImGuiLayer()
	{

	}
    void ImGuiLayer::OnAttach()
    {
        // Setup Dear ImGui context
        //IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        // Initialize OpenGL loader
        if (!gladLoadGL()) {
            CH_CORE_ERROR("Failed to initialize GLAD!");
            return;
        }

        const char* glsl_version = "#version 410";
        if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
            CH_CORE_ERROR("ImGui_ImplOpenGL3_Init failed!");
            return;
        }
    }

	void ImGuiLayer::OnDetach()
	{

	}
	void ImGuiLayer::OnUpdate()
	{
        

        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

        

        float time = (float)glfwGetTime();
        io.DeltaTime = m_time > 0.0 ? (time - m_time) : (1.0f / 60.0f);
        m_time = time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


	}
	void ImGuiLayer::OnEvent(Event& event)
	{

	}
}
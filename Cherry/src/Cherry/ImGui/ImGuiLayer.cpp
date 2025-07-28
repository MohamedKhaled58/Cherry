#include "CHpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"

#include "Cherry/Core/Application.h"

//Temporary include for GLFW and glad
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Cherry {
    ImGuiLayer::ImGuiLayer() :Layer("ImGuiLayer")
    {
        CH_CORE_INFO("ImGuiLayer created");
    }
    ImGuiLayer::~ImGuiLayer()
    {
        CH_CORE_INFO("ImGuiLayer Destroyed");
    }

    // Attach the ImGui layer, called when the application starts
    void ImGuiLayer::OnAttach()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Application& app = Application::Get();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());


        ImGui_ImplGlfw_InitForOpenGL(window, true);
        const char* glsl_version = "#version 410";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    // Detach the ImGui layer and clean up resources
    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        CH_CORE_INFO("ImGuiLayer detached and resources cleaned up");
    }

    // Begin a new ImGui frame, setting up the necessary state for rendering
    void ImGuiLayer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // End the ImGui frame, render the ImGui draw data, and handle viewports if enabled
    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    // Render ImGui content, this is where you define your ImGui UI elements For The Engine
    void ImGuiLayer::OnImGuiRender()
    {
    }
}


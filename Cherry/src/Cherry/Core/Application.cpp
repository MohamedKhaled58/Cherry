#include "CHpch.h"
#include "Application.h"
#include "Cherry/Renderer/Buffer.h"
#include "Cherry/Renderer/Renderer.h"
#include <GLFW/glfw3.h>


namespace Cherry {

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        CH_PROFILE_FUNCTION();
		CH_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(CH_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();
       
        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
        CH_CORE_INFO("Application initialized successfully");
    }

    Application::~Application()
    {
        CH_PROFILE_FUNCTION();

        CH_CORE_TRACE("Application Destroyed!");

        // Note: m_ImGuiLayer is owned by LayerStack and will be deleted
        // when LayerStack destructor runs (after this destructor completes)

        // Shutdown renderer
        Renderer::Shutdown();

        s_Instance = nullptr;
    }

    void Application::PushLayer(Layer* layer)
    {
        CH_PROFILE_FUNCTION();

        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
        CH_CORE_TRACE("Pushed Layer: {0}", layer->GetName());
    }

    void Application::PushOverlay(Layer* layer)
    {
        CH_PROFILE_FUNCTION();

        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
        CH_CORE_TRACE("Pushed Overlay: {0}", layer->GetName());
    }

    void Application::OnEvent(Event& e)
    {
        CH_PROFILE_FUNCTION();

        EventDispatcher dispatcher(e);
        // Dispatch event to layers
        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
            (*--it)->OnEvent(e);
            if (e.Handled) break;
        }
        dispatcher.Dispatch<WindowCloseEvent>(CH_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(CH_BIND_EVENT_FN(Application::OnWindowResize));
    }


    void Application::Run()
    {
        CH_PROFILE_FUNCTION();

        while (m_Running)
        {
            CH_PROFILE_SCOPE("Run Loop");

            float time = (float)glfwGetTime();
            TimeStep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                {
                    CH_PROFILE_SCOPE("LayerStack OnUpdate");

                    for (Layer* layer : m_LayerStack)
                        layer->OnUpdate(timestep);
                }
                m_ImGuiLayer->Begin();
                // Render profiler UI
                {
                    CH_PROFILE_SCOPE("LayerStack OnImGuiRender");

                    for (Layer* layer : m_LayerStack)
                        layer->OnImGuiRender();
                }
                m_ImGuiLayer->End();
            }
                m_Window->OnUpdate();
        }
    }


    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }
    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        CH_PROFILE_FUNCTION();

        // Get new window dimensions
        unsigned int width = e.GetWidth();
        unsigned int height = e.GetHeight();

        // Handle window minimization (width or height = 0)
        if (width == 0 || height == 0)
        {
            m_Minimized = true;
            return false; // Don't block the event
        }

        // Window is no longer minimized if it was before
        m_Minimized = false;

        // Update renderer viewport to match new window size
        Cherry::Renderer::OnWindowResize(width, height);

        // DON'T update camera controller here - it should be handled in the layer
        // The Application class shouldn't directly manage layer-specific components
        // Each layer should handle its own camera controller in its OnEvent method

        // Store new window dimensions (if you need them in Application class)

        // Log the resize for debugging (optional)
        CH_CORE_INFO("Window resized to {}x{}", width, height);

        // Return false to allow the event to propagate to other layers
        // This ensures each layer can handle the resize event for their own cameras
        return false;
    }
}
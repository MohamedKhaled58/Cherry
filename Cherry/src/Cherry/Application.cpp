#include "CHpch.h"
#include "Application.h"
#include "Cherry/Renderer/Buffer.h"
#include "Cherry/Renderer/Renderer.h"


namespace Cherry {

    Application* Application::s_Instance = nullptr;


    Application::Application()
    {
        s_Instance = this;
        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(CH_BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
        CH_CORE_INFO("Application initialized successfully");
    }

    // Destroy the application instance and clean up resources
    Application::~Application()
    {
        CH_CORE_TRACE("Application Destroyed!");

        s_Instance = nullptr;
    }

    // Initialize the application
    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
        CH_CORE_TRACE("Pushed Layer: {0}", layer->GetName());
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
        CH_CORE_TRACE("Pushed Overlay: {0}", layer->GetName());
    }

    // Handle events in the application
    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        // Dispatch event to layers
        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
            (*--it)->OnEvent(e);
            if (e.Handled) break;
        }
        // Handle window events
        dispatcher.Dispatch<WindowCloseEvent>(CH_BIND_EVENT_FN(Application::OnWindowClose));
    }

    // Main loop of the application
    void Application::Run()
    {
        while (m_Running) {
            // Update layers
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate();

            // Update IMGUI
            if (m_ImGuiLayer) {
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
                m_ImGuiLayer->End();
            }

            // Update Window
            CH_CORE_ASSERT(m_Window, "Window is null!");
            if (m_Window) {
                m_Window->OnUpdate();
            }
        }
    }


    // Handle window close event
    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }
}
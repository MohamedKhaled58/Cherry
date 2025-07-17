#pragma once
#include "Core.h"
#include "Window.h"

#include "Cherry/LayerStack.h"
#include "Cherry/Events/Event.h"
#include "Cherry/Events/ApplicationEvent.h"

#include "Cherry/ImGui/ImGuiLayer.h"

#include "Cherry/Renderer/Shader.h"
#include "Cherry/Renderer/Buffer.h"
#include "Cherry/Renderer/VertexArray.h"


namespace Cherry {

	class CHERRY_API Application
	{

	public:
		Application();
		virtual ~Application();
		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() {
			if (!m_Window)
				throw std::runtime_error("Window is null");
			return *m_Window;
		}
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		static Application* s_Instance;

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		std::shared_ptr<Shader> m_Shader;
		std::shared_ptr<VertexArray> m_VertexArray;

		std::shared_ptr<Shader> m_SquareS;
		std::shared_ptr<VertexArray> m_SquareVA;

	};

	// TO BE DEFINED IN CLIENT
	Application* CreateApplication();
}



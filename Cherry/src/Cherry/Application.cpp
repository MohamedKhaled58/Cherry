#include "CHpch.h"
#include "Application.h"
#include <glad/glad.h>
#include "Cherry/Input.h"
#include "KeyCodes.h"

namespace Cherry {

	Application* Application::s_Instance = nullptr;

	static GLenum ShaderDataTypeToOpenGlBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case Cherry::ShaderDataType::Float:		return GL_FLOAT;
		case Cherry::ShaderDataType::Float2:		return GL_FLOAT;
		case Cherry::ShaderDataType::Float3:		return GL_FLOAT;
		case Cherry::ShaderDataType::Float4:		return GL_FLOAT;
		case Cherry::ShaderDataType::Mat3:			return GL_FLOAT;
		case Cherry::ShaderDataType::Mat4:			return GL_FLOAT;
		case Cherry::ShaderDataType::Int:			return GL_INT;
		case Cherry::ShaderDataType::Int2:			return GL_INT;
		case Cherry::ShaderDataType::Int3:			return GL_INT;
		case Cherry::ShaderDataType::Int4:			return GL_INT;
		case Cherry::ShaderDataType::Bool:			return GL_BOOL;
		}
		CH_CORE_ASSERT(false, "UnKnown ShaderDataType : {0}", type);
		return 0;
	}

	// Create the application instance and initialize the window and ImGui layer
	Application::Application()
	{
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		float Vertices[3 * 7] =
		{
			//     Position            |      Color (RGBA)
			-0.5f, -0.5f,  0.0f,        1.0f, 0.5f, 0.0f, 1.0f,  // Left - Orange
			 0.5f, -0.5f,  0.0f,        1.0f, 1.0f, 0.0f, 1.0f,  // Right - Yellow
			 0.0f,  0.5f,  0.0f,        1.0f, 0.0f, 0.0f, 1.0f   // Top - Red
		};


		//	Vertex Array
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		//	Vertex Buffer
		m_VertexBuffer.reset(VertexBuffer::Create(Vertices, sizeof(Vertices), GL_STATIC_DRAW));

		{
			BufferLayout layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float4, "a_Color" }
			};
			m_VertexBuffer->SetLayout(layout);
		}

		uint32_t Index = 0;
		const auto& layout = m_VertexBuffer->GetLayout();
		for (const auto& element : layout )
		{
			glEnableVertexAttribArray(Index);
			glVertexAttribPointer
			(
				Index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGlBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset
			);

			Index++;
		}




		//	Index Buffer
		uint32_t Indices[3] = { 0 , 1 , 2 };
		m_IndexBuffer.reset(IndexBuffer::Create(Indices, sizeof(Indices) / sizeof(uint32_t), GL_STATIC_DRAW));


		//	Shader		--ALOT OF WORK
		std::string vertexSource = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);
			}
		)";

		std::string fregmantSource = R"(
			#version 330 core
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;

			}
		)";

		m_Shader.reset(new Shader(vertexSource, fregmantSource));
	}


	// Destroy the application instance and clean up resources
	Application::~Application()
	{
		CH_CORE_TRACE("Application Destroyed!");
		s_Instance = nullptr;
		CH_CORE_ASSERT(!s_Instance, "Application Already exist!");
	}

	// Initialize the application
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
		CH_CORE_TRACE("Pushed Layer: {0}", layer->GetName());
		CH_CORE_TRACE("Pushed Layer: {0}", m_ImGuiLayer->GetName());

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
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

	}

	// Main loop of the application
	void Application::Run()
	{
		while (m_Running) {

			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

			// Update Input
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();



			//Update IMGUI
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();




			// Update Window
			CH_CORE_ASSERT(m_Window, "Window is null!");
			m_Window->OnUpdate();
		}
	}

	// Handle window close event
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}

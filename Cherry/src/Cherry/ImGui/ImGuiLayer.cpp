#include "CHpch.h"
#include "ImGuiLayer.h"
#include "Cherry/Application.h"

#include "imgui.h"
#include "Cherry/Platform/OpenGL/ImGuiOpenGlRenderer.h"
#include <Cherry/Platform/GLFW/GLFW.h>

//Temporary include for GLFW and glad
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
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
		ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // Enable Mouse Cursors
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // Enable Set Mouse Position
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // Enable Mouse Hovered Viewport
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows
		
		

        

        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE; // Required for navigation
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A; // For Ctrl+A
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C; // For Ctrl+C
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V; // For Ctrl+V
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X; // For Ctrl+X
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y; // For Ctrl+Y
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z; // For Ctrl+Z



        /*
        // Load Fonts
        io.Fonts->AddFontDefault();
        io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Bold.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Light.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Italic.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Regular.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Bold.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Light.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Italic.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Medium.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-BoldItalic.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-LightItalic.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Italic.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-MediumItalic.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Bold.ttf", 16.0f);
        io.Fonts->AddFontFromFileTTF("assets/fonts/RobotoMono-Light.ttf", 16.0f);
        */
        


		// Setup Platform/Renderer bindings
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
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
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
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
        dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
        dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
        dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
        dispatcher.Dispatch<KeyTypedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
        dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent));
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(ImGuiLayer::OnWindowCloseEvent));

	}

	// Event Handlers
    bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& event)
    {
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[event.GetMouseButton()] = true;
        return false;
    }
    bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = false;
        return false;
    }
    bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& event)
    {

		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(event.GetX(), event.GetY());
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		if (main_viewport) {
			main_viewport->Pos = ImVec2(event.GetX(), event.GetY());
			main_viewport->Size = ImVec2(io.DisplaySize.x, io.DisplaySize.y);
		}
        return false;
    }
    bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += event.GetXOffset();
		io.MouseWheel += event.GetYOffset();
		io.MouseWheelH = event.GetXOffset();
		io.MouseWheel = event.GetYOffset();
        return false;
    }
    bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        int keycode = event.GetKeyCode();

        // Only handle printable ASCII characters (32 to 126) for text input
        if (keycode >= 32 && keycode <= 126) {
            io.AddInputCharacter(static_cast<unsigned short>(keycode));
            return true; // Mark event as handled
        }

        // Special keys like Enter, Backspace, etc., should not be handled as typed characters
        // They are typically handled in OnKeyPressedEvent or by ImGui's key state
        return false;
    }

    bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        int keycode = event.GetKeyCode();

        // Update ImGui key state
        if (keycode >= 0 && keycode < IM_ARRAYSIZE(io.KeysDown)) {
            io.KeysDown[keycode] = true;
        }
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
        return false;
    }
    bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();
        int keycode = event.GetKeyCode();
        io.KeysDown[keycode] = false;
        return false;

    }
    bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& event)
    {
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(event.GetWidth(), event.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f); // Assuming no DPI scaling for simplicity
		//glViewport(0, 0, event.GetWidth(), event.GetHeight()); // Update OpenGL viewport
        return false;
    }
    bool ImGuiLayer::OnWindowCloseEvent(WindowCloseEvent& event)
    {
        ImGuiIO& io = ImGui::GetIO();

        return false;
    }
}
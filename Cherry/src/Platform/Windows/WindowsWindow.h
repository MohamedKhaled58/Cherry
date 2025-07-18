#pragma once
#include "Cherry/Window.h"
#include <GLFW/glfw3.h>
#include "Cherry/Renderer/GraphicsContext.h"

namespace Cherry
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;
		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }
		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		// Returns the native window pointer (GLFWwindow* in this case)
		inline virtual void* GetNativeWindow() const override { return m_Window; } 

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		// Assuming I have a GraphicsContext class to manage OpenGL context
		GraphicsContext* m_Context; 

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
	
}

#pragma once
#include "CHpch.h"
#include "Cherry/Core.h"
#include "Cherry/Events/Event.h"

namespace Cherry {

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;

		WindowProps
		(
			const std::string& title = "Cherry Engine",
			unsigned int width = 1280,
			unsigned int height = 720
		)
			: Title(title), Width(width), Height(height)
		{

		}
	};

	// Interface representing a desktop system based Window
	class CHERRY_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		
		// Returns a pointer to the native window(GLFWwindow* for GLFW)
		virtual void* GetNativeWindow() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};

}



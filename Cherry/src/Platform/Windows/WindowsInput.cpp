// ReSharper disable All
#include "CHpch.h"
#include "WindowsInput.h"

#include "Cherry/Core/Application.h"
#include "glfw/glfw3.h"
namespace Cherry
{
	Input* Input::s_Instance = new WindowsInput(); // Initialize the static instance of Input

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		auto window =static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(window, keycode);
		if (state == GLFW_PRESS || state == GLFW_REPEAT)
			return true;
		else if (state == GLFW_RELEASE)
			return false;
		else
			CH_CORE_ERROR("Unknown key state: {0}", state);

		return false;
	}
	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, button);
		if (state == GLFW_PRESS)
			return true;
		else if (state == GLFW_RELEASE)
			return false;
		else
			CH_CORE_ERROR("Unknown mouse button state: {0}", state);
		return false;
	}

	float WindowsInput::GetMouseXImpl()
	{
		auto [X, Y] = GetMousePositionImp();
		// Return the X position of the mouse cursor
		return (X);
	}
		

	float WindowsInput::GetMouseYImpl()
	{
		auto [X, Y] = GetMousePositionImp();
		// Return the y position of the mouse cursor
		return (Y);

		
	}

	std::pair<float, float> WindowsInput::GetMousePositionImp()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		// Return the mouse position as a float
		if (xPos >= 0 && yPos >= 0) // Ensure the position is valid
			return { (float)xPos , (float)yPos };
		else
			CH_CORE_ERROR("Invalid mouse position: ({0}, {1})", xPos, yPos);
		return { 0.0f, 0.0f }; // Return a default value if the position is invalid
	}

	
}
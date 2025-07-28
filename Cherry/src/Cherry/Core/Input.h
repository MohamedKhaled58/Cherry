#pragma once
#include "Cherry/Core/Core.h"
#include <utility> // for std::pair

namespace Cherry {
	// Input class to handle input events
	class CHERRY_API Input {
	protected:
		Input() = default;
	public:
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;
		// Check if a key is pressed
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		// Get the current mouse position
		inline static std::pair<float, float> GetMousePosition() {return s_Instance->GetMousePositionImp();}
		
		
	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
		virtual std::pair<float, float> GetMousePositionImp() = 0;
	private:
			static Input* s_Instance;
	};
} // namespace Cherry
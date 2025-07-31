#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace Cherry {
    // Input system type definitions
    enum class InputType {
        None = 0,
        Keyboard,
        Mouse,
        Gamepad
    };

    enum class GamepadAxis {
        LeftX = 0,
        LeftY,
        RightX,
        RightY,
        LeftTrigger,
        RightTrigger
    };

    struct InputBinding {
        InputType Type = InputType::None;
        int32_t Code = 0;           // Key code, mouse button, or gamepad button
        GamepadAxis Axis = GamepadAxis::LeftX; // For gamepad axis inputs
        float Threshold = 0.5f;     // For analog inputs
        bool IsAxis = false;        // True for analog inputs

        InputBinding() = default;
        InputBinding(InputType type, int32_t code) : Type(type), Code(code) {}
        InputBinding(GamepadAxis axis, float threshold = 0.5f) 
            : Type(InputType::Gamepad), Axis(axis), Threshold(threshold), IsAxis(true) {}
    };

    class InputManager {
    public:
        // Action mapping system
        void MapAction(const std::string& action, InputBinding binding);
        bool IsActionPressed(const std::string& action);
        bool IsActionJustPressed(const std::string& action);

        // Input contexts for different game states
        void PushContext(const std::string& context);
        void PopContext();

        // Gamepad support
        bool IsGamepadConnected(int gamepadId);
        float GetGamepadAxis(int gamepadId, GamepadAxis axis);

    private:
        std::vector<std::string> m_ContextStack;
        std::unordered_map<std::string, std::vector<InputBinding>> m_ActionMappings;
    };
}
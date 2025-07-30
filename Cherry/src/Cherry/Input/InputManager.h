#pragma once
#include <string>
#include <vector>
#include <unordered_map>
namespace Cherry {
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
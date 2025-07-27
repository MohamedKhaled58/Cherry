#pragma once

#include "Cherry/Renderer/Camera.h"
#include "Cherry/Core/TimeStep.h"
#include "Cherry/Events/ApplicationEvent.h"
#include "Cherry/Events/MouseEvent.h"
#include "Cherry/Events/KeyEvent.h"
#include "Cherry/Core/Input.h"
#include <glm/glm.hpp>

namespace Cherry {

    class OrthographicCameraController {
    public:
        OrthographicCameraController(float aspectRatio, bool enableRotation = false);
        virtual ~OrthographicCameraController() = default;

        // Disable copy constructor and assignment operator
        OrthographicCameraController(const OrthographicCameraController&) = delete;
        OrthographicCameraController& operator=(const OrthographicCameraController&) = delete;

        // Camera access - return by reference to avoid copies
        OrthographicCamera& GetCamera() { return m_Camera; }
        const OrthographicCamera& GetCamera() const { return m_Camera; }

        // Update (called every frame)
        void OnUpdate(TimeStep ts);
        void OnEvent(Event& e);

        // ===== Configuration Methods =====

        // Zoom configuration
        void SetZoomLevel(float level);
        float GetZoomLevel() const { return m_ZoomLevel; }
        void SetZoomLimits(float minZoom, float maxZoom);
        float GetMinZoom() const { return m_MinZoomLevel; }
        float GetMaxZoom() const { return m_MaxZoomLevel; }
        void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }
        float GetZoomSpeed() const { return m_ZoomSpeed; }

        // Movement configuration
        void SetCameraSpeed(float speed) { m_CameraTranslationSpeed = speed; }
        float GetCameraSpeed() const { return m_CameraTranslationSpeed; }

        // Rotation configuration
        void SetRotationEnabled(bool enabled) { m_Rotation = enabled; }
        bool IsRotationEnabled() const { return m_Rotation; }
        void SetRotationSpeed(float speed) { m_CameraRotationSpeed = speed; }
        float GetRotationSpeed() const { return m_CameraRotationSpeed; }

        // Mouse configuration
        void SetMouseSensitivity(float sensitivity) { m_MouseSensitivity = sensitivity; }
        float GetMouseSensitivity() const { return m_MouseSensitivity; }

        // Camera bounds configuration
        void SetBounds(float left, float right, float bottom, float top);
        void EnableBounds(bool enable) { m_BoundsEnabled = enable; }
        bool IsBoundsEnabled() const { return m_BoundsEnabled; }
        glm::vec4 GetBounds() const { return { m_BoundsLeft, m_BoundsRight, m_BoundsBottom, m_BoundsTop }; }

        // ===== Utility Methods =====

        // Reset camera to default position
        void Reset();

        // Coordinate conversion
        glm::vec2 ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize) const;
        glm::vec2 WorldToScreen(const glm::vec3& worldPos, const glm::vec2& screenSize) const;

        // Camera information
        glm::vec3 GetCameraPosition() const { return m_Camera.GetPosition(); }
        float GetCameraRotation() const { return m_Camera.GetRotation(); }

    private:
        // ===== Event Handlers =====
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& e);
        bool OnMouseMoved(MouseMovedEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnKeyReleased(KeyReleasedEvent& e);

        // ===== Helper Methods =====
        void UpdateCameraPosition(TimeStep ts);
        void UpdateCameraRotation(TimeStep ts);
        void ApplyBounds();
        void RecalculateView();

        // ===== Validation Methods =====
        bool IsZoomLevelValid(float zoom) const;
        bool AreBoundsValid(float left, float right, float bottom, float top) const;

    private:
        // ===== Core Properties =====
        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;

        // ===== Camera Settings =====
        bool m_Rotation = false;
        float m_CameraTranslationSpeed = DEFAULT_CAMERA_SPEED;
        float m_CameraRotationSpeed = DEFAULT_ROTATION_SPEED;
        float m_ZoomSpeed = DEFAULT_ZOOM_SPEED;
        float m_MouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;

        // ===== Input State =====
        struct InputState {
            bool Left = false;
            bool Right = false;
            bool Up = false;
            bool Down = false;
            bool RotateLeft = false;
            bool RotateRight = false;

            // Helper methods
            void Reset() {
                Left = Right = Up = Down = RotateLeft = RotateRight = false;
            }

            bool HasMovement() const {
                return Left || Right || Up || Down;
            }

            bool HasRotation() const {
                return RotateLeft || RotateRight;
            }
        } m_KeyState;

        // ===== Mouse State =====
        bool m_MousePanning = false;
        glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };

        // ===== Camera Bounds =====
        bool m_BoundsEnabled = false;
        float m_BoundsLeft = -10.0f;
        float m_BoundsRight = 10.0f;
        float m_BoundsBottom = -10.0f;
        float m_BoundsTop = 10.0f;

        // ===== Zoom Limits =====
        float m_MinZoomLevel = 0.25f;
        float m_MaxZoomLevel = 10.0f;

        // ===== Camera Instance (initialized last) =====
        OrthographicCamera m_Camera;

        // ===== Constants =====
        static constexpr float DEFAULT_CAMERA_SPEED = 5.0f;
        static constexpr float DEFAULT_ROTATION_SPEED = 180.0f;
        static constexpr float DEFAULT_ZOOM_SPEED = 0.25f;
        static constexpr float DEFAULT_MOUSE_SENSITIVITY = 0.002f;

        // Additional useful constants
        static constexpr float MIN_ZOOM_LIMIT = 0.01f;
        static constexpr float MAX_ZOOM_LIMIT = 100.0f;
        static constexpr float DEFAULT_MIN_ZOOM = 0.25f;
        static constexpr float DEFAULT_MAX_ZOOM = 10.0f;
    };

} // namespace Cherry
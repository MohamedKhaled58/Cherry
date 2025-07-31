// Cherry/src/Cherry/Scene/EditorCamera.h (Create this file - basic implementation)
#pragma once
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Events/Event.h"
#include "Cherry/Events/MouseEvent.h"
#include "Cherry/Core/TimeStep.h"
#include <glm/glm.hpp>

namespace Cherry {

    class EditorCamera : public OrthographicCamera {
    public:
        EditorCamera() = default;
        EditorCamera(float left, float right, float bottom, float top)
            : OrthographicCamera(left, right, bottom, top, -1.0f, 1.0f) {
        }

        void OnUpdate(TimeStep ts) {
            // TODO: Implement editor camera controls
        }

        void OnEvent(Event& e) {
            // TODO: Handle editor camera events
        }

        float GetDistance() const { return m_Distance; }
        void SetDistance(float distance) { m_Distance = distance; }

        void SetViewportSize(float width, float height) {
            m_ViewportWidth = width;
            m_ViewportHeight = height;
            UpdateProjection();
        }

        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        glm::mat4 GetViewProjection() const { return GetProjectionMatrix() * m_ViewMatrix; }

        glm::vec3 GetUpDirection() const { return glm::vec3(0.0f, 1.0f, 0.0f); }
        glm::vec3 GetRightDirection() const { return glm::vec3(1.0f, 0.0f, 0.0f); }
        glm::vec3 GetForwardDirection() const { return glm::vec3(0.0f, 0.0f, -1.0f); }

        const glm::vec3& GetPosition() const { return m_Position; }

    private:
        void UpdateProjection() {
            m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
            SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
        }

        void UpdateView() {
            m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position);
            m_ViewMatrix = glm::inverse(m_ViewMatrix);
        }

    private:
        float m_AspectRatio = 1.778f;
        float m_ZoomLevel = 1.0f;
        float m_Distance = 10.0f;

        glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };

        float m_ViewportWidth = 1280, m_ViewportHeight = 720;
    };

} // namespace Cherry
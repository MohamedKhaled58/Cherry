#pragma once
#include <glm/glm.hpp>

namespace Cherry {
    class OrthographicCamera {
    public:
        OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar);

        // Position
        const glm::vec3& GetPosition() const { return m_CameraPosition; }
        void SetPosition(const glm::vec3& position) { m_CameraPosition = position; ReCalculateViewMatrix(); }

        // Rotation (Z-axis only)
        float GetRotation() const { return m_CameraRotation; }
        void SetRotation(float rotation) { m_CameraRotation = rotation; ReCalculateViewMatrix(); }

        // Matrices
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

        // Near/Far planes
        void SetNear(float znear) { m_zNear = znear; ReCalculateProjectionMatrix(); }
        float GetNear() const { return m_zNear; }
        void SetFar(float zfar) { m_zFar = zfar; ReCalculateProjectionMatrix(); }
        float GetFar() const { return m_zFar; }

        // Projection bounds
        float GetLeft() const { return m_cLeft; }
        float GetRight() const { return m_cRight; }
        float GetBottom() const { return m_cBottom; }
        float GetTop() const { return m_cTop; }

        // Update projection
        void SetProjection(float left, float right, float bottom, float top);
        void SetProjection(float left, float right, float bottom, float top, float znear, float zfar);

        // Utility methods
        void Translate(const glm::vec3& delta);
        void Rotate(float deltaRotation);
        void Zoom(float factor); // Scales the projection bounds
    private:
        void ReCalculateViewMatrix();
        void ReCalculateProjectionMatrix();

    private:
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

        float m_zNear = 0.0f;
        float m_zFar = 0.0f;
        float m_cLeft, m_cRight, m_cBottom, m_cTop;

        glm::vec3 m_CameraPosition = glm::vec3(0.0f);
        float m_CameraRotation = 0.0f;
    };
}

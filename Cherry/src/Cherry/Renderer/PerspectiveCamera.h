#pragma once
#include <glm/glm.hpp>

namespace Cherry {
    class PerspectiveCamera {
    public:
        PerspectiveCamera(float fov, float aspectRatio, float znear, float zfar);

        // Position
        const glm::vec3& GetPosition() const { return m_Position; }
        void SetPosition(const glm::vec3& position);

        // Rotation (Euler angles in degrees: Pitch, Yaw, Roll)
        const glm::vec3& GetRotation() const { return m_Rotation; }
        void SetRotation(const glm::vec3& rotation);

        // Direction vectors
        glm::vec3 GetForward() const;
        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;

        // Matrices
        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

        // FOV
        void SetFOV(float fov);
        float GetFOV() const { return m_FOV; }

        // Aspect Ratio
        void SetAspectRatio(float aspectRatio);
        float GetAspectRatio() const { return m_AspectRatio; }

        // Near/Far planes
        void SetNear(float znear);
        float GetNear() const { return m_zNear; }
        void SetFar(float zfar);
        float GetFar() const { return m_zFar; }

        // Utility
        void Translate(const glm::vec3& delta);
        void Rotate(const glm::vec3& delta);

    private:
        void ReCalculateViewMatrix();
        void ReCalculateProjectionMatrix();

    private:
        glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);

        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::vec3 m_Rotation = glm::vec3(0.0f); // Pitch, Yaw, Roll (in degrees)

        float m_FOV = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_zNear = 0.1f;
        float m_zFar = 1000.0f;
    };
}


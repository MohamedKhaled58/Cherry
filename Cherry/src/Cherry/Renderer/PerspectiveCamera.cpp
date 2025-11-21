#include "CHpch.h"
#include "PerspectiveCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Cherry {
    PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float znear, float zfar)
        : m_FOV(fov)
        , m_AspectRatio(aspectRatio)
        , m_zNear(znear)
        , m_zFar(zfar)
    {
        CH_PROFILE_FUNCTION();
        ReCalculateProjectionMatrix();
        ReCalculateViewMatrix();
        CH_CORE_INFO("Perspective Camera Initialized!");
    }

    void PerspectiveCamera::SetPosition(const glm::vec3& position) {
        m_Position = position;
        ReCalculateViewMatrix();
    }

    void PerspectiveCamera::SetRotation(const glm::vec3& rotation) {
        m_Rotation = rotation;
        // Clamp pitch to prevent gimbal lock
        m_Rotation.x = glm::clamp(m_Rotation.x, -89.0f, 89.0f);
        ReCalculateViewMatrix();
    }

    glm::vec3 PerspectiveCamera::GetForward() const {
        float yaw = glm::radians(m_Rotation.y);
        float pitch = glm::radians(m_Rotation.x);

        glm::vec3 forward;
        forward.x = cos(yaw) * cos(pitch);
        forward.y = sin(pitch);
        forward.z = sin(yaw) * cos(pitch);
        return glm::normalize(forward);
    }

    glm::vec3 PerspectiveCamera::GetRight() const {
        return glm::normalize(glm::cross(GetForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 PerspectiveCamera::GetUp() const {
        return glm::normalize(glm::cross(GetRight(), GetForward()));
    }

    void PerspectiveCamera::SetFOV(float fov) {
        m_FOV = glm::clamp(fov, 1.0f, 120.0f);
        ReCalculateProjectionMatrix();
    }

    void PerspectiveCamera::SetAspectRatio(float aspectRatio) {
        m_AspectRatio = aspectRatio;
        ReCalculateProjectionMatrix();
    }

    void PerspectiveCamera::SetNear(float znear) {
        m_zNear = znear;
        ReCalculateProjectionMatrix();
    }

    void PerspectiveCamera::SetFar(float zfar) {
        m_zFar = zfar;
        ReCalculateProjectionMatrix();
    }

    void PerspectiveCamera::Translate(const glm::vec3& delta) {
        m_Position += delta;
        ReCalculateViewMatrix();
    }

    void PerspectiveCamera::Rotate(const glm::vec3& delta) {
        m_Rotation += delta;
        m_Rotation.x = glm::clamp(m_Rotation.x, -89.0f, 89.0f);
        ReCalculateViewMatrix();
    }

    void PerspectiveCamera::ReCalculateViewMatrix() {
        CH_PROFILE_FUNCTION();

        glm::vec3 forward = GetForward();
        glm::vec3 right = GetRight();
        glm::vec3 up = GetUp();

        // Look-at matrix
        m_ViewMatrix = glm::lookAt(m_Position, m_Position + forward, up);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void PerspectiveCamera::ReCalculateProjectionMatrix() {
        CH_PROFILE_FUNCTION();

        m_ProjectionMatrix = glm::perspective(
            glm::radians(m_FOV),
            m_AspectRatio,
            m_zNear,
            m_zFar
        );
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }
}


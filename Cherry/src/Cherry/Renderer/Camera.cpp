#include "Chpch.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Cherry {
    OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar)
        : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, znear, zfar)),
        m_ViewMatrix(1.0f),
        m_zNear(znear),
        m_zFar(zfar),
        m_cLeft(left),
        m_cRight(right),
        m_cBottom(bottom),
        m_cTop(top)
    {
        CH_PROFILE_FUNCTION();

        CH_CORE_INFO("Orthographic Camera Initialized!");
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::ReCalculateViewMatrix()
    {
        CH_PROFILE_FUNCTION();

        // Create transformation matrix (translation * rotation)
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_CameraPosition);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(m_CameraRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 transform = translation * rotation;

        // View matrix is the inverse of the transformation
        m_ViewMatrix = glm::inverse(transform);

        // Update view-projection matrix
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::ReCalculateProjectionMatrix()
    {
        CH_PROFILE_FUNCTION();

        m_ProjectionMatrix = glm::ortho(m_cLeft, m_cRight, m_cBottom, m_cTop, m_zNear, m_zFar);
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
    {
        CH_PROFILE_FUNCTION();

        m_cLeft = left;
        m_cRight = right;
        m_cBottom = bottom;
        m_cTop = top;
        ReCalculateProjectionMatrix();
    }

    void OrthographicCamera::SetProjection(float left, float right, float bottom, float top, float znear, float zfar)
    {
        CH_PROFILE_FUNCTION();

        m_cLeft = left;
        m_cRight = right;
        m_cBottom = bottom;
        m_cTop = top;
        m_zNear = znear;
        m_zFar = zfar;
        ReCalculateProjectionMatrix();
    }

    void OrthographicCamera::Translate(const glm::vec3& delta)
    {
        m_CameraPosition += delta;
        ReCalculateViewMatrix();
    }

    void OrthographicCamera::Rotate(float deltaRotation)
    {
        m_CameraRotation += deltaRotation;
        ReCalculateViewMatrix();
    }

    void OrthographicCamera::Zoom(float factor)
    {
        float centerX = (m_cLeft + m_cRight) * 0.5f;
        float centerY = (m_cBottom + m_cTop) * 0.5f;
        float halfWidth = (m_cRight - m_cLeft) * 0.5f * factor;
        float halfHeight = (m_cTop - m_cBottom) * 0.5f * factor;

        m_cLeft = centerX - halfWidth;
        m_cRight = centerX + halfWidth;
        m_cBottom = centerY - halfHeight;
        m_cTop = centerY + halfHeight;

        ReCalculateProjectionMatrix();
    }
}
#include "CHpch.h"
#include "Cherry/Core/KeyCodes.h"
#include "OrthographicCameraController.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>
#include "Cherry/Core/MouseButtonCodes.h"

namespace Cherry {

    OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool enableRotation)
        : m_AspectRatio(aspectRatio)
        , m_Rotation(enableRotation)
        , m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel, -1.0f, 1.0f)
    {
    }

    void OrthographicCameraController::OnUpdate(TimeStep ts) 
    {
        CH_PROFILE_FUNCTION();

        if (m_KeyState.HasMovement()) {
            UpdateCameraPosition(ts);
        }

        if (m_Rotation && m_KeyState.HasRotation()) {
            UpdateCameraRotation(ts);
        }

        if (m_BoundsEnabled) {
            ApplyBounds();
        }
    }

    void OrthographicCameraController::OnEvent(Event& e) 
    {
        CH_PROFILE_FUNCTION();

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
        dispatcher.Dispatch<WindowResizeEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
        dispatcher.Dispatch<MouseButtonPressedEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnMouseButtonReleased));
        dispatcher.Dispatch<MouseMovedEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnMouseMoved));
        dispatcher.Dispatch<KeyPressedEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnKeyPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(CH_BIND_EVENT_FN(OrthographicCameraController::OnKeyReleased));
    }

    // ===== Configuration Methods =====

    void OrthographicCameraController::SetZoomLevel(float level) {
        // Clamp level within controller limits first
        float newZoom = glm::clamp(level, m_MinZoomLevel, m_MaxZoomLevel);
        // Also ensure it respects global allowed zoom limits
        if (IsZoomLevelValid(newZoom) && newZoom != m_ZoomLevel) {
            m_ZoomLevel = newZoom;
            RecalculateView();
        }
    }

    void OrthographicCameraController::SetZoomLimits(float minZoom, float maxZoom) 
    {
        if (minZoom > MIN_ZOOM_LIMIT && maxZoom <= MAX_ZOOM_LIMIT && minZoom < maxZoom) {
            m_MinZoomLevel = minZoom;
            m_MaxZoomLevel = maxZoom;

            // Clamp current zoom within new limits
            SetZoomLevel(m_ZoomLevel);
        }
    }

    void OrthographicCameraController::SetBounds(float left, float right, float bottom, float top) {
        if (AreBoundsValid(left, right, bottom, top)) {
            m_BoundsLeft = left;
            m_BoundsRight = right;
            m_BoundsBottom = bottom;
            m_BoundsTop = top;
            m_BoundsEnabled = true;
        }
    }

    void OrthographicCameraController::Reset() {
        m_Camera.SetPosition({ 0.0f, 0.0f, 0.0f });
        m_Camera.SetRotation(0.0f);
        m_ZoomLevel = 1.0f;
        RecalculateView();

        m_KeyState.Reset();
        m_MousePanning = false;
        m_LastMousePosition = { 0.0f, 0.0f };
    }

    glm::vec2 OrthographicCameraController::ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize) const {
        glm::vec2 ndc = {};
        ndc.x = (2.0f * screenPos.x) / screenSize.x - 1.0f;
        ndc.y = 1.0f - (2.0f * screenPos.y) / screenSize.y;

        glm::vec2 worldPos;
        worldPos.x = ndc.x * m_AspectRatio * m_ZoomLevel;
        worldPos.y = ndc.y * m_ZoomLevel;

        if (m_Rotation && m_Camera.GetRotation() != 0.0f) {
            float rotation = glm::radians(m_Camera.GetRotation());
            float cos_r = cos(rotation);
            float sin_r = sin(rotation);

            glm::vec2 rotatedPos;
            rotatedPos.x = worldPos.x * cos_r - worldPos.y * sin_r;
            rotatedPos.y = worldPos.x * sin_r + worldPos.y * cos_r;
            worldPos = rotatedPos;
        }

        glm::vec3 cameraPos = m_Camera.GetPosition();
        worldPos.x += cameraPos.x;
        worldPos.y += cameraPos.y;

        return worldPos;
    }

    glm::vec2 OrthographicCameraController::WorldToScreen(const glm::vec3& worldPos, const glm::vec2& screenSize) const {
        glm::vec2 relativePos = { worldPos.x, worldPos.y };
        glm::vec3 cameraPos = m_Camera.GetPosition();

        relativePos.x -= cameraPos.x;
        relativePos.y -= cameraPos.y;

        if (m_Rotation && m_Camera.GetRotation() != 0.0f) {
            float rotation = -glm::radians(m_Camera.GetRotation());
            float cos_r = cos(rotation);
            float sin_r = sin(rotation);

            glm::vec2 rotatedPos;
            rotatedPos.x = relativePos.x * cos_r - relativePos.y * sin_r;
            rotatedPos.y = relativePos.x * sin_r + relativePos.y * cos_r;
            relativePos = rotatedPos;
        }

        glm::vec2 ndc;
        ndc.x = relativePos.x / (m_AspectRatio * m_ZoomLevel);
        ndc.y = relativePos.y / m_ZoomLevel;

        glm::vec2 screenPos;
        screenPos.x = (ndc.x + 1.0f) * screenSize.x * 0.5f;
        screenPos.y = (1.0f - ndc.y) * screenSize.y * 0.5f;

        return screenPos;
    }

    // ===== Private Event Handlers =====

    bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        CH_PROFILE_FUNCTION();

        float oldZoom = m_ZoomLevel;
        m_ZoomLevel -= e.GetYOffset() * m_ZoomSpeed;
        m_ZoomLevel = glm::clamp(m_ZoomLevel, m_MinZoomLevel, m_MaxZoomLevel);

        if (IsZoomLevelValid(m_ZoomLevel) && oldZoom != m_ZoomLevel) {
            RecalculateView();
            return true; // Event consumed
        }
        return false;
    }

    bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e) 
    {
        CH_PROFILE_FUNCTION();

        if (e.GetHeight() > 0) {
            float newAspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
            if (std::abs(newAspectRatio - m_AspectRatio) > 0.001f) {
                m_AspectRatio = newAspectRatio;
                RecalculateView();
            }
            return true; // Event consumed
        }
        return false;
    }

    bool OrthographicCameraController::OnMouseButtonPressed(MouseButtonPressedEvent& e) 
    {
        CH_PROFILE_FUNCTION();

        if (e.GetMouseButton() == CH_MOUSE_BUTTON_3) {
            m_MousePanning = true;
            auto [x, y] = Input::GetMousePosition();
            m_LastMousePosition = { x, y };
            return true;
        }
        return false;
    }

    bool OrthographicCameraController::OnMouseButtonReleased(MouseButtonReleasedEvent& e) 
    {
        CH_PROFILE_FUNCTION();

        if (e.GetMouseButton() == CH_MOUSE_BUTTON_3) {
            m_MousePanning = false;
            return true;
        }
        return false;
    }

    bool OrthographicCameraController::OnMouseMoved(MouseMovedEvent& e) 
    {
        CH_PROFILE_FUNCTION();

        if (m_MousePanning) {
            glm::vec2 mousePos = { e.GetX(), e.GetY() };
            // Changed delta to mousePos - m_LastMousePosition for intuitive panning
            glm::vec2 delta = (mousePos - m_LastMousePosition) * m_MouseSensitivity * m_ZoomLevel;

            if (m_Rotation && m_Camera.GetRotation() != 0.0f) {
                float rotation = glm::radians(m_Camera.GetRotation());
                float cos_r = cos(rotation);
                float sin_r = sin(rotation);

                glm::vec2 rotatedDelta;
                rotatedDelta.x = delta.x * cos_r - delta.y * sin_r;
                rotatedDelta.y = delta.x * sin_r + delta.y * cos_r;
                delta = rotatedDelta;
            }

            glm::vec3 position = m_Camera.GetPosition();
            position.x -= delta.x; // Subtract delta for natural drag direction
            position.y -= delta.y;
            m_Camera.SetPosition(position);

            m_LastMousePosition = mousePos;
            return true;
        }
        return false;
    }

    bool OrthographicCameraController::OnKeyPressed(KeyPressedEvent& e)
    {
        CH_PROFILE_FUNCTION();

        if (e.GetRepeatCount() > 0)
            return false;

        switch (e.GetKeyCode()) {
        case CH_KEY_A:
        case CH_KEY_LEFT:
            m_KeyState.Left = true;
            break;
        case CH_KEY_D:
        case CH_KEY_RIGHT:
            m_KeyState.Right = true;
            break;
        case CH_KEY_W:
        case CH_KEY_UP:
            m_KeyState.Up = true;
            break;
        case CH_KEY_S:
        case CH_KEY_DOWN:
            m_KeyState.Down = true;
            break;
        case CH_KEY_Q:
            if (m_Rotation) m_KeyState.RotateLeft = true;
            break;
        case CH_KEY_E:
            if (m_Rotation) m_KeyState.RotateRight = true;
            break;
        default:
            return false;
        }
        return true;
    }

    bool OrthographicCameraController::OnKeyReleased(KeyReleasedEvent& e) 
    {
        CH_PROFILE_FUNCTION();

        switch (e.GetKeyCode()) {
        case CH_KEY_A:
        case CH_KEY_LEFT:
            m_KeyState.Left = false;
            break;
        case CH_KEY_D:
        case CH_KEY_RIGHT:
            m_KeyState.Right = false;
            break;
        case CH_KEY_W:
        case CH_KEY_UP:
            m_KeyState.Up = false;
            break;
        case CH_KEY_S:
        case CH_KEY_DOWN:
            m_KeyState.Down = false;
            break;
        case CH_KEY_Q:
            m_KeyState.RotateLeft = false;
            break;
        case CH_KEY_E:
            m_KeyState.RotateRight = false;
            break;
        default:
            return false;
        }
        return true;
    }

    // ===== Private Helper Methods =====

    void OrthographicCameraController::UpdateCameraPosition(TimeStep ts) 
    {
        glm::vec3 position = m_Camera.GetPosition();

        float baseVelocity = m_CameraTranslationSpeed * ts;
        float velocity = baseVelocity * m_ZoomLevel;

        glm::vec2 movement(0.0f);
        if (m_KeyState.Left)  movement.x -= velocity;
        if (m_KeyState.Right) movement.x += velocity;
        if (m_KeyState.Down)  movement.y -= velocity;
        if (m_KeyState.Up)    movement.y += velocity;

        if (m_Rotation && m_Camera.GetRotation() != 0.0f && (movement.x != 0.0f || movement.y != 0.0f)) {
            float rotation = glm::radians(m_Camera.GetRotation());
            float cos_r = cos(rotation);
            float sin_r = sin(rotation);

            glm::vec2 rotatedMovement;
            rotatedMovement.x = movement.x * cos_r - movement.y * sin_r;
            rotatedMovement.y = movement.x * sin_r + movement.y * cos_r;
            movement = rotatedMovement;
        }

        position.x += movement.x;
        position.y += movement.y;

        // Optional rounding for floating-point stability; comment/uncomment as needed.
        // position.x = round(position.x * 100.0f) / 100.0f;
        // position.y = round(position.y * 100.0f) / 100.0f;

        m_Camera.SetPosition(position);
    }

    void OrthographicCameraController::UpdateCameraRotation(TimeStep ts) 
    {
        float rotation = m_Camera.GetRotation();
        float rotationSpeed = m_CameraRotationSpeed * ts;

        if (m_KeyState.RotateLeft)
            rotation += rotationSpeed;
        if (m_KeyState.RotateRight)
            rotation -= rotationSpeed;

        rotation = std::fmod(rotation, 360.0f);
        if (rotation < 0.0f)
            rotation += 360.0f;

        m_Camera.SetRotation(rotation);
    }

    void OrthographicCameraController::ApplyBounds() 
    {
        glm::vec3 position = m_Camera.GetPosition();

        float halfWidth = m_AspectRatio * m_ZoomLevel;
        float halfHeight = m_ZoomLevel;

        float effectiveLeft = m_BoundsLeft + halfWidth;
        float effectiveRight = m_BoundsRight - halfWidth;
        float effectiveBottom = m_BoundsBottom + halfHeight;
        float effectiveTop = m_BoundsTop - halfHeight;

        if (effectiveLeft < effectiveRight) {
            position.x = glm::clamp(position.x, effectiveLeft, effectiveRight);
        }
        if (effectiveBottom < effectiveTop) {
            position.y = glm::clamp(position.y, effectiveBottom, effectiveTop);
        }

        m_Camera.SetPosition(position);
    }

    void OrthographicCameraController::RecalculateView() 
    {
        CH_PROFILE_FUNCTION();

        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel,
            -m_ZoomLevel, m_ZoomLevel);
    }

    // ===== Private Validation Methods =====

    bool OrthographicCameraController::IsZoomLevelValid(float zoom) const {
        // You must define MIN_ZOOM_LIMIT and MAX_ZOOM_LIMIT appropriately somewhere
        return zoom > MIN_ZOOM_LIMIT && zoom <= MAX_ZOOM_LIMIT;
    }

    bool OrthographicCameraController::AreBoundsValid(float left, float right, float bottom, float top) const {
        return left < right && bottom < top;
    }

} // namespace Cherry

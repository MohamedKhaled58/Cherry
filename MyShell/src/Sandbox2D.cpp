#include "Cherry.h"
#include "Sandbox2D.h"

#include <Platform/OpenGL/OpenGLShader.h>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <Cherry/Renderer/Renderer.h>

namespace Cherry {
    Sandbox2D::Sandbox2D()
        :Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
    {

    }

    void Sandbox2D::OnAttach()
    {

    }

    void Sandbox2D::OnDetach()
    {

    }

    void Sandbox2D::OnUpdate(TimeStep timeStep)
    {
        //  UPDATE
        m_CameraController.OnUpdate(timeStep);

        //  RENDER
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        RenderCommand::Clear();

        Renderer2D::BeginScene(m_CameraController.GetCamera());

        Renderer2D::DrawQuad({ 0.0f, 0.0f,0.5f }, { 0.2f, 0.2f }, { 0.8f, 0.2f, 0.3f, 1.0f });
        Renderer2D::DrawQuad({ 0.0f, 0.0f,0.8f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });


        Renderer2D::EndScene();

        //TODO : Add These Func (Shader::SetMat4 , Shader::SetFloat4)
        //std::dynamic_pointer_cast<OpenGLShader>(m_FlatColorShader)->Bind();
        //std::dynamic_pointer_cast<OpenGLShader>(m_FlatColorShader)->UploadUniformFloat4("u_Color", m_SquareColor);
    }

    void Sandbox2D::OnImGuiRender()
    {
        ImGui::Begin("Camera Controller");

        // Camera Position Controls
        if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
        {
            glm::vec3 position = m_CameraController.GetCameraPosition();
            ImGui::Text("Position: (%.2f, %.2f, %.2f)", position.x, position.y, position.z);

            float pos[3] = { position.x, position.y, position.z }; // Convert to float array for ImGui
            if (ImGui::SliderFloat3("Position", pos, -20.0f, 20.0f))
            {
                // If the slider changed the values, update the camera
                position = glm::vec3(position.x, position.y, position.z);
                m_CameraController.SetCameraPosition(position);
            }
        }


            float cameraSpeed = m_CameraController.GetCameraSpeed();
            if (ImGui::SliderFloat("Camera Speed", &cameraSpeed, 0.1f, 20.0f))
            {
                m_CameraController.SetCameraSpeed(cameraSpeed);
            }

            if (ImGui::Button("Reset Camera"))
            {
                m_CameraController.Reset();
            }
        

        // Camera Rotation Controls
        if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float rotation = m_CameraController.GetCameraRotation();
            ImGui::Text("Rotation: %.1f°", rotation);

            float rotationSpeed = m_CameraController.GetRotationSpeed();
            if (ImGui::SliderFloat("Rotation Speed", &rotationSpeed, 10.0f, 360.0f))
            {
                m_CameraController.SetRotationSpeed(rotationSpeed);
            }

            bool rotationEnabled = m_CameraController.IsRotationEnabled();
            if (ImGui::Checkbox("Enable Rotation", &rotationEnabled))
            {
                m_CameraController.SetRotationEnabled(rotationEnabled);
            }
        }

        // Camera Zoom Controls
        if (ImGui::CollapsingHeader("Zoom", ImGuiTreeNodeFlags_DefaultOpen))
        {
            float zoomLevel = m_CameraController.GetZoomLevel();
            float minZoom = m_CameraController.GetMinZoom();
            float maxZoom = m_CameraController.GetMaxZoom();

            if (ImGui::SliderFloat("Zoom Level", &zoomLevel, minZoom, maxZoom))
            {
                m_CameraController.SetZoomLevel(zoomLevel);
            }

            float zoomSpeed = m_CameraController.GetZoomSpeed();
            if (ImGui::SliderFloat("Zoom Speed", &zoomSpeed, 0.1f, 2.0f))
            {
                m_CameraController.SetZoomSpeed(zoomSpeed);
            }

            // Zoom limits controls
            ImGui::Separator();
            ImGui::Text("Zoom Limits:");
            float newMinZoom = minZoom;
            float newMaxZoom = maxZoom;

            if (ImGui::DragFloat("Min Zoom", &newMinZoom, 0.01f, 0.01f, maxZoom - 0.01f))
            {
                m_CameraController.SetZoomLimits(newMinZoom, maxZoom);
            }

            if (ImGui::DragFloat("Max Zoom", &newMaxZoom, 0.1f, minZoom + 0.01f, 100.0f))
            {
                m_CameraController.SetZoomLimits(minZoom, newMaxZoom);
            }
        }

        // Camera Bounds Controls
        if (ImGui::CollapsingHeader("Camera Bounds"))
        {
            bool boundsEnabled = m_CameraController.IsBoundsEnabled();
            if (ImGui::Checkbox("Enable Bounds", &boundsEnabled))
            {
                m_CameraController.EnableBounds(boundsEnabled);
            }

            if (boundsEnabled)
            {
                glm::vec4 bounds = m_CameraController.GetBounds();
                float left = bounds.x, right = bounds.y;
                float bottom = bounds.z, top = bounds.w;

                bool boundsChanged = false;
                boundsChanged |= ImGui::DragFloat("Left", &left, 0.1f);
                boundsChanged |= ImGui::DragFloat("Right", &right, 0.1f);
                boundsChanged |= ImGui::DragFloat("Bottom", &bottom, 0.1f);
                boundsChanged |= ImGui::DragFloat("Top", &top, 0.1f);

                if (boundsChanged && left < right && bottom < top)
                {
                    m_CameraController.SetBounds(left, right, bottom, top);
                }
            }
        }

        // Mouse Controls
        if (ImGui::CollapsingHeader("Mouse Controls"))
        {
            float mouseSensitivity = m_CameraController.GetMouseSensitivity();
            if (ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.0001f, 0.01f, "%.4f"))
            {
                m_CameraController.SetMouseSensitivity(mouseSensitivity);
            }

            ImGui::Text("Mouse panning: Hold middle mouse button and drag");
        }

        // Camera Information (Read-only)
        if (ImGui::CollapsingHeader("Camera Info"))
        {
            glm::vec3 pos = m_CameraController.GetCameraPosition();
            float rotation = m_CameraController.GetCameraRotation();
            float zoom = m_CameraController.GetZoomLevel();

            ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
            ImGui::Text("Rotation: %.1f°", rotation);
            ImGui::Text("Zoom Level: %.2f", zoom);
            ImGui::Text("Zoom Range: %.2f - %.2f",
                m_CameraController.GetMinZoom(),
                m_CameraController.GetMaxZoom());

            if (m_CameraController.IsBoundsEnabled())
            {
                glm::vec4 bounds = m_CameraController.GetBounds();
                ImGui::Text("Bounds: L:%.1f R:%.1f B:%.1f T:%.1f",
                    bounds.x, bounds.y, bounds.z, bounds.w);
            }
            else
            {
                ImGui::Text("Bounds: Disabled");
            }
        }

        // Controls Reference
        if (ImGui::CollapsingHeader("Controls"))
        {
            ImGui::Text("Movement:");
            ImGui::BulletText("WASD or Arrow Keys - Move camera");
            ImGui::Text("Rotation:");
            ImGui::BulletText("Q/E - Rotate camera (if enabled)");
            ImGui::Text("Zoom:");
            ImGui::BulletText("Mouse Wheel - Zoom in/out");
            ImGui::Text("Mouse:");
            ImGui::BulletText("Middle Mouse + Drag - Pan camera");
        }

        ImGui::End();

        // Keep your existing settings window
        ImGui::Begin("Render Settings");
        ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
        ImGui::End();
    }

    void Sandbox2D::OnEvent(Event& e)
    {
        m_CameraController.OnEvent(e);
    }
}
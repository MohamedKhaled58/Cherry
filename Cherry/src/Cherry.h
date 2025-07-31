#pragma once

// Core
#include "Cherry/Core/Application.h"
#include "Cherry/Core/Layer.h"
#include "Cherry/Core/Log.h"
#include "Cherry/Core/TimeStep.h"
#include "Cherry/Core/Input.h"
#include "Cherry/Core/KeyCodes.h"
#include "Cherry/Core/MouseButtonCodes.h"
#include "Cherry/Core/Memory.h"
#include "Cherry/Core/Threading.h"
#include "Cherry/Core/Config.h"
#include "Cherry/Core/Debug/Profiler.h"

// ImGui
#include "Cherry/ImGui/ImGuiLayer.h"

// Renderer
#include "Cherry/Renderer/Renderer.h"
#include "Cherry/Renderer/Renderer2D.h"
#include "Cherry/Renderer/BatchRenderer2D.h"
#include "Cherry/Renderer/RenderCommand.h"
#include "Cherry/Renderer/Shader.h"
#include "Cherry/Renderer/Buffer.h"
#include "Cherry/Renderer/Texture.h"
#include "Cherry/Renderer/VertexArray.h"
#include "Cherry/Renderer/Camera.h"
#include "Cherry/Renderer/Sprite2D.h"
#include "Cherry/Renderer/EffectSystem.h"

// Camera Controller
#include "Cherry/OrthographicCameraController.h"

// Scene System
#include "Cherry/Scene/Scene.h"
#include "Cherry/Scene/Entity.h"
#include "Cherry/Scene/Components.h"
#include "Cherry/Scene/SceneManager.h"

// Animation System
#include "Cherry/Animation/AnimationSystem.h"
#include "Cherry/Animation/KeyFrameSystem.h"

// Audio System
#include "Cherry/Audio/Audio.h"
#include "Cherry/Audio/AudioManager.h"

// Resource Management
#include "Cherry/Resources/ResourceManager.h"
#include "Cherry/Resources/PackageManager.h"
#include "Cherry/Assets/AssetManager.h"

// Networking
#include "Cherry/Network/NetworkManager.h"
#include "Cherry/Network/MMONetworking.h"

// Input System
#include "Cherry/Input/InputManager.h"

// UI System
#include "Cherry/UI/UIManager.h"
#include "Cherry/UI/GameUI.h"

// Game Systems
#include "Cherry/Game/GameMap.h"
#include "Cherry/Game/Player.h"
#include "Cherry/Game/ItemSystem.h"
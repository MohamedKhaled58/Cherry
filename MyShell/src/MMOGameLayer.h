// MyShell/src/MMOGameLayer.h
#pragma once
#include "Cherry.h"
#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "Cherry/Network/MMONetworking.h"
#include "Cherry/Renderer/Sprite2D.h"
#include "Cherry/Animation/AnimationSystem.h"
#include "Cherry/Resources/PackageManager.h"
#include <Cherry/Core/Debug/Profiler.h>

namespace Cherry {

    // Main MMO game layer
    class MMOGameLayer : public Layer {
    public:
        MMOGameLayer();
        virtual ~MMOGameLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(TimeStep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& e) override;

    private:
        // Core systems
        OrthographicCameraController m_CameraController;
        std::unique_ptr<SpriteBatch> m_SpriteBatch;


        // Game state
        enum class GameState {
            MainMenu,
            ServerSelection,
            CharacterSelection,
            InGame,
            Trading,
            Disconnected
        };
        GameState m_CurrentState = GameState::MainMenu;

        // Player data
        struct LocalPlayer {
            uint32_t PlayerID = 0;
            std::string Name;
            glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
            glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };
            uint8_t Direction = 0;
            REF(EnhancedSprite2D) Sprite;
            std::unique_ptr<AnimationController> AnimController;

            // Stats
            uint32_t Health = 100;
            uint32_t MaxHealth = 100;
            uint32_t Mana = 100;
            uint32_t MaxMana = 100;
            uint32_t Level = 1;
        } m_LocalPlayer;

        // Other players
        struct RemotePlayer {
            uint32_t PlayerID;
            std::string Name;
            glm::vec3 Position;
            REF(EnhancedSprite2D) Sprite;
            std::unique_ptr<AnimationController> AnimController;
            std::unique_ptr<NetworkInterpolator> Interpolator;
        };
        std::unordered_map<uint32_t, RemotePlayer> m_RemotePlayers;

        // NPCs and world objects
        struct NPC {
            uint32_t NPCID;
            std::string Name;
            glm::vec3 Position;
            REF(EnhancedSprite2D) Sprite;
            std::unique_ptr<AnimationController> AnimController;
        };
        std::unordered_map<uint32_t, NPC> m_NPCs;

        // UI Elements
        struct GameUI {
            bool ShowChat = true;
            bool ShowInventory = false;
            bool ShowCharacterStats = false;
            bool ShowMap = false;

            std::vector<std::string> ChatMessages;
            std::string ChatInput;
            MMOChatPacket::ChatChannel CurrentChatChannel = MMOChatPacket::ChatChannel::Say;
        } m_UI;

        // Input handling
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
        void HandleMovementInput(TimeStep ts);

        // Game state management
        void ChangeGameState(GameState newState);
        void UpdateMainMenu();
        void UpdateServerSelection();
        void UpdateCharacterSelection();
        void UpdateInGame(TimeStep ts);

        // Rendering
        void RenderMainMenu();
        void RenderServerSelection();
        void RenderCharacterSelection();
        void RenderInGame();
        void RenderUI();

        // Network event handlers
        void OnPlayerLogin(uint32_t playerId, const std::string& playerName);
        void OnPlayerLogout(uint32_t playerId);
        void OnPlayerMove(uint32_t playerId, const glm::vec3& position);
        void OnChatMessage(const MMOChatPacket& packet);

        // Resource loading
        void LoadGameResources();
        void LoadPlayerSprites();
        void LoadNPCSprites();
        void LoadMapAssets();

        // Helper functions
        void SpawnRemotePlayer(uint32_t playerId, const std::string& name, const glm::vec3& position);
        void DespawnRemotePlayer(uint32_t playerId);
        void UpdatePlayerAnimation(const std::string& animationName, bool isLocal = true, uint32_t playerId = 0);
        glm::vec2 WorldToScreen(const glm::vec3& worldPos);
        glm::vec3 ScreenToWorld(const glm::vec2& screenPos);
    };

    // Implementation of MMO Game Layer
    MMOGameLayer::MMOGameLayer()
        : Layer("MMOGameLayer"), m_CameraController(1280.0f / 720.0f, true) {
    }

    void MMOGameLayer::OnAttach() {
        CH_PROFILE_FUNCTION();

        // Initialize core systems
        m_SpriteBatch = std::make_unique<SpriteBatch>(5000);

        // Initialize MMO client
        MMOClient::Get().Initialize();

        // Setup network event handlers
        auto& networkManager = MMONetworkManager::Get();
        networkManager.SetOnPlayerLogin([this](uint32_t id, const std::string& name) {
            OnPlayerLogin(id, name);
            });
        networkManager.SetOnPlayerLogout([this](uint32_t id) {
            OnPlayerLogout(id);
            });
        networkManager.SetOnPlayerMove([this](uint32_t id, const glm::vec3& pos) {
            OnPlayerMove(id, pos);
            });
        networkManager.SetOnChatMessage([this](const MMOChatPacket& packet) {
            OnChatMessage(packet);
            });

        // Load game packages
        EnhancedPackageManager::Get().LoadPackage("assets/packages/characters.cpkg");
        EnhancedPackageManager::Get().LoadPackage("assets/packages/maps.cpkg");
        EnhancedPackageManager::Get().LoadPackage("assets/packages/ui.cpkg");

        LoadGameResources();

        CH_CORE_INFO("MMO Game Layer initialized");
    }

    void MMOGameLayer::OnDetach() {
        CH_PROFILE_FUNCTION();

        // Cleanup
        MMOClient::Get().Shutdown();
        m_SpriteBatch.reset();

        CH_CORE_INFO("MMO Game Layer detached");
    }

    void MMOGameLayer::OnUpdate(TimeStep ts) {
        CH_PROFILE_FUNCTION();

        // Update camera
        m_CameraController.OnUpdate(ts);

        // Update MMO client
        MMOClient::Get().Update(ts);

        // Handle input
        HandleMovementInput(ts);

        // Update based on current state
        switch (m_CurrentState) {
        case GameState::MainMenu:
            UpdateMainMenu();
            break;
        case GameState::ServerSelection:
            UpdateServerSelection();
            break;
        case GameState::CharacterSelection:
            UpdateCharacterSelection();
            break;
        case GameState::InGame:
            UpdateInGame(ts);
            break;
        default:
            break;
        }

        // Render everything
        RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        RenderCommand::Clear();

        m_SpriteBatch->Begin(m_CameraController.GetCamera().GetViewProjectionMatrix());

        switch (m_CurrentState) {
        case GameState::MainMenu:
            RenderMainMenu();
            break;
        case GameState::ServerSelection:
            RenderServerSelection();
            break;
        case GameState::CharacterSelection:
            RenderCharacterSelection();
            break;
        case GameState::InGame:
            RenderInGame();
            break;
        default:
            break;
        }

        m_SpriteBatch->End();
    }

    void MMOGameLayer::OnImGuiRender() {
        RenderUI();
    }

    void MMOGameLayer::OnEvent(Event& e) {
        m_CameraController.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(CH_BIND_EVENT_FN(MMOGameLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(CH_BIND_EVENT_FN(MMOGameLayer::OnMouseButtonPressed));
    }

    bool MMOGameLayer::OnKeyPressed(KeyPressedEvent& e) {
        switch (e.GetKeyCode()) {
        case CH_KEY_ENTER:
            if (m_CurrentState == GameState::InGame && !m_UI.ChatInput.empty()) {
                MMOClient::Get().SendChatMessage(m_UI.ChatInput, m_UI.CurrentChatChannel);
                m_UI.ChatInput.clear();
                return true;
            }
            break;
        case CH_KEY_TAB:
            if (m_CurrentState == GameState::InGame) {
                // Cycle through chat channels
                auto currentChannel = static_cast<int>(m_UI.CurrentChatChannel);
                currentChannel = (currentChannel + 1) % 5; // 0-4 channels
                m_UI.CurrentChatChannel = static_cast<MMOChatPacket::ChatChannel>(currentChannel);
                return true;
            }
            break;
        case CH_KEY_I:
            if (m_CurrentState == GameState::InGame) {
                m_UI.ShowInventory = !m_UI.ShowInventory;
                return true;
            }
            break;
        case CH_KEY_C:
            if (m_CurrentState == GameState::InGame) {
                m_UI.ShowCharacterStats = !m_UI.ShowCharacterStats;
                return true;
            }
            break;
        case CH_KEY_M:
            if (m_CurrentState == GameState::InGame) {
                m_UI.ShowMap = !m_UI.ShowMap;
                return true;
            }
            break;
        }
        return false;
    }

    bool MMOGameLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e) {
        if (m_CurrentState == GameState::InGame && e.GetMouseButton() == CH_MOUSE_BUTTON_LEFT) {
            // Handle click-to-move
            auto [mouseX, mouseY] = Input::GetMousePosition();
            glm::vec3 worldPos = ScreenToWorld({ mouseX, mouseY });
            MMOClient::Get().MovePlayer(worldPos);
            return true;
        }
        return false;
    }

    void MMOGameLayer::HandleMovementInput(TimeStep ts) {
        if (m_CurrentState != GameState::InGame) return;

        glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
        bool isMoving = false;

        if (Input::IsKeyPressed(CH_KEY_W) || Input::IsKeyPressed(CH_KEY_UP)) {
            velocity.y += 1.0f;
            isMoving = true;
        }
        if (Input::IsKeyPressed(CH_KEY_S) || Input::IsKeyPressed(CH_KEY_DOWN)) {
            velocity.y -= 1.0f;
            isMoving = true;
        }
        if (Input::IsKeyPressed(CH_KEY_A) || Input::IsKeyPressed(CH_KEY_LEFT)) {
            velocity.x -= 1.0f;
            isMoving = true;
        }
        if (Input::IsKeyPressed(CH_KEY_D) || Input::IsKeyPressed(CH_KEY_RIGHT)) {
            velocity.x += 1.0f;
            isMoving = true;
        }

        if (isMoving) {
            // Normalize velocity
            if (glm::length(velocity) > 0.0f) {
                velocity = glm::normalize(velocity);
            }

            // Apply movement speed
            const float movementSpeed = 5.0f;
            velocity *= movementSpeed;

            // Update local position
            m_LocalPlayer.Position += velocity * (float)ts;
            m_LocalPlayer.Velocity = velocity;

            // Update camera to follow player
            m_CameraController.SetCameraPosition(m_LocalPlayer.Position);

            // Send movement update to server
            uint8_t direction = 0; // Calculate direction based on velocity
            if (velocity.x > 0) direction |= 1; // Right
            if (velocity.x < 0) direction |= 2; // Left
            if (velocity.y > 0) direction |= 4; // Up
            if (velocity.y < 0) direction |= 8; // Down

            // TODO: Implement movement networking
            // MMONetworkManager::Get().SendMovementUpdate(m_LocalPlayer.Position, velocity, direction);

            // Update animation
            UpdatePlayerAnimation("walk", true);
        }
        else {
            m_LocalPlayer.Velocity = { 0.0f, 0.0f, 0.0f };
            UpdatePlayerAnimation("idle", true);
        }
    }

    void MMOGameLayer::UpdateInGame(TimeStep ts) {
        // Update local player animation
        if (m_LocalPlayer.AnimController) {
            m_LocalPlayer.AnimController->Update(ts);
        }

        // Update remote players
        for (auto& [playerId, player] : m_RemotePlayers) {
            if (player.AnimController) {
                player.AnimController->Update(ts);
            }
            if (player.Interpolator) {
                player.Interpolator->Update(ts);
                auto interpolatedState = player.Interpolator->GetInterpolatedState();
                player.Position = interpolatedState.Position;
            }
        }

        // Update NPCs
        for (auto& [npcId, npc] : m_NPCs) {
            if (npc.AnimController) {
                npc.AnimController->Update(ts);
            }
        }

        // Update world synchronizer
        MMOClient::Get().GetWorldSynchronizer().Update(ts);
    }

    void MMOGameLayer::RenderInGame() {
        // Render map background (if loaded)
        // TODO: Implement tile-based map rendering

        // Render NPCs
        for (const auto& [npcId, npc] : m_NPCs) {
            if (npc.Sprite) {
                npc.Sprite->SetPosition({ npc.Position.x, npc.Position.y });
                m_SpriteBatch->Draw(npc.Sprite);
            }
        }

        // Render remote players
        for (const auto& [playerId, player] : m_RemotePlayers) {
            if (player.Sprite) {
                player.Sprite->SetPosition({ player.Position.x, player.Position.y });
                m_SpriteBatch->Draw(player.Sprite);
            }
        }

        // Render local player
        if (m_LocalPlayer.Sprite) {
            m_LocalPlayer.Sprite->SetPosition({ m_LocalPlayer.Position.x, m_LocalPlayer.Position.y });
            m_SpriteBatch->Draw(m_LocalPlayer.Sprite);
        }
    }

    void MMOGameLayer::RenderMainMenu() {
        // Simple main menu
        // TODO: Create proper UI sprites for menu

        // For now, just render a simple background
        // This would be replaced with actual menu sprites from packages
    }

    void MMOGameLayer::RenderServerSelection() {
        // Render server list
        // TODO: Implement server selection UI
    }

    void MMOGameLayer::RenderCharacterSelection() {
        // Render character selection screen
        // TODO: Implement character selection UI
    }

    void MMOGameLayer::RenderUI() {
        if (m_CurrentState == GameState::InGame) {
            // Chat window
            if (m_UI.ShowChat) {
                ImGui::Begin("Chat");

                // Chat messages
                ImGui::BeginChild("ChatMessages", ImVec2(0, -25));
                for (const auto& message : m_UI.ChatMessages) {
                    ImGui::TextWrapped("%s", message.c_str());
                }
                ImGui::EndChild();

                // Chat input
                ImGui::Separator();
                ImGui::PushItemWidth(-50);
                ImGui::InputText("##ChatInput", &m_UI.ChatInput);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button("Send") && !m_UI.ChatInput.empty()) {
                    MMOClient::Get().SendChatMessage(m_UI.ChatInput, m_UI.CurrentChatChannel);
                    m_UI.ChatInput.clear();
                }

                // Channel selection
                const char* channels[] = { "Say", "Whisper", "Party", "Guild", "World" };
                int currentChannel = static_cast<int>(m_UI.CurrentChatChannel);
                if (ImGui::Combo("Channel", &currentChannel, channels, 5)) {
                    m_UI.CurrentChatChannel = static_cast<MMOChatPacket::ChatChannel>(currentChannel);
                }

                ImGui::End();
            }

            // Character stats
            if (m_UI.ShowCharacterStats) {
                ImGui::Begin("Character Stats", &m_UI.ShowCharacterStats);

                ImGui::Text("Name: %s", m_LocalPlayer.Name.c_str());
                ImGui::Text("Level: %d", m_LocalPlayer.Level);
                ImGui::Text("Health: %d/%d", m_LocalPlayer.Health, m_LocalPlayer.MaxHealth);
                ImGui::Text("Mana: %d/%d", m_LocalPlayer.Mana, m_LocalPlayer.MaxMana);
                ImGui::Text("Position: (%.1f, %.1f, %.1f)",
                    m_LocalPlayer.Position.x, m_LocalPlayer.Position.y, m_LocalPlayer.Position.z);

                ImGui::End();
            }

            // Inventory
            if (m_UI.ShowInventory) {
                ImGui::Begin("Inventory", &m_UI.ShowInventory);

                // TODO: Implement inventory grid
                ImGui::Text("Inventory items would be displayed here");

                ImGui::End();
            }

            // Mini-map
            if (m_UI.ShowMap) {
                ImGui::Begin("Map", &m_UI.ShowMap);

                // TODO: Implement minimap rendering
                ImGui::Text("Minimap would be displayed here");

                ImGui::End();
            }

            // Network statistics
            ImGui::Begin("Network Stats");
            auto stats = MMOClient::Get().GetNetworkStats();
            ImGui::Text("Connected Players: %zu", stats.ConnectedPlayers);
            ImGui::Text("Messages/sec: %zu", stats.MessagesPerSecond);
            ImGui::Text("Movement Updates/sec: %zu", stats.MovementUpdatesPerSecond);
            ImGui::Text("Average Latency: %.1fms", stats.AverageLatency);
            ImGui::Text("Cheat Attempts: %zu", stats.CheatAttempts);
            ImGui::End();
        }

        // Game state switching for testing
        ImGui::Begin("Game State");
        const char* states[] = { "Main Menu", "Server Selection", "Character Selection", "In Game", "Trading", "Disconnected" };
        int currentState = static_cast<int>(m_CurrentState);
        if (ImGui::Combo("Current State", &currentState, states, 6)) {
            ChangeGameState(static_cast<GameState>(currentState));
        }

        if (ImGui::Button("Connect to Test Server")) {
            MMOClient::Get().ConnectToServer("localhost");
        }

        if (ImGui::Button("Login Test User")) {
            MMOClient::Get().Login("testuser", "testpass");
        }

        ImGui::End();
    }

    void MMOGameLayer::OnPlayerLogin(uint32_t playerId, const std::string& playerName) {
        CH_CORE_INFO("Player logged in: {} (ID: {})", playerName, playerId);

        // Add to chat
        std::string message = playerName + " has joined the game.";
        m_UI.ChatMessages.push_back(message);

        // Spawn remote player sprite
        SpawnRemotePlayer(playerId, playerName, { 0.0f, 0.0f, 0.0f });
    }

    void MMOGameLayer::OnPlayerLogout(uint32_t playerId) {
        auto it = m_RemotePlayers.find(playerId);
        if (it != m_RemotePlayers.end()) {
            CH_CORE_INFO("Player logged out: {} (ID: {})", it->second.Name, playerId);

            // Add to chat
            std::string message = it->second.Name + " has left the game.";
            m_UI.ChatMessages.push_back(message);

            DespawnRemotePlayer(playerId);
        }
    }

    void MMOGameLayer::OnPlayerMove(uint32_t playerId, const glm::vec3& position) {
        auto it = m_RemotePlayers.find(playerId);
        if (it != m_RemotePlayers.end()) {
            // Add to interpolator for smooth movement
            if (it->second.Interpolator) {
                it->second.Interpolator->AddState(position, { 0.0f, 0.0f, 0.0f });
            }

            // Update animation based on movement
            bool isMoving = glm::length(position - it->second.Position) > 0.1f;
            UpdatePlayerAnimation(isMoving ? "walk" : "idle", false, playerId);
        }
    }

    void MMOGameLayer::OnChatMessage(const MMOChatPacket& packet) {
        // Format chat message
        std::string channelName;
        switch (packet.Channel) {
        case MMOChatPacket::ChatChannel::Say: channelName = "[Say]"; break;
        case MMOChatPacket::ChatChannel::Whisper: channelName = "[Whisper]"; break;
        case MMOChatPacket::ChatChannel::Party: channelName = "[Party]"; break;
        case MMOChatPacket::ChatChannel::Guild: channelName = "[Guild]"; break;
        case MMOChatPacket::ChatChannel::World: channelName = "[World]"; break;
        case MMOChatPacket::ChatChannel::System: channelName = "[System]"; break;
        case MMOChatPacket::ChatChannel::Admin: channelName = "[Admin]"; break;
        }

        std::string formattedMessage = channelName + " " + packet.SenderName + ": " + packet.Message;
        m_UI.ChatMessages.push_back(formattedMessage);

        // Keep only last 100 messages
        if (m_UI.ChatMessages.size() > 100) {
            m_UI.ChatMessages.erase(m_UI.ChatMessages.begin());
        }
    }

    void MMOGameLayer::LoadGameResources() {
        CH_PROFILE_FUNCTION();

        // Load textures from packages
        LoadPlayerSprites();
        LoadNPCSprites();
        LoadMapAssets();

        // Create local player sprite
        m_LocalPlayer.Sprite = SpriteManager::Get().CreateSprite("character/player_base.png");
        if (!m_LocalPlayer.Sprite) {
            // Fallback: create a simple colored sprite
            auto whiteTexture = Texture2D::Create(32, 32);
            uint32_t whiteData = 0xFFFFFFFF;
            whiteTexture->SetData(&whiteData, sizeof(uint32_t));
            m_LocalPlayer.Sprite = SpriteManager::Get().CreateSprite(whiteTexture);
            m_LocalPlayer.Sprite->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f }); // Green for local player
        }

        // Setup animations
        auto& animSystem = EnhancedAnimationSystem::Get();
        m_LocalPlayer.AnimController = std::make_unique<AnimationController>();

        // Create basic animations
        auto idleAnim = animSystem.CreateAnimation("player_idle", 30); // 30 frames
        auto walkAnim = animSystem.CreateAnimation("player_walk", 60); // 60 frames

        m_LocalPlayer.AnimController->AddAnimation("idle", idleAnim);
        m_LocalPlayer.AnimController->AddAnimation("walk", walkAnim);
        m_LocalPlayer.AnimController->PlayAnimation("idle", true);

        CH_CORE_INFO("Game resources loaded successfully");
    }

    void MMOGameLayer::LoadPlayerSprites() {
        // Load player character sprites from packages
        // This would load various character classes, equipment, etc.

        // Example: Loading from enhanced package system
        uint32_t size;
        void* spriteData = EnhancedPackageManager::Get().LoadFile("characters/warrior_male.sprite", size);
        if (spriteData) {
            // Process sprite data
            free(spriteData);
        }
    }

    void MMOGameLayer::LoadNPCSprites() {
        // Load NPC sprites
        // Similar to player sprites but for NPCs
    }

    void MMOGameLayer::LoadMapAssets() {
        // Load map tiles, objects, etc.
    }

    void MMOGameLayer::SpawnRemotePlayer(uint32_t playerId, const std::string& name, const glm::vec3& position) {
        RemotePlayer player;
        player.PlayerID = playerId;
        player.Name = name;
        player.Position = position;

        // Create sprite (blue for remote players)
        auto whiteTexture = Texture2D::Create(32, 32);
        uint32_t whiteData = 0xFFFFFFFF;
        whiteTexture->SetData(&whiteData, sizeof(uint32_t));
        player.Sprite = SpriteManager::Get().CreateSprite(whiteTexture);
        player.Sprite->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f }); // Blue for remote players

        // Create animation controller
        player.AnimController = std::make_unique<AnimationController>();
        auto& animSystem = EnhancedAnimationSystem::Get();
        auto idleAnim = animSystem.CreateAnimation("remote_idle", 30);
        auto walkAnim = animSystem.CreateAnimation("remote_walk", 60);
        player.AnimController->AddAnimation("idle", idleAnim);
        player.AnimController->AddAnimation("walk", walkAnim);
        player.AnimController->PlayAnimation("idle", true);

        // Create interpolator for smooth movement
        player.Interpolator = std::make_unique<NetworkInterpolator>(0.1f);

        m_RemotePlayers[playerId] = std::move(player);
    }

    void MMOGameLayer::DespawnRemotePlayer(uint32_t playerId) {
        m_RemotePlayers.erase(playerId);
    }

    void MMOGameLayer::UpdatePlayerAnimation(const std::string& animationName, bool isLocal, uint32_t playerId) {
        if (isLocal) {
            if (m_LocalPlayer.AnimController) {
                m_LocalPlayer.AnimController->PlayAnimation(animationName, true);
            }
        }
        else {
            auto it = m_RemotePlayers.find(playerId);
            if (it != m_RemotePlayers.end() && it->second.AnimController) {
                it->second.AnimController->PlayAnimation(animationName, true);
            }
        }
    }

    void MMOGameLayer::ChangeGameState(GameState newState) {
        if (m_CurrentState == newState) return;

        CH_CORE_INFO("Changing game state from {} to {}",
            static_cast<int>(m_CurrentState), static_cast<int>(newState));

        m_CurrentState = newState;

        // Handle state-specific initialization
        switch (newState) {
        case GameState::InGame:
            // Initialize game world
            m_LocalPlayer.Position = { 0.0f, 0.0f, 0.0f };
            m_CameraController.SetCameraPosition(m_LocalPlayer.Position);
            break;
        default:
            break;
        }
    }

    glm::vec2 MMOGameLayer::WorldToScreen(const glm::vec3& worldPos) {
        // Convert world coordinates to screen coordinates
        // This is a simplified version - you'd use the actual view-projection matrix
        auto screenSize = glm::vec2(1280.0f, 720.0f); // Get from application
        return m_CameraController.WorldToScreen(worldPos, screenSize);
    }

    glm::vec3 MMOGameLayer::ScreenToWorld(const glm::vec2& screenPos) {
        // Convert screen coordinates to world coordinates
        auto screenSize = glm::vec2(1280.0f, 720.0f); // Get from application
        auto worldPos2D = m_CameraController.ScreenToWorld(screenPos, screenSize);
        return { worldPos2D.x, worldPos2D.y, 0.0f };
    }

    void MMOGameLayer::UpdateMainMenu() {
        // Handle main menu logic
    }

    void MMOGameLayer::UpdateServerSelection() {
        // Handle server selection logic
    }

    void MMOGameLayer::UpdateCharacterSelection() {
        // Handle character selection logic
    }

} // namespace Cherry


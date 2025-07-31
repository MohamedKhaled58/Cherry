#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Network/NetworkManager.h"
#include "ConquerPackets.h"
#include <functional>
#include <unordered_map>
#include <queue>

namespace Cherry {

    // Network event callbacks
    using ConquerPacketHandler = std::function<void(std::unique_ptr<ConquerPacket>)>;
    using ConnectionCallback = std::function<void(bool connected, const std::string& error)>;

    // Connection states
    enum class ConquerConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Authenticating,
        InGame,
        Error
    };

    // Conquer Online specific network manager
    class ConquerNetworkManager {
    public:
        static ConquerNetworkManager& Get() {
            static ConquerNetworkManager instance;
            return instance;
        }

        // Connection management
        bool Connect(const std::string& serverIP, uint16_t port, uint32_t timeout = 10000);
        void Disconnect();
        bool IsConnected() const {
            return m_State == ConquerConnectionState::Connected ||
                m_State == ConquerConnectionState::InGame;
        }
        ConquerConnectionState GetState() const { return m_State; }

        // Authentication
        bool Login(const std::string& username, const std::string& password, const std::string& server = "");
        bool SelectCharacter(uint32_t characterID);

        // Packet handling
        void SendPacket(std::unique_ptr<ConquerPacket> packet);
        void RegisterPacketHandler(ConquerPacketType type, ConquerPacketHandler handler);
        void UnregisterPacketHandler(ConquerPacketType type);

        // Update (call every frame)
        void Update();

        // Callbacks
        void SetConnectionCallback(ConnectionCallback callback) { m_ConnectionCallback = callback; }

        // Game state
        uint32_t GetPlayerID() const { return m_PlayerID; }
        const std::string& GetPlayerName() const { return m_PlayerName; }

        // Statistics
        struct NetworkStats {
            uint32_t PacketsSent = 0;
            uint32_t PacketsReceived = 0;
            uint32_t BytesSent = 0;
            uint32_t BytesReceived = 0;
            float Latency = 0.0f;
        };
        const NetworkStats& GetStats() const { return m_Stats; }

    private:
        ConquerNetworkManager() = default;
        ~ConquerNetworkManager() { Disconnect(); }

        // Internal packet processing
        void ProcessIncomingPackets();
        void HandlePacket(const uint8_t* data, uint16_t length);

        // Default packet handlers
        void HandleConnect(std::unique_ptr<ConquerPacket> packet);
        void HandleUserInfo(std::unique_ptr<ConquerPacket> packet);
        void HandleTalk(std::unique_ptr<ConquerPacket> packet);
        void HandleWalk(std::unique_ptr<ConquerPacket> packet);
        void HandleAction(std::unique_ptr<ConquerPacket> packet);

        // State management
        void SetState(ConquerConnectionState newState);

    private:
        ConquerConnectionState m_State = ConquerConnectionState::Disconnected;

        // Network core
        NetworkManager* m_NetworkManager = nullptr;

        // Packet handling
        std::unordered_map<ConquerPacketType, ConquerPacketHandler> m_PacketHandlers;
        std::queue<std::vector<uint8_t>> m_IncomingPackets;

        // Game state
        uint32_t m_PlayerID = 0;
        std::string m_PlayerName;

        // Connection info
        std::string m_ServerIP;
        uint16_t m_ServerPort = 0;

        // Callbacks
        ConnectionCallback m_ConnectionCallback;

        // Statistics
        NetworkStats m_Stats;

        // Synchronization
        std::mutex m_PacketQueueMutex;
    };

    // Global access (Conquer style)
    extern ConquerNetworkManager& g_ConquerNetwork;

} // namespace Cherry
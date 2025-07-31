#include "CHpch.h"
#include "ConquerNetworkManager.h"
#include <chrono>

namespace Cherry {

    // Global instance
    ConquerNetworkManager& g_ConquerNetwork = ConquerNetworkManager::Get();

    bool ConquerNetworkManager::Connect(const std::string& serverIP, uint16_t port, uint32_t timeout) {
        if (IsConnected()) {
            CH_CORE_WARN("Already connected to server");
            return true;
        }

        SetState(ConquerConnectionState::Connecting);
        
        m_ServerIP = serverIP;
        m_ServerPort = port;

        // Initialize base network manager if needed
        if (!m_NetworkManager) {
            m_NetworkManager = &NetworkManager::Get();
            m_NetworkManager->Initialize();
        }

        // Set up packet handlers
        RegisterPacketHandler(ConquerPacketType::MsgConnect, 
            [this](std::unique_ptr<ConquerPacket> packet) { HandleConnect(std::move(packet)); });
        RegisterPacketHandler(ConquerPacketType::MsgUserInfo, 
            [this](std::unique_ptr<ConquerPacket> packet) { HandleUserInfo(std::move(packet)); });
        RegisterPacketHandler(ConquerPacketType::MsgTalk, 
            [this](std::unique_ptr<ConquerPacket> packet) { HandleTalk(std::move(packet)); });
        RegisterPacketHandler(ConquerPacketType::MsgWalk, 
            [this](std::unique_ptr<ConquerPacket> packet) { HandleWalk(std::move(packet)); });
        RegisterPacketHandler(ConquerPacketType::MsgAction, 
            [this](std::unique_ptr<ConquerPacket> packet) { HandleAction(std::move(packet)); });

        // Attempt connection
        bool connected = m_NetworkManager->Connect(serverIP, port, timeout / 1000.0f);
        
        if (connected) {
            SetState(ConquerConnectionState::Connected);
            
            // Send initial connect packet
            auto connectPacket = std::make_unique<MsgConnect>();
            connectPacket->Timestamp = static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());
            connectPacket->ClientVersion = 1024; // Example version
            
            SendPacket(std::move(connectPacket));
            
            CH_CORE_INFO("Connected to Conquer server: {}:{}", serverIP, port);
            
            if (m_ConnectionCallback) {
                m_ConnectionCallback(true, "");
            }
            
            return true;
        } else {
            SetState(ConquerConnectionState::Error);
            CH_CORE_ERROR("Failed to connect to server: {}:{}", serverIP, port);
            
            if (m_ConnectionCallback) {
                m_ConnectionCallback(false, "Connection failed");
            }
            
            return false;
        }
    }

    void ConquerNetworkManager::Disconnect() {
        if (m_NetworkManager) {
            m_NetworkManager->Disconnect();
        }
        
        SetState(ConquerConnectionState::Disconnected);
        
        // Clear state
        m_PlayerID = 0;
        m_PlayerName.clear();
        
        // Clear packet handlers
        m_PacketHandlers.clear();
        
        // Clear packet queue
        std::lock_guard<std::mutex> lock(m_PacketQueueMutex);
        std::queue<std::vector<uint8_t>> empty;
        m_IncomingPackets.swap(empty);
        
        CH_CORE_INFO("Disconnected from Conquer server");
        
        if (m_ConnectionCallback) {
            m_ConnectionCallback(false, "Disconnected");
        }
    }

    bool ConquerNetworkManager::Login(const std::string& username, const std::string& password, const std::string& server) {
        if (!IsConnected()) {
            CH_CORE_ERROR("Not connected to server");
            return false;
        }

        SetState(ConquerConnectionState::Authenticating);
        
        auto loginPacket = std::make_unique<MsgAccount>();
        loginPacket->SetCredentials(username, password, server);
        
        SendPacket(std::move(loginPacket));
        
        CH_CORE_INFO("Attempting login for user: {}", username);
        return true;
    }

    bool ConquerNetworkManager::SelectCharacter(uint32_t characterID) {
        if (m_State != ConquerConnectionState::Connected) {
            CH_CORE_ERROR("Invalid state for character selection");
            return false;
        }

        // Send character selection packet
        // This would be implemented based on the specific server protocol
        
        return true;
    }

    void ConquerNetworkManager::SendPacket(std::unique_ptr<ConquerPacket> packet) {
        if (!m_NetworkManager || !IsConnected()) {
            CH_CORE_ERROR("Cannot send packet - not connected");
            return;
        }

        // Serialize packet
        std::vector<uint8_t> buffer;
        packet->Serialize(buffer);
        
        // Send via base network manager
        m_NetworkManager->SendRawData(buffer);
        
        // Update statistics
        m_Stats.PacketsSent++;
        m_Stats.BytesSent += static_cast<uint32_t>(buffer.size());
        
        CH_CORE_TRACE("Sent packet type: {} ({} bytes)", 
                     static_cast<uint16_t>(packet->GetType()), buffer.size());
    }

    void ConquerNetworkManager::RegisterPacketHandler(ConquerPacketType type, ConquerPacketHandler handler) {
        m_PacketHandlers[type] = handler;
    }

    void ConquerNetworkManager::UnregisterPacketHandler(ConquerPacketType type) {
        m_PacketHandlers.erase(type);
    }

    void ConquerNetworkManager::Update() {
        if (!m_NetworkManager) return;
        
        // Update base network manager
        m_NetworkManager->ProcessMessages();
        
        // Process incoming packets
        ProcessIncomingPackets();
    }

    void ConquerNetworkManager::ProcessIncomingPackets() {
        std::lock_guard<std::mutex> lock(m_PacketQueueMutex);
        
        while (!m_IncomingPackets.empty()) {
            auto& packetData = m_IncomingPackets.front();
            
            if (packetData.size() >= sizeof(ConquerPacket::PacketHeader)) {
                HandlePacket(packetData.data(), static_cast<uint16_t>(packetData.size()));
            }
            
            m_IncomingPackets.pop();
        }
    }

    void ConquerNetworkManager::HandlePacket(const uint8_t* data, uint16_t length) {
        ConquerPacketType type = ConquerPacketFactory::GetPacketType(data, length);
        
        auto packet = ConquerPacketFactory::CreatePacket(type);
        if (!packet) {
            CH_CORE_WARN("Unknown packet type received: {}", static_cast<uint16_t>(type));
            return;
        }
        
        if (!packet->Deserialize(data, length)) {
            CH_CORE_ERROR("Failed to deserialize packet type: {}", static_cast<uint16_t>(type));
            return;
        }
        
        // Update statistics
        m_Stats.PacketsReceived++;
        m_Stats.BytesReceived += length;
        
        // Find and call handler
        auto it = m_PacketHandlers.find(type);
        if (it != m_PacketHandlers.end()) {
            it->second(std::move(packet));
        } else {
            CH_CORE_TRACE("No handler for packet type: {}", static_cast<uint16_t>(type));
        }
    }

    void ConquerNetworkManager::SetState(ConquerConnectionState newState) {
        if (m_State != newState) {
            ConquerConnectionState oldState = m_State;
            m_State = newState;
            
            CH_CORE_INFO("Network state changed: {} -> {}", 
                        static_cast<int>(oldState), static_cast<int>(newState));
        }
    }

    // Default packet handlers
    void ConquerNetworkManager::HandleConnect(std::unique_ptr<ConquerPacket> packet) {
        auto connectMsg = static_cast<MsgConnect*>(packet.get());
        CH_CORE_INFO("Received connect response - Server timestamp: {}", connectMsg->Timestamp);
    }

    void ConquerNetworkManager::HandleUserInfo(std::unique_ptr<ConquerPacket> packet) {
        auto userInfo = static_cast<MsgUserInfo*>(packet.get());
        
        m_PlayerID = userInfo->PlayerID;
        m_PlayerName = std::string(userInfo->PlayerName);
        
        SetState(ConquerConnectionState::InGame);
        
        CH_CORE_INFO("Player info received - ID: {}, Name: {}, Level: {}", 
                    userInfo->PlayerID, userInfo->PlayerName, userInfo->Level);
    }

    void ConquerNetworkManager::HandleTalk(std::unique_ptr<ConquerPacket> packet) {
        auto talkMsg = static_cast<MsgTalk*>(packet.get());
        CH_CORE_INFO("Chat: [{}] {}: {}", 
                    static_cast<int>(talkMsg->TalkChannel), 
                    talkMsg->SenderName, talkMsg->Message);
    }

    void ConquerNetworkManager::HandleWalk(std::unique_ptr<ConquerPacket> packet) {
        auto walkMsg = static_cast<MsgWalk*>(packet.get());
        CH_CORE_TRACE("Player {} moved to ({}, {}) facing {}", 
                     walkMsg->PlayerID, walkMsg->X, walkMsg->Y, walkMsg->Direction);
    }

    void ConquerNetworkManager::HandleAction(std::unique_ptr<ConquerPacket> packet) {
        auto actionMsg = static_cast<MsgAction*>(packet.get());
        CH_CORE_TRACE("Player {} performed action {} at ({}, {})", 
                     actionMsg->PlayerID, static_cast<int>(actionMsg->Action), 
                     actionMsg->X, actionMsg->Y);
    }

} // namespace Cherry
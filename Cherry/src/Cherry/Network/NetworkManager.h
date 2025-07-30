#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Core/Threading.h"
#include <functional>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>
#include <memory>
#include <unordered_set>

namespace Cherry {

    // Forward declarations
    class NetworkMessage;
    class PacketSerializer;
    class PacketDeserializer;

    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Reconnecting,
        Error
    };

    enum class PacketType : uint16_t {
        // Connection packets
        CONNECT_REQUEST = 1000,
        CONNECT_RESPONSE = 1001,
        DISCONNECT = 1002,
        HEARTBEAT = 1003,

        // Authentication packets
        LOGIN_REQUEST = 2000,
        LOGIN_RESPONSE = 2001,
        LOGOUT = 2002,

        // Character packets
        CHARACTER_LIST = 3000,
        CHARACTER_CREATE = 3001,
        CHARACTER_DELETE = 3002,
        CHARACTER_SELECT = 3003,

        // Game world packets
        ENTER_WORLD = 4000,
        LEAVE_WORLD = 4001,
        PLAYER_MOVEMENT = 4002,
        PLAYER_UPDATE = 4003,
        SPAWN_ENTITY = 4004,
        DESPAWN_ENTITY = 4005,

        // Chat packets
        CHAT_MESSAGE = 5000,
        WHISPER_MESSAGE = 5001,
        SYSTEM_MESSAGE = 5002,

        // Combat packets
        ATTACK = 6000,
        SKILL_USE = 6001,
        DAMAGE_DEALT = 6002,
        HEAL = 6003,
        BUFF_APPLIED = 6004,
        DEBUFF_APPLIED = 6005,

        // Inventory packets
        INVENTORY_UPDATE = 7000,
        ITEM_USE = 7001,
        ITEM_MOVE = 7002,
        ITEM_DROP = 7003,
        ITEM_PICKUP = 7004,

        // Trade packets
        TRADE_REQUEST = 8000,
        TRADE_ACCEPT = 8001,
        TRADE_CANCEL = 8002,
        TRADE_COMPLETE = 8003,

        // Custom game packets
        CUSTOM_PACKET = 9000
    };

    struct NetworkMessage {
        PacketType Type;
        std::vector<uint8_t> Data;
        uint32_t Timestamp;
        uint32_t Sequence;

        NetworkMessage() : Type(PacketType::CUSTOM_PACKET), Timestamp(0), Sequence(0) {}
        NetworkMessage(PacketType type, const std::vector<uint8_t>& data)
            : Type(type), Data(data), Timestamp(GetCurrentTimestamp()), Sequence(0) {
        }

        size_t GetSize() const { return sizeof(PacketType) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + Data.size(); }

    private:
        uint32_t GetCurrentTimestamp() const;
    };

    // Base packet interface
    class IPacket {
    public:
        virtual ~IPacket() = default;
        virtual PacketType GetType() const = 0;
        virtual void Serialize(PacketSerializer& serializer) const = 0;
        virtual void Deserialize(PacketDeserializer& deserializer) = 0;
        virtual size_t GetSize() const = 0;
    };

    // Packet serialization utilities
    class PacketSerializer {
    public:
        PacketSerializer(std::vector<uint8_t>& buffer) : m_Buffer(buffer) {}

        void Write(const void* data, size_t size) {
            const uint8_t* bytes = static_cast<const uint8_t*>(data);
            m_Buffer.insert(m_Buffer.end(), bytes, bytes + size);
        }

        template<typename T>
        void Write(const T& value) {
            Write(&value, sizeof(T));
        }

        void WriteString(const std::string& str) {
            uint32_t length = static_cast<uint32_t>(str.length());
            Write(length);
            Write(str.data(), str.length());
        }

        template<typename T>
        void WriteVector(const std::vector<T>& vec) {
            uint32_t size = static_cast<uint32_t>(vec.size());
            Write(size);
            for (const auto& item : vec) {
                Write(item);
            }
        }

    private:
        std::vector<uint8_t>& m_Buffer;
    };

    class PacketDeserializer {
    public:
        PacketDeserializer(const std::vector<uint8_t>& buffer) : m_Buffer(buffer), m_Position(0) {}

        void Read(void* data, size_t size) {
            if (m_Position + size > m_Buffer.size()) {
                throw std::runtime_error("PacketDeserializer: Not enough data to read");
            }
            memcpy(data, m_Buffer.data() + m_Position, size);
            m_Position += size;
        }

        template<typename T>
        T Read() {
            T value;
            Read(&value, sizeof(T));
            return value;
        }

        std::string ReadString() {
            uint32_t length = Read<uint32_t>();
            if (m_Position + length > m_Buffer.size()) {
                throw std::runtime_error("PacketDeserializer: Not enough data to read string");
            }
            std::string str(reinterpret_cast<const char*>(m_Buffer.data() + m_Position), length);
            m_Position += length;
            return str;
        }

        template<typename T>
        std::vector<T> ReadVector() {
            uint32_t size = Read<uint32_t>();
            std::vector<T> vec;
            vec.reserve(size);
            for (uint32_t i = 0; i < size; ++i) {
                vec.push_back(Read<T>());
            }
            return vec;
        }

        bool HasMoreData() const { return m_Position < m_Buffer.size(); }
        size_t GetRemainingSize() const { return m_Buffer.size() - m_Position; }

    private:
        const std::vector<uint8_t>& m_Buffer;
        size_t m_Position;
    };

    // Example packet implementations
    struct LoginRequestPacket : public IPacket {
        std::string Username;
        std::string Password;
        std::string Version;

        PacketType GetType() const override { return PacketType::LOGIN_REQUEST; }

        void Serialize(PacketSerializer& serializer) const override {
            serializer.WriteString(Username);
            serializer.WriteString(Password);
            serializer.WriteString(Version);
        }

        void Deserialize(PacketDeserializer& deserializer) override {
            Username = deserializer.ReadString();
            Password = deserializer.ReadString();
            Version = deserializer.ReadString();
        }

        size_t GetSize() const override {
            return sizeof(uint32_t) * 3 + Username.size() + Password.size() + Version.size();
        }
    };

    struct PlayerMovementPacket : public IPacket {
        uint32_t PlayerID;
        float X, Y, Z;
        float VelocityX, VelocityY;
        uint32_t Timestamp;

        PacketType GetType() const override { return PacketType::PLAYER_MOVEMENT; }

        void Serialize(PacketSerializer& serializer) const override {
            serializer.Write(PlayerID);
            serializer.Write(X);
            serializer.Write(Y);
            serializer.Write(Z);
            serializer.Write(VelocityX);
            serializer.Write(VelocityY);
            serializer.Write(Timestamp);
        }

        void Deserialize(PacketDeserializer& deserializer) override {
            PlayerID = deserializer.Read<uint32_t>();
            X = deserializer.Read<float>();
            Y = deserializer.Read<float>();
            Z = deserializer.Read<float>();
            VelocityX = deserializer.Read<float>();
            VelocityY = deserializer.Read<float>();
            Timestamp = deserializer.Read<uint32_t>();
        }

        size_t GetSize() const override {
            return sizeof(uint32_t) * 2 + sizeof(float) * 5;
        }
    };

    struct ChatMessagePacket : public IPacket {
        uint32_t SenderID;
        std::string SenderName;
        std::string Message;
        uint8_t Channel; // 0=World, 1=Guild, 2=Party, etc.

        PacketType GetType() const override { return PacketType::CHAT_MESSAGE; }

        void Serialize(PacketSerializer& serializer) const override {
            serializer.Write(SenderID);
            serializer.WriteString(SenderName);
            serializer.WriteString(Message);
            serializer.Write(Channel);
        }

        void Deserialize(PacketDeserializer& deserializer) override {
            SenderID = deserializer.Read<uint32_t>();
            SenderName = deserializer.ReadString();
            Message = deserializer.ReadString();
            Channel = deserializer.Read<uint8_t>();
        }

        size_t GetSize() const override {
            return sizeof(uint32_t) * 3 + SenderName.size() + Message.size() + sizeof(uint8_t);
        }
    };

    // Network statistics for monitoring
    struct NetworkStatistics {
        uint64_t BytesSent = 0;
        uint64_t BytesReceived = 0;
        uint64_t PacketsSent = 0;
        uint64_t PacketsReceived = 0;
        uint64_t PacketsLost = 0;
        float Ping = 0.0f;
        float PacketLoss = 0.0f;
        uint32_t LastPingTime = 0;

        void Reset() {
            BytesSent = BytesReceived = 0;
            PacketsSent = PacketsReceived = PacketsLost = 0;
            Ping = PacketLoss = 0.0f;
            LastPingTime = 0;
        }
    };

    // Main network manager class
    class NetworkManager {
    public:
        static NetworkManager& Get() {
            static NetworkManager instance;
            return instance;
        }

        // Initialization
        void Initialize();
        void Shutdown();

        // Connection management
        bool Connect(const std::string& host, uint16_t port, float timeout = 10.0f);
        void Disconnect();
        bool IsConnected() const { return m_ConnectionState == ConnectionState::Connected; }
        ConnectionState GetConnectionState() const { return m_ConnectionState; }

        // Packet sending
        template<typename PacketType>
        void SendPacket(const PacketType& packet, bool reliable = true) {
            std::vector<uint8_t> buffer;
            PacketSerializer serializer(buffer);
            packet.Serialize(serializer);

            NetworkMessage message(packet.GetType(), buffer);
            if (reliable) {
                SendReliableMessage(message);
            }
            else {
                SendUnreliableMessage(message);
            }
        }

        void SendRawData(const std::vector<uint8_t>& data);

        // Packet handling registration
        template<typename PacketType>
        void RegisterPacketHandler(std::function<void(const PacketType&)> handler) {
            PacketType dummy;
            auto packetType = dummy.GetType();

            m_PacketHandlers[packetType] = [handler](const std::vector<uint8_t>& data) {
                PacketDeserializer deserializer(data);
                PacketType packet;
                packet.Deserialize(deserializer);
                handler(packet);
                };
        }

        // Message processing
        void ProcessMessages();
        void SetMaxMessagesPerFrame(uint32_t maxMessages) { m_MaxMessagesPerFrame = maxMessages; }

        // Network configuration
        void SetHeartbeatInterval(float interval) { m_HeartbeatInterval = interval; }
        void SetReconnectAttempts(uint32_t attempts) { m_MaxReconnectAttempts = attempts; }
        void SetPacketCompressionThreshold(size_t threshold) { m_CompressionThreshold = threshold; }

        // Statistics
        const NetworkStatistics& GetStatistics() const { return m_Statistics; }
        void ResetStatistics() { m_Statistics.Reset(); }

        // Event callbacks
        void SetOnConnected(std::function<void()> callback) { m_OnConnected = callback; }
        void SetOnDisconnected(std::function<void()> callback) { m_OnDisconnected = callback; }
        void SetOnConnectionError(std::function<void(const std::string&)> callback) { m_OnConnectionError = callback; }

    private:
        NetworkManager() = default;
        ~NetworkManager() { Shutdown(); }

        // Internal message handling
        void SendReliableMessage(const NetworkMessage& message);
        void SendUnreliableMessage(const NetworkMessage& message);
        void ProcessIncomingData();
        void NetworkThreadFunction();
        void HeartbeatThreadFunction();

        // Connection management
        void HandleConnectionState();
        void AttemptReconnect();
        void SendHeartbeat();

        // Message processing
        void HandleIncomingMessage(const NetworkMessage& message);
        std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data);
        std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& data);

        // Socket management
        bool CreateSocket();
        void CloseSocket();
        bool SendData(const std::vector<uint8_t>& data);
        std::vector<uint8_t> ReceiveData();

    private:
        // Connection state
        ConnectionState m_ConnectionState = ConnectionState::Disconnected;
        std::string m_ServerHost;
        uint16_t m_ServerPort = 0;
        int m_Socket = -1;

        // Threading
        std::thread m_NetworkThread;
        std::thread m_HeartbeatThread;
        std::atomic<bool> m_NetworkThreadRunning = false;
        std::atomic<bool> m_HeartbeatThreadRunning = false;

        // Message queues
        ThreadSafeQueue<NetworkMessage> m_IncomingMessages;
        ThreadSafeQueue<NetworkMessage> m_OutgoingReliableMessages;
        ThreadSafeQueue<NetworkMessage> m_OutgoingUnreliableMessages;

        // Packet handlers
        std::unordered_map<PacketType, std::function<void(const std::vector<uint8_t>&)>> m_PacketHandlers;

        // Configuration
        float m_HeartbeatInterval = 30.0f; // seconds
        uint32_t m_MaxReconnectAttempts = 3;
        uint32_t m_ReconnectAttempts = 0;
        uint32_t m_MaxMessagesPerFrame = 100;
        size_t m_CompressionThreshold = 1024; // bytes

        // Statistics
        NetworkStatistics m_Statistics;

        // Reliable messaging
        uint32_t m_NextSequenceNumber = 1;
        std::unordered_map<uint32_t, NetworkMessage> m_ReliableMessages; // Pending acknowledgment
        std::unordered_set<uint32_t> m_ReceivedSequenceNumbers;

        // Event callbacks
        std::function<void()> m_OnConnected;
        std::function<void()> m_OnDisconnected;
        std::function<void(const std::string&)> m_OnConnectionError;

        // Thread synchronization
        std::mutex m_SocketMutex;
        std::mutex m_StatisticsMutex;
    };

    // Utility class for easy packet registration
    template<typename PacketType>
    class PacketHandler {
    public:
        PacketHandler(std::function<void(const PacketType&)> handler) {
            NetworkManager::Get().RegisterPacketHandler<PacketType>(handler);
        }
    };

    // Macro for easy packet handler registration
#define REGISTER_PACKET_HANDLER(PacketType, Handler) \
        static PacketHandler<PacketType> s_##PacketType##Handler(Handler)

} // namespace Cherry

// Cherry/src/Cherry/Network/NetworkedEntity.h
#pragma once
#include "Cherry/Scene/Entity.h"
#include "Cherry/Scene/Components.h"
#include "NetworkManager.h"

namespace Cherry {

    class NetworkedEntityManager {
    public:
        static NetworkedEntityManager& Get() {
            static NetworkedEntityManager instance;
            return instance;
        }

        // Entity synchronization
        void RegisterNetworkedEntity(Entity entity);
        void UnregisterNetworkedEntity(Entity entity);
        void SynchronizeEntity(Entity entity);
        void SynchronizeAllEntities();

        // Network event handling
        void OnEntitySpawned(uint32_t networkID, const std::string& entityType, const glm::vec3& position);
        void OnEntityDespawned(uint32_t networkID);
        void OnEntityUpdated(uint32_t networkID, const glm::vec3& position, const glm::vec3& rotation);

        // Configuration
        void SetSyncRate(float rate) { m_SyncRate = rate; }
        void SetInterpolationEnabled(bool enabled) { m_InterpolationEnabled = enabled; }

    private:
        std::unordered_map<uint32_t, Entity> m_NetworkedEntities;
        float m_SyncRate = 20.0f; // Hz
        float m_LastSyncTime = 0.0f;
        bool m_InterpolationEnabled = true;

        // Position interpolation data
        struct InterpolationData {
            glm::vec3 StartPosition;
            glm::vec3 TargetPosition;
            glm::vec3 StartRotation;
            glm::vec3 TargetRotation;
            float StartTime;
            float Duration;
        };
        std::unordered_map<uint32_t, InterpolationData> m_InterpolationData;
    };

} // namespace Cherry
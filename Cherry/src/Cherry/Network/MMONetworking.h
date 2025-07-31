// Cherry/src/Cherry/Network/MMONetworking.h
#pragma once
#include "Cherry/Network/NetworkManager.h"
#include "Cherry/Scene/Components.h"
#include <unordered_set>
#include <chrono>

namespace Cherry {

    // MMO-specific packet types (extending your existing system)
    enum class MMOPacketType : uint16_t {
        // Player Management
        PLAYER_LOGIN = 10000,
        PLAYER_LOGOUT = 10001,
        PLAYER_CREATE_CHARACTER = 10002,
        PLAYER_DELETE_CHARACTER = 10003,
        PLAYER_SELECT_CHARACTER = 10004,
        PLAYER_ENTER_WORLD = 10005,
        PLAYER_LEAVE_WORLD = 10006,

        // Movement & Position
        PLAYER_MOVE_REQUEST = 10100,
        PLAYER_MOVE_RESPONSE = 10101,
        PLAYER_POSITION_UPDATE = 10102,
        PLAYER_TELEPORT = 10103,
        PLAYER_JUMP = 10104,

        // Combat System
        COMBAT_ATTACK = 10200,
        COMBAT_SKILL_USE = 10201,
        COMBAT_DAMAGE = 10202,
        COMBAT_HEAL = 10203,
        COMBAT_BUFF_APPLY = 10204,
        COMBAT_DEBUFF_APPLY = 10205,
        COMBAT_STATUS_UPDATE = 10206,

        // Inventory & Items
        INVENTORY_UPDATE = 10300,
        ITEM_PICKUP = 10301,
        ITEM_DROP = 10302,
        ITEM_USE = 10303,
        ITEM_EQUIP = 10304,
        ITEM_UNEQUIP = 10305,
        ITEM_TRADE_REQUEST = 10306,
        ITEM_TRADE_RESPONSE = 10307,

        // Social & Chat
        CHAT_MESSAGE = 10400,
        WHISPER_MESSAGE = 10401,
        GUILD_MESSAGE = 10402,
        PARTY_MESSAGE = 10403,
        SYSTEM_MESSAGE = 10404,
        FRIEND_REQUEST = 10405,
        GUILD_INVITE = 10406,
        PARTY_INVITE = 10407,

        // World Events
        NPC_SPAWN = 10500,
        NPC_DESPAWN = 10501,
        NPC_UPDATE = 10502,
        OBJECT_INTERACT = 10503,
        DOOR_STATE = 10504,
        WEATHER_UPDATE = 10505,
        TIME_UPDATE = 10506,

        // Administrative
        ADMIN_COMMAND = 10600,
        SERVER_SHUTDOWN = 10601,
        MAINTENANCE_MODE = 10602,
        PLAYER_KICK = 10603,
        PLAYER_BAN = 10604
    };

    // MMO Player data packet
    struct MMOPlayerData : public IPacket {
        uint32_t PlayerID = 0;
        std::string PlayerName;
        uint32_t Level = 1;
        uint32_t Experience = 0;
        uint32_t Health = 100;
        uint32_t MaxHealth = 100;
        uint32_t Mana = 100;
        uint32_t MaxMana = 100;
        uint32_t Class = 0;
        uint32_t MapID = 0;
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        uint8_t Direction = 0;

        PacketType GetType() const override { return static_cast<PacketType>(MMOPacketType::PLAYER_ENTER_WORLD); }

        void Serialize(PacketSerializer& serializer) const override {
            serializer.Write(PlayerID);
            serializer.WriteString(PlayerName);
            serializer.Write(Level);
            serializer.Write(Experience);
            serializer.Write(Health);
            serializer.Write(MaxHealth);
            serializer.Write(Mana);
            serializer.Write(MaxMana);
            serializer.Write(Class);
            serializer.Write(MapID);
            serializer.Write(Position.x);
            serializer.Write(Position.y);
            serializer.Write(Position.z);
            serializer.Write(Rotation.x);
            serializer.Write(Rotation.y);
            serializer.Write(Rotation.z);
            serializer.Write(Direction);
        }

        void Deserialize(PacketDeserializer& deserializer) override {
            PlayerID = deserializer.Read<uint32_t>();
            PlayerName = deserializer.ReadString();
            Level = deserializer.Read<uint32_t>();
            Experience = deserializer.Read<uint32_t>();
            Health = deserializer.Read<uint32_t>();
            MaxHealth = deserializer.Read<uint32_t>();
            Mana = deserializer.Read<uint32_t>();
            MaxMana = deserializer.Read<uint32_t>();
            Class = deserializer.Read<uint32_t>();
            MapID = deserializer.Read<uint32_t>();
            Position.x = deserializer.Read<float>();
            Position.y = deserializer.Read<float>();
            Position.z = deserializer.Read<float>();
            Rotation.x = deserializer.Read<float>();
            Rotation.y = deserializer.Read<float>();
            Rotation.z = deserializer.Read<float>();
            Direction = deserializer.Read<uint8_t>();
        }

        size_t GetSize() const override {
            return sizeof(uint32_t) * 10 + sizeof(float) * 6 + sizeof(uint8_t) +
                sizeof(uint32_t) + PlayerName.size();
        }
    };

    // Movement packet with anti-cheat validation
    struct MMOMovementPacket : public IPacket {
        uint32_t PlayerID = 0;
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Velocity = { 0.0f, 0.0f, 0.0f };
        uint8_t Direction = 0;
        uint32_t Timestamp = 0;
        uint16_t SequenceNumber = 0;
        uint8_t MovementFlags = 0; // Running, jumping, etc.

        PacketType GetType() const override { return static_cast<PacketType>(MMOPacketType::PLAYER_MOVE_REQUEST); }

        void Serialize(PacketSerializer& serializer) const override {
            serializer.Write(PlayerID);
            serializer.Write(Position.x);
            serializer.Write(Position.y);
            serializer.Write(Position.z);
            serializer.Write(Velocity.x);
            serializer.Write(Velocity.y);
            serializer.Write(Velocity.z);
            serializer.Write(Direction);
            serializer.Write(Timestamp);
            serializer.Write(SequenceNumber);
            serializer.Write(MovementFlags);
        }

        void Deserialize(PacketDeserializer& deserializer) override {
            PlayerID = deserializer.Read<uint32_t>();
            Position.x = deserializer.Read<float>();
            Position.y = deserializer.Read<float>();
            Position.z = deserializer.Read<float>();
            Velocity.x = deserializer.Read<float>();
            Velocity.y = deserializer.Read<float>();
            Velocity.z = deserializer.Read<float>();
            Direction = deserializer.Read<uint8_t>();
            Timestamp = deserializer.Read<uint32_t>();
            SequenceNumber = deserializer.Read<uint16_t>();
            MovementFlags = deserializer.Read<uint8_t>();
        }

        size_t GetSize() const override {
            return sizeof(uint32_t) * 2 + sizeof(float) * 6 + sizeof(uint8_t) * 2 + sizeof(uint16_t);
        }
    };

    // Chat message packet
    struct MMOChatPacket : public IPacket {
        enum class ChatChannel : uint8_t {
            Say = 0,
            Whisper = 1,
            Party = 2,
            Guild = 3,
            World = 4,
            System = 5,
            Admin = 6
        };

        uint32_t SenderID = 0;
        uint32_t TargetID = 0; // For whispers
        std::string SenderName;
        std::string Message;
        ChatChannel Channel = ChatChannel::Say;
        uint32_t Timestamp = 0;

        PacketType GetType() const override { return static_cast<PacketType>(MMOPacketType::CHAT_MESSAGE); }

        void Serialize(PacketSerializer& serializer) const override {
            serializer.Write(SenderID);
            serializer.Write(TargetID);
            serializer.WriteString(SenderName);
            serializer.WriteString(Message);
            serializer.Write(static_cast<uint8_t>(Channel));
            serializer.Write(Timestamp);
        }

        void Deserialize(PacketDeserializer& deserializer) override {
            SenderID = deserializer.Read<uint32_t>();
            TargetID = deserializer.Read<uint32_t>();
            SenderName = deserializer.ReadString();
            Message = deserializer.ReadString();
            Channel = static_cast<ChatChannel>(deserializer.Read<uint8_t>());
            Timestamp = deserializer.Read<uint32_t>();
        }

        size_t GetSize() const override {
            return sizeof(uint32_t) * 3 + sizeof(uint8_t) +
                sizeof(uint32_t) * 2 + SenderName.size() + Message.size();
        }
    };

    // Area of Interest (AOI) manager for efficient networking
    class AOIManager {
    public:
        struct AOIObject {
            uint32_t ID;
            glm::vec3 Position;
            float ViewDistance;
            std::unordered_set<uint32_t> Observers; // Players who can see this object

            AOIObject(uint32_t id, const glm::vec3& pos, float viewDist = 50.0f)
                : ID(id), Position(pos), ViewDistance(viewDist) {
            }
        };

        AOIManager(float worldWidth, float worldHeight, size_t gridSize = 100);

        // Object management
        void AddObject(uint32_t id, const glm::vec3& position, float viewDistance = 50.0f);
        void RemoveObject(uint32_t id);
        void UpdateObjectPosition(uint32_t id, const glm::vec3& newPosition);

        // Query objects in range
        std::vector<uint32_t> GetObjectsInRange(const glm::vec3& position, float range) const;
        std::vector<uint32_t> GetPlayersInRange(uint32_t objectId, float range = 0.0f) const;

        // Observer management
        void AddObserver(uint32_t objectId, uint32_t observerId);
        void RemoveObserver(uint32_t objectId, uint32_t observerId);
        const std::unordered_set<uint32_t>& GetObservers(uint32_t objectId) const;

        // Update all AOI relationships
        void UpdateAOI();

        // Statistics
        size_t GetObjectCount() const { return m_Objects.size(); }
        size_t GetGridCellCount() const { return m_GridCells.size(); }

    private:
        struct GridCell {
            std::unordered_set<uint32_t> Objects;
        };

        std::unordered_map<uint32_t, AOIObject> m_Objects;
        std::vector<std::vector<GridCell>> m_GridCells;

        float m_WorldWidth;
        float m_WorldHeight;
        size_t m_GridSize;
        float m_CellWidth;
        float m_CellHeight;

        std::pair<size_t, size_t> GetGridCoordinates(const glm::vec3& position) const;
        std::vector<std::pair<size_t, size_t>> GetNeighborCells(size_t x, size_t y, float range) const;
    };

    // MMO-specific network manager
    class MMONetworkManager {
    public:
        static MMONetworkManager& Get() {
            static MMONetworkManager instance;
            return instance;
        }

        void Initialize();
        void Shutdown();

        // Player management
        bool LoginPlayer(const std::string& username, const std::string& password);
        void LogoutPlayer(uint32_t playerId);
        bool CreateCharacter(const std::string& characterName, uint32_t classType);
        bool SelectCharacter(uint32_t characterId);

        // Movement synchronization
        void SendMovementUpdate(const glm::vec3& position, const glm::vec3& velocity, uint8_t direction);
        void ProcessMovementUpdate(const MMOMovementPacket& packet);

        // Chat system
        void SendChatMessage(const std::string& message, MMOChatPacket::ChatChannel channel, uint32_t targetId = 0);
        void SendWhisper(const std::string& targetPlayer, const std::string& message);

        // Combat system
        void SendAttack(uint32_t targetId, uint32_t skillId = 0);
        void SendSkillUse(uint32_t skillId, const glm::vec3& targetPosition);

        // Inventory system
        void SendItemPickup(uint32_t itemId);
        void SendItemDrop(uint32_t itemId, uint32_t quantity = 1);
        void SendItemUse(uint32_t itemId);

        // World synchronization
        void SetPlayerPosition(uint32_t playerId, const glm::vec3& position);
        std::vector<uint32_t> GetPlayersInRange(const glm::vec3& position, float range) const;

        // Anti-cheat systems
        bool ValidateMovement(uint32_t playerId, const glm::vec3& oldPos, const glm::vec3& newPos, float deltaTime);
        void ReportSuspiciousActivity(uint32_t playerId, const std::string& reason);

        // Event callbacks
        void SetOnPlayerLogin(std::function<void(uint32_t, const std::string&)> callback) { m_OnPlayerLogin = callback; }
        void SetOnPlayerLogout(std::function<void(uint32_t)> callback) { m_OnPlayerLogout = callback; }
        void SetOnChatMessage(std::function<void(const MMOChatPacket&)> callback) { m_OnChatMessage = callback; }
        void SetOnPlayerMove(std::function<void(uint32_t, const glm::vec3&)> callback) { m_OnPlayerMove = callback; }

        // Statistics
        struct MMONetworkStats {
            size_t ConnectedPlayers = 0;
            size_t MessagesPerSecond = 0;
            size_t MovementUpdatesPerSecond = 0;
            float AverageLatency = 0.0f;
            size_t CheatAttempts = 0;
        };

        MMONetworkStats GetStats() const { return m_Stats; }

    private:
        MMONetworkManager() = default;
        ~MMONetworkManager() = default;

        // Core networking
        NetworkManager* m_NetworkManager = nullptr;
        std::unique_ptr<AOIManager> m_AOIManager;

        // Player data
        struct ConnectedPlayer {
            uint32_t PlayerID;
            std::string Name;
            glm::vec3 Position;
            glm::vec3 LastValidPosition;
            std::chrono::steady_clock::time_point LastUpdateTime;
            float MovementSpeed = 5.0f;
            uint16_t LastSequenceNumber = 0;
        };

        std::unordered_map<uint32_t, ConnectedPlayer> m_ConnectedPlayers;
        uint32_t m_LocalPlayerId = 0;

        // Event callbacks
        std::function<void(uint32_t, const std::string&)> m_OnPlayerLogin;
        std::function<void(uint32_t)> m_OnPlayerLogout;
        std::function<void(const MMOChatPacket&)> m_OnChatMessage;
        std::function<void(uint32_t, const glm::vec3&)> m_OnPlayerMove;

        // Statistics
        MMONetworkStats m_Stats;
        std::mutex m_StatsMutex;

        // Packet handlers
        void HandlePlayerLogin(const MMOPlayerData& packet);
        void HandlePlayerLogout(uint32_t playerId);
        void HandleMovementUpdate(const MMOMovementPacket& packet);
        void HandleChatMessage(const MMOChatPacket& packet);

        // Anti-cheat
        struct AntiCheatData {
            std::chrono::steady_clock::time_point LastMovementTime;
            glm::vec3 LastPosition;
            float AccumulatedDistance = 0.0f;
            uint32_t SuspiciousCount = 0;
        };
        std::unordered_map<uint32_t, AntiCheatData> m_AntiCheatData;

        void UpdateAntiCheat();
    };

    // Network interpolation for smooth movement
    class NetworkInterpolator {
    public:
        struct InterpolationState {
            glm::vec3 Position;
            glm::vec3 Rotation;
            std::chrono::steady_clock::time_point Timestamp;
        };

        NetworkInterpolator(float bufferTime = 0.1f); // 100ms buffer

        void AddState(const glm::vec3& position, const glm::vec3& rotation);
        void AddState(const InterpolationState& state);

        InterpolationState GetInterpolatedState() const;
        void Update(float deltaTime);

        void SetBufferTime(float bufferTime) { m_BufferTime = bufferTime; }
        float GetBufferTime() const { return m_BufferTime; }

        void Clear() { m_States.clear(); }
        bool HasStates() const { return !m_States.empty(); }

    private:
        std::deque<InterpolationState> m_States;
        float m_BufferTime;

        void CleanOldStates();
    };

    // Lag compensation system
    class LagCompensation {
    public:
        struct PlayerSnapshot {
            uint32_t PlayerID;
            glm::vec3 Position;
            glm::vec3 Rotation;
            uint32_t Health;
            std::chrono::steady_clock::time_point Timestamp;
        };

        LagCompensation(float maxCompensationTime = 1.0f);

        void AddSnapshot(const PlayerSnapshot& snapshot);
        PlayerSnapshot GetSnapshotAtTime(uint32_t playerId, std::chrono::steady_clock::time_point targetTime) const;

        // Rewind world state for hit validation
        void RewindWorld(std::chrono::steady_clock::time_point targetTime);
        void RestoreWorld();

        void SetMaxCompensationTime(float time) { m_MaxCompensationTime = time; }
        float GetMaxCompensationTime() const { return m_MaxCompensationTime; }

    private:
        std::unordered_map<uint32_t, std::deque<PlayerSnapshot>> m_PlayerSnapshots;
        float m_MaxCompensationTime;

        void CleanOldSnapshots();
    };

    // Packet reliability system for critical messages
    class ReliablePacketSystem {
    public:
        struct ReliablePacket {
            uint32_t ID;
            std::vector<uint8_t> Data;
            std::chrono::steady_clock::time_point SendTime;
            uint32_t RetryCount = 0;
            bool Acknowledged = false;
        };

        ReliablePacketSystem(float retryInterval = 0.5f, uint32_t maxRetries = 5);

        uint32_t SendReliablePacket(const std::vector<uint8_t>& data);
        void AcknowledgePacket(uint32_t packetId);
        void ProcessAcknowledgments(const std::vector<uint32_t>& acknowledgedIds);

        void Update();

        // Statistics
        size_t GetPendingPacketCount() const { return m_PendingPackets.size(); }
        size_t GetTotalPacketsSent() const { return m_TotalPacketsSent; }
        size_t GetPacketsLost() const { return m_PacketsLost; }

    private:
        std::unordered_map<uint32_t, ReliablePacket> m_PendingPackets;
        uint32_t m_NextPacketId = 1;
        float m_RetryInterval;
        uint32_t m_MaxRetries;

        size_t m_TotalPacketsSent = 0;
        size_t m_PacketsLost = 0;

        void RetryPacket(uint32_t packetId);
        void RemovePacket(uint32_t packetId);
    };

    // Server list and connection management
    class ServerManager {
    public:
        struct ServerInfo {
            std::string Name;
            std::string Address;
            uint16_t Port;
            uint32_t OnlinePlayers;
            uint32_t MaxPlayers;
            float Ping;
            std::string Version;
            std::unordered_map<std::string, std::string> Properties;
        };

        ServerManager();

        // Server discovery
        void RefreshServerList();
        std::vector<ServerInfo> GetServerList() const;
        ServerInfo GetBestServer() const; // Based on ping and player count

        // Connection management
        bool ConnectToServer(const std::string& serverName);
        bool ConnectToServer(const std::string& address, uint16_t port);
        void DisconnectFromServer();
        bool IsConnected() const;

        const ServerInfo& GetCurrentServer() const { return m_CurrentServer; }

        // Events
        void SetOnServerListUpdated(std::function<void()> callback) { m_OnServerListUpdated = callback; }
        void SetOnConnectionStateChanged(std::function<void(bool)> callback) { m_OnConnectionStateChanged = callback; }

    private:
        std::vector<ServerInfo> m_ServerList;
        ServerInfo m_CurrentServer;
        bool m_IsConnected = false;

        std::function<void()> m_OnServerListUpdated;
        std::function<void(bool)> m_OnConnectionStateChanged;

        void PingServer(ServerInfo& server);
        void QueryServerInfo(ServerInfo& server);
    };

    // Character management system
    class CharacterManager {
    public:
        struct CharacterData {
            uint32_t CharacterID;
            std::string Name;
            uint32_t Level;
            uint32_t Experience;
            uint32_t Class;
            uint32_t MapID;
            glm::vec3 Position;

            // Equipment
            std::unordered_map<uint32_t, uint32_t> Equipment; // slot -> item id

            // Stats
            uint32_t Health;
            uint32_t MaxHealth;
            uint32_t Mana;
            uint32_t MaxMana;
            uint32_t Strength;
            uint32_t Agility;
            uint32_t Intelligence;
            uint32_t Vitality;

            // Game-specific data
            uint32_t Silver;
            uint32_t ConquerPoints;
            uint32_t Profession;
            uint32_t ProfessionLevel;
        };

        CharacterManager();

        // Character operations
        bool CreateCharacter(const std::string& name, uint32_t classType);
        bool DeleteCharacter(uint32_t characterId);
        bool SelectCharacter(uint32_t characterId);

        std::vector<CharacterData> GetCharacterList() const { return m_Characters; }
        const CharacterData* GetCurrentCharacter() const { return m_CurrentCharacter; }

        // Character updates
        void UpdateCharacterPosition(const glm::vec3& position);
        void UpdateCharacterStats(const CharacterData& newStats);
        void UpdateCharacterEquipment(uint32_t slot, uint32_t itemId);

        // Events
        void SetOnCharacterListUpdated(std::function<void()> callback) { m_OnCharacterListUpdated = callback; }
        void SetOnCharacterSelected(std::function<void(const CharacterData&)> callback) { m_OnCharacterSelected = callback; }

    private:
        std::vector<CharacterData> m_Characters;
        CharacterData* m_CurrentCharacter = nullptr;

        std::function<void()> m_OnCharacterListUpdated;
        std::function<void(const CharacterData&)> m_OnCharacterSelected;

        void RequestCharacterList();
        void HandleCharacterListResponse(const std::vector<CharacterData>& characters);
    };

    // World synchronization system
    class WorldSynchronizer {
    public:
        struct WorldObject {
            uint32_t ID;
            uint32_t Type; // NPC, item, door, etc.
            glm::vec3 Position;
            glm::vec3 Rotation;
            uint32_t State; // health, open/closed, etc.
            std::unordered_map<std::string, std::any> Properties;
        };

        WorldSynchronizer();

        // Object management
        void AddObject(const WorldObject& obj);
        void RemoveObject(uint32_t objectId);
        void UpdateObject(uint32_t objectId, const WorldObject& newData);

        const WorldObject* GetObject(uint32_t objectId) const;
        std::vector<WorldObject> GetObjectsInArea(const glm::vec3& center, float radius) const;

        // Map management
        void LoadMap(uint32_t mapId);
        void UnloadMap(uint32_t mapId);
        uint32_t GetCurrentMap() const { return m_CurrentMapId; }

        // Environment
        void SetWeather(uint32_t weatherType, float intensity);
        void SetTimeOfDay(float timeOfDay); // 0.0 = midnight, 0.5 = noon

        // Events
        void SetOnObjectSpawned(std::function<void(const WorldObject&)> callback) { m_OnObjectSpawned = callback; }
        void SetOnObjectDespawned(std::function<void(uint32_t)> callback) { m_OnObjectDespawned = callback; }
        void SetOnObjectUpdated(std::function<void(const WorldObject&)> callback) { m_OnObjectUpdated = callback; }

        void Update(float deltaTime);

    private:
        std::unordered_map<uint32_t, WorldObject> m_WorldObjects;
        uint32_t m_CurrentMapId = 0;

        struct EnvironmentState {
            uint32_t WeatherType = 0;
            float WeatherIntensity = 0.0f;
            float TimeOfDay = 0.5f; // Noon
        } m_Environment;

        std::function<void(const WorldObject&)> m_OnObjectSpawned;
        std::function<void(uint32_t)> m_OnObjectDespawned;
        std::function<void(const WorldObject&)> m_OnObjectUpdated;

        void HandleObjectSpawn(const WorldObject& obj);
        void HandleObjectDespawn(uint32_t objectId);
        void HandleObjectUpdate(const WorldObject& obj);
    };

    // Trade system
    class TradeSystem {
    public:
        struct TradeItem {
            uint32_t ItemID;
            uint32_t Quantity;
            std::string Name;
        };

        struct TradeWindow {
            uint32_t PartnerID;
            std::string PartnerName;
            std::vector<TradeItem> MyItems;
            std::vector<TradeItem> PartnerItems;
            uint32_t MyGold = 0;
            uint32_t PartnerGold = 0;
            bool MyAccepted = false;
            bool PartnerAccepted = false;
            bool IsActive = false;
        };

        TradeSystem();

        // Trade operations
        bool RequestTrade(uint32_t targetPlayerId);
        void AcceptTradeRequest(uint32_t requesterId);
        void DeclineTradeRequest(uint32_t requesterId);
        void CancelTrade();

        // Item management
        void AddItemToTrade(uint32_t itemId, uint32_t quantity);
        void RemoveItemFromTrade(uint32_t itemId);
        void SetGoldAmount(uint32_t goldAmount);

        // Trade confirmation
        void AcceptTrade();
        void DeclineTrade();

        const TradeWindow& GetTradeWindow() const { return m_TradeWindow; }
        bool IsTrading() const { return m_TradeWindow.IsActive; }

        // Events
        void SetOnTradeRequested(std::function<void(uint32_t, const std::string&)> callback) { m_OnTradeRequested = callback; }
        void SetOnTradeStarted(std::function<void(uint32_t, const std::string&)> callback) { m_OnTradeStarted = callback; }
        void SetOnTradeCompleted(std::function<void()> callback) { m_OnTradeCompleted = callback; }
        void SetOnTradeCancelled(std::function<void()> callback) { m_OnTradeCancelled = callback; }

    private:
        TradeWindow m_TradeWindow;

        std::function<void(uint32_t, const std::string&)> m_OnTradeRequested;
        std::function<void(uint32_t, const std::string&)> m_OnTradeStarted;
        std::function<void()> m_OnTradeCompleted;
        std::function<void()> m_OnTradeCancelled;

        void HandleTradeRequest(uint32_t requesterId, const std::string& requesterName);
        void HandleTradeUpdate(const TradeWindow& updatedWindow);
        void HandleTradeCompletion();
        void HandleTradeCancellation();
    };

    // Guild system
    class GuildSystem {
    public:
        struct GuildMember {
            uint32_t PlayerID;
            std::string Name;
            uint32_t Level;
            uint32_t Rank; // 0 = leader, 1 = deputy, 2 = member, etc.
            std::string RankName;
            bool IsOnline;
            std::chrono::system_clock::time_point LastSeen;
        };

        struct GuildInfo {
            uint32_t GuildID;
            std::string Name;
            std::string Description;
            uint32_t Level;
            uint32_t Experience;
            uint32_t MemberCount;
            uint32_t MaxMembers;
            std::vector<GuildMember> Members;
        };

        GuildSystem();

        // Guild operations
        bool CreateGuild(const std::string& name, const std::string& description);
        bool JoinGuild(uint32_t guildId);
        void LeaveGuild();
        bool InvitePlayer(uint32_t playerId);
        bool KickMember(uint32_t playerId);

        // Guild management
        void SetMemberRank(uint32_t playerId, uint32_t newRank);
        void UpdateGuildDescription(const std::string& description);
        void DonateToGuild(uint32_t amount);

        // Guild info
        const GuildInfo* GetGuildInfo() const { return m_CurrentGuild.get(); }
        bool IsInGuild() const { return m_CurrentGuild != nullptr; }

        // Guild chat
        void SendGuildMessage(const std::string& message);

        // Events
        void SetOnGuildJoined(std::function<void(const GuildInfo&)> callback) { m_OnGuildJoined = callback; }
        void SetOnGuildLeft(std::function<void()> callback) { m_OnGuildLeft = callback; }
        void SetOnGuildMessage(std::function<void(const std::string&, const std::string&)> callback) { m_OnGuildMessage = callback; }

    private:
        std::unique_ptr<GuildInfo> m_CurrentGuild;

        std::function<void(const GuildInfo&)> m_OnGuildJoined;
        std::function<void()> m_OnGuildLeft;
        std::function<void(const std::string&, const std::string&)> m_OnGuildMessage;

        void HandleGuildInvite(uint32_t guildId, const std::string& guildName, const std::string& inviterName);
        void HandleGuildUpdate(const GuildInfo& updatedInfo);
        void HandleGuildMessage(const std::string& senderName, const std::string& message);
    };

    // High-level MMO client interface
    class MMOClient {
    public:
        static MMOClient& Get() {
            static MMOClient instance;
            return instance;
        }

        void Initialize();
        void Shutdown();
        void Update(float deltaTime);

        // Connection management
        bool ConnectToServer(const std::string& serverName);
        void DisconnectFromServer();
        bool IsConnected() const;

        // Authentication
        bool Login(const std::string& username, const std::string& password);
        void Logout();

        // Character management
        CharacterManager& GetCharacterManager() { return m_CharacterManager; }

        // World interaction
        WorldSynchronizer& GetWorldSynchronizer() { return m_WorldSynchronizer; }

        // Social systems
        TradeSystem& GetTradeSystem() { return m_TradeSystem; }
        GuildSystem& GetGuildSystem() { return m_GuildSystem; }

        // Movement
        void MovePlayer(const glm::vec3& targetPosition);
        void SetPlayerDirection(uint8_t direction);

        // Combat
        void AttackTarget(uint32_t targetId);
        void UseSkill(uint32_t skillId, uint32_t targetId = 0);

        // Chat
        void SendChatMessage(const std::string& message, MMOChatPacket::ChatChannel channel = MMOChatPacket::ChatChannel::Say);
        void SendWhisper(const std::string& targetPlayer, const std::string& message);

        // Statistics
        MMONetworkManager::MMONetworkStats GetNetworkStats() const;

    private:
        MMOClient() = default;
        ~MMOClient() = default;

        // Core systems
        MMONetworkManager* m_NetworkManager = nullptr;
        ServerManager m_ServerManager;
        CharacterManager m_CharacterManager;
        WorldSynchronizer m_WorldSynchronizer;
        TradeSystem m_TradeSystem;
        GuildSystem m_GuildSystem;

        // State
        bool m_IsInitialized = false;
        bool m_IsLoggedIn = false;

        void RegisterPacketHandlers();
    };

} // namespace Cherry
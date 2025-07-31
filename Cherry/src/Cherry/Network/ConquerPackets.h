// Cherry/src/Cherry/Network/ConquerPackets.h
#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Network/NetworkManager.h"
#include <cstring>
#include <Cherry/Core/Log.h>

namespace Cherry {

    // Conquer Online packet types (authentic IDs from the game)
    enum class ConquerPacketType : uint16_t {
        // Authentication & Connection
        MsgConnect = 1052,
        MsgAccount = 1051,
        MsgTalk = 1004,

        // Character Management
        MsgCharacter = 1005,
        MsgUserInfo = 1006,
        MsgUserAttrib = 1017,

        // Movement & Actions
        MsgWalk = 1005,
        MsgAction = 1010,
        MsgPos = 1025,

        // Items & Equipment
        MsgItem = 1009,
        MsgItemInfo = 1008,

        // Combat & Magic
        MsgInteract = 1022,
        MsgMagicEffect = 1012,
        MsgMagicInfo = 1011,

        // Map & NPCs
        MsgMapInfo = 1101,
        MsgNpcInfo = 1030,

        // Trade & Shop
        MsgTrade = 1056,
        MsgShop = 1057,

        // Guild & Team
        MsgTeam = 1023,
        MsgSyndicate = 1102,

        // Server Messages
        MsgServer = 1070,
        MsgTick = 1012,

        // Custom Extensions
        MsgPlayerList = 2000,
        MsgServerInfo = 2001
    };

    // Base packet class following Conquer Online structure
    class ConquerPacket {
    public:
        ConquerPacket(ConquerPacketType type) : m_Type(type), m_Length(0) {}
        virtual ~ConquerPacket() = default;

        // Core packet interface
        virtual void Serialize(std::vector<uint8_t>& buffer) = 0;
        virtual bool Deserialize(const uint8_t* data, uint16_t length) = 0;

        ConquerPacketType GetType() const { return m_Type; }
        uint16_t GetLength() const { return m_Length; }

        // Utility for packet header
        struct PacketHeader {
            uint16_t Length;
            uint16_t Type;
        };

    protected:
        void WriteHeader(std::vector<uint8_t>& buffer) {
            PacketHeader header;
            header.Length = m_Length;
            header.Type = static_cast<uint16_t>(m_Type);

            buffer.resize(sizeof(PacketHeader));
            memcpy(buffer.data(), &header, sizeof(PacketHeader));
        }

        bool ReadHeader(const uint8_t* data, uint16_t length) {
            if (length < sizeof(PacketHeader)) return false;

            const PacketHeader* header = reinterpret_cast<const PacketHeader*>(data);
            m_Length = header->Length;
            return static_cast<ConquerPacketType>(header->Type) == m_Type;
        }

    protected:
        ConquerPacketType m_Type;
        uint16_t m_Length;
    };

    // Helper class for binary data reading/writing
    class BinaryWriter {
    public:
        BinaryWriter(std::vector<uint8_t>& buffer) : m_Buffer(buffer) {}

        void WriteUInt8(uint8_t value) {
            m_Buffer.push_back(value);
        }

        void WriteUInt16(uint16_t value) {
            m_Buffer.insert(m_Buffer.end(),
                reinterpret_cast<const uint8_t*>(&value),
                reinterpret_cast<const uint8_t*>(&value) + sizeof(value));
        }

        void WriteUInt32(uint32_t value) {
            m_Buffer.insert(m_Buffer.end(),
                reinterpret_cast<const uint8_t*>(&value),
                reinterpret_cast<const uint8_t*>(&value) + sizeof(value));
        }

        void WriteString(const std::string& str, uint8_t maxLength = 255) {
            uint8_t length = static_cast<uint8_t>(std::min(str.length(), static_cast<size_t>(maxLength)));
            WriteUInt8(length);
            m_Buffer.insert(m_Buffer.end(), str.begin(), str.begin() + length);

            // Pad with zeros if necessary
            if (length < maxLength) {
                m_Buffer.insert(m_Buffer.end(), maxLength - length, 0);
            }
        }

        void WriteBytes(const void* data, size_t size) {
            const uint8_t* bytes = static_cast<const uint8_t*>(data);
            m_Buffer.insert(m_Buffer.end(), bytes, bytes + size);
        }

    private:
        std::vector<uint8_t>& m_Buffer;
    };

    class BinaryReader {
    public:
        BinaryReader(const uint8_t* data, size_t size)
            : m_Data(data), m_Size(size), m_Position(0) {
        }

        uint8_t ReadUInt8() {
            if (m_Position + sizeof(uint8_t) > m_Size) return 0;
            return m_Data[m_Position++];
        }

        uint16_t ReadUInt16() {
            if (m_Position + sizeof(uint16_t) > m_Size) return 0;
            uint16_t value = *reinterpret_cast<const uint16_t*>(m_Data + m_Position);
            m_Position += sizeof(uint16_t);
            return value;
        }

        uint32_t ReadUInt32() {
            if (m_Position + sizeof(uint32_t) > m_Size) return 0;
            uint32_t value = *reinterpret_cast<const uint32_t*>(m_Data + m_Position);
            m_Position += sizeof(uint32_t);
            return value;
        }

        std::string ReadString(uint8_t maxLength = 255) {
            if (m_Position >= m_Size) return "";

            uint8_t length = ReadUInt8();
            if (length > maxLength || m_Position + length > m_Size) return "";

            std::string result(reinterpret_cast<const char*>(m_Data + m_Position), length);
            m_Position += maxLength; // Skip padding
            return result;
        }

        void ReadBytes(void* dest, size_t size) {
            if (m_Position + size > m_Size) return;
            memcpy(dest, m_Data + m_Position, size);
            m_Position += size;
        }

        bool HasMoreData() const { return m_Position < m_Size; }
        size_t GetPosition() const { return m_Position; }
        size_t GetRemaining() const { return m_Size - m_Position; }

    private:
        const uint8_t* m_Data;
        size_t m_Size;
        size_t m_Position;
    };

    // ===== SPECIFIC PACKET IMPLEMENTATIONS =====

    // MsgConnect - Server connection packet
    class MsgConnect : public ConquerPacket {
    public:
        MsgConnect() : ConquerPacket(ConquerPacketType::MsgConnect) {
            Timestamp = 0;
            ClientVersion = 0;
            memset(Reserved, 0, sizeof(Reserved));
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(uint32_t) * 2 + sizeof(Reserved);
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteUInt32(Timestamp);
            writer.WriteUInt32(ClientVersion);
            writer.WriteBytes(Reserved, sizeof(Reserved));
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            Timestamp = reader.ReadUInt32();
            ClientVersion = reader.ReadUInt32();
            reader.ReadBytes(Reserved, sizeof(Reserved));
            return true;
        }

    public:
        uint32_t Timestamp;
        uint32_t ClientVersion;
        uint8_t Reserved[12];
    };

    // MsgAccount - Login credentials
    class MsgAccount : public ConquerPacket {
    public:
        MsgAccount() : ConquerPacket(ConquerPacketType::MsgAccount) {
            memset(Username, 0, sizeof(Username));
            memset(Password, 0, sizeof(Password));
            memset(Server, 0, sizeof(Server));
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(Username) + sizeof(Password) + sizeof(Server);
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteBytes(Username, sizeof(Username));
            writer.WriteBytes(Password, sizeof(Password));
            writer.WriteBytes(Server, sizeof(Server));
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            reader.ReadBytes(Username, sizeof(Username));
            reader.ReadBytes(Password, sizeof(Password));
            reader.ReadBytes(Server, sizeof(Server));
            return true;
        }

        void SetCredentials(const std::string& username, const std::string& password, const std::string& server = "") {
            strncpy_s(Username, username.c_str(), sizeof(Username) - 1);
            strncpy_s(Password, password.c_str(), sizeof(Password) - 1);
            strncpy_s(Server, server.c_str(), sizeof(Server) - 1);
        }

    public:
        char Username[32];
        char Password[32];
        char Server[16];
    };

    // MsgWalk - Player movement
    class MsgWalk : public ConquerPacket {
    public:
        MsgWalk() : ConquerPacket(ConquerPacketType::MsgWalk) {
            Timestamp = 0;
            PlayerID = 0;
            Direction = 0;
            X = 0;
            Y = 0;
            Mode = 0;
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(uint32_t) * 2 + sizeof(uint16_t) * 3 + sizeof(uint8_t);
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteUInt32(Timestamp);
            writer.WriteUInt32(PlayerID);
            writer.WriteUInt16(Direction);
            writer.WriteUInt16(X);
            writer.WriteUInt16(Y);
            writer.WriteUInt8(Mode);
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            Timestamp = reader.ReadUInt32();
            PlayerID = reader.ReadUInt32();
            Direction = reader.ReadUInt16();
            X = reader.ReadUInt16();
            Y = reader.ReadUInt16();
            Mode = reader.ReadUInt8();
            return true;
        }

    public:
        uint32_t Timestamp;
        uint32_t PlayerID;
        uint16_t Direction;  // 0-7 (8 directions)
        uint16_t X, Y;
        uint8_t Mode;       // Walk mode (run, walk, etc.)
    };

    // MsgUserInfo - Player information
    class MsgUserInfo : public ConquerPacket {
    public:
        MsgUserInfo() : ConquerPacket(ConquerPacketType::MsgUserInfo) {
            PlayerID = 0;
            Lookface = 0;
            Hair = 0;
            Silver = 0;
            ConquerPoints = 0;
            Experience = 0;
            Level = 1;
            Strength = 0;
            Agility = 0;
            Vitality = 0;
            Spirit = 0;
            AttributePoints = 0;
            Health = 0;
            MaxHealth = 0;
            Mana = 0;
            MaxMana = 0;
            PkPoints = 0;
            Class = 0;
            X = 0;
            Y = 0;
            MapID = 0;
            Direction = 0;
            memset(PlayerName, 0, sizeof(PlayerName));
            memset(SpouseName, 0, sizeof(SpouseName));
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(uint32_t) * 15 + sizeof(uint16_t) * 6 +
                sizeof(uint8_t) * 2 + sizeof(PlayerName) + sizeof(SpouseName);
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteUInt32(PlayerID);
            writer.WriteUInt32(Lookface);
            writer.WriteUInt16(Hair);
            writer.WriteUInt32(Silver);
            writer.WriteUInt32(ConquerPoints);
            writer.WriteUInt32(Experience);
            writer.WriteUInt16(Level);
            writer.WriteUInt16(Strength);
            writer.WriteUInt16(Agility);
            writer.WriteUInt16(Vitality);
            writer.WriteUInt16(Spirit);
            writer.WriteUInt16(AttributePoints);
            writer.WriteUInt32(Health);
            writer.WriteUInt32(MaxHealth);
            writer.WriteUInt32(Mana);
            writer.WriteUInt32(MaxMana);
            writer.WriteUInt32(PkPoints);
            writer.WriteUInt8(Class);
            writer.WriteUInt32(X);
            writer.WriteUInt32(Y);
            writer.WriteUInt32(MapID);
            writer.WriteUInt8(Direction);
            writer.WriteBytes(PlayerName, sizeof(PlayerName));
            writer.WriteBytes(SpouseName, sizeof(SpouseName));
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            PlayerID = reader.ReadUInt32();
            Lookface = reader.ReadUInt32();
            Hair = reader.ReadUInt16();
            Silver = reader.ReadUInt32();
            ConquerPoints = reader.ReadUInt32();
            Experience = reader.ReadUInt32();
            Level = reader.ReadUInt16();
            Strength = reader.ReadUInt16();
            Agility = reader.ReadUInt16();
            Vitality = reader.ReadUInt16();
            Spirit = reader.ReadUInt16();
            AttributePoints = reader.ReadUInt16();
            Health = reader.ReadUInt32();
            MaxHealth = reader.ReadUInt32();
            Mana = reader.ReadUInt32();
            MaxMana = reader.ReadUInt32();
            PkPoints = reader.ReadUInt32();
            Class = reader.ReadUInt8();
            X = reader.ReadUInt32();
            Y = reader.ReadUInt32();
            MapID = reader.ReadUInt32();
            Direction = reader.ReadUInt8();
            reader.ReadBytes(PlayerName, sizeof(PlayerName));
            reader.ReadBytes(SpouseName, sizeof(SpouseName));
            return true;
        }

    public:
        uint32_t PlayerID;
        uint32_t Lookface;      // Avatar appearance
        uint16_t Hair;          // Hair style
        uint32_t Silver;
        uint32_t ConquerPoints;
        uint32_t Experience;
        uint16_t Level;
        uint16_t Strength;
        uint16_t Agility;
        uint16_t Vitality;
        uint16_t Spirit;
        uint16_t AttributePoints;
        uint32_t Health;
        uint32_t MaxHealth;
        uint32_t Mana;
        uint32_t MaxMana;
        uint32_t PkPoints;
        uint8_t Class;          // Trojan, Warrior, etc.
        uint32_t X, Y;
        uint32_t MapID;
        uint8_t Direction;
        char PlayerName[16];
        char SpouseName[16];
    };

    // MsgItem - Item management
    class MsgItem : public ConquerPacket {
    public:
        enum class Action : uint32_t {
            Buy = 1,
            Sell = 2,
            Drop = 3,
            Use = 4,
            Equip = 5,
            Unequip = 6,
            Move = 7,
            Improve = 8,
            Enchant = 9,
            Repair = 10
        };

        MsgItem() : ConquerPacket(ConquerPacketType::MsgItem) {
            Timestamp = 0;
            ItemAction = Action::Buy;
            ItemUID = 0;
            Param1 = 0;
            Param2 = 0;
            Param3 = 0;
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(uint32_t) * 6;
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteUInt32(Timestamp);
            writer.WriteUInt32(static_cast<uint32_t>(ItemAction));
            writer.WriteUInt32(ItemUID);
            writer.WriteUInt32(Param1);
            writer.WriteUInt32(Param2);
            writer.WriteUInt32(Param3);
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            Timestamp = reader.ReadUInt32();
            ItemAction = static_cast<Action>(reader.ReadUInt32());
            ItemUID = reader.ReadUInt32();
            Param1 = reader.ReadUInt32();
            Param2 = reader.ReadUInt32();
            Param3 = reader.ReadUInt32();
            return true;
        }

    public:
        uint32_t Timestamp;
        Action ItemAction;
        uint32_t ItemUID;
        uint32_t Param1;        // Context-dependent parameters
        uint32_t Param2;
        uint32_t Param3;
    };

    // MsgTalk - Chat messages
    class MsgTalk : public ConquerPacket {
    public:
        enum class Channel : uint32_t {
            Talk = 2000,
            Whisper = 2001,
            Action = 2002,
            Team = 2003,
            Guild = 2004,
            System = 2005,
            Yell = 2006,
            Friend = 2007,
            Global = 2008,
            GM = 2009,
            Ghost = 2010
        };

        MsgTalk() : ConquerPacket(ConquerPacketType::MsgTalk) {
            Color = 0xFFFFFFFF;
            TalkChannel = Channel::Talk;
            Style = 0;
            Timestamp = 0;
            SenderID = 0;
            RecipientID = 0;
            memset(SenderName, 0, sizeof(SenderName));
            memset(RecipientName, 0, sizeof(RecipientName));
            memset(Message, 0, sizeof(Message));
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(uint32_t) * 5 + sizeof(uint8_t) +
                sizeof(SenderName) + sizeof(RecipientName) + sizeof(Message);
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteUInt32(Color);
            writer.WriteUInt32(static_cast<uint32_t>(TalkChannel));
            writer.WriteUInt8(Style);
            writer.WriteUInt32(Timestamp);
            writer.WriteUInt32(SenderID);
            writer.WriteUInt32(RecipientID);
            writer.WriteBytes(SenderName, sizeof(SenderName));
            writer.WriteBytes(RecipientName, sizeof(RecipientName));
            writer.WriteBytes(Message, sizeof(Message));
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            Color = reader.ReadUInt32();
            TalkChannel = static_cast<Channel>(reader.ReadUInt32());
            Style = reader.ReadUInt8();
            Timestamp = reader.ReadUInt32();
            SenderID = reader.ReadUInt32();
            RecipientID = reader.ReadUInt32();
            reader.ReadBytes(SenderName, sizeof(SenderName));
            reader.ReadBytes(RecipientName, sizeof(RecipientName));
            reader.ReadBytes(Message, sizeof(Message));
            return true;
        }

        void SetMessage(const std::string& sender, const std::string& message,
            const std::string& recipient = "", Channel channel = Channel::Talk) {
            strncpy_s(SenderName, sender.c_str(), sizeof(SenderName) - 1);
            strncpy_s(Message, message.c_str(), sizeof(Message) - 1);
            strncpy_s(RecipientName, recipient.c_str(), sizeof(RecipientName) - 1);
            TalkChannel = channel;
        }

    public:
        uint32_t Color;
        Channel TalkChannel;
        uint8_t Style;
        uint32_t Timestamp;
        uint32_t SenderID;
        uint32_t RecipientID;
        char SenderName[16];
        char RecipientName[16];
        char Message[256];
    };

    // MsgAction - Player actions (attack, emote, etc.)
    class MsgAction : public ConquerPacket {
    public:
        enum class ActionType : uint32_t {
            None = 0,
            Dance = 1,
            Wave = 2,
            Bow = 3,
            Kneel = 4,
            Sit = 5,
            Lie = 6,
            Attack = 7,
            Magic = 8,
            Die = 9,
            Revive = 10,
            Portal = 11,
            Transform = 12
        };

        MsgAction() : ConquerPacket(ConquerPacketType::MsgAction) {
            Timestamp = 0;
            PlayerID = 0;
            TargetID = 0;
            X = 0;
            Y = 0;
            Direction = 0;
            Action = ActionType::None;
            Data = 0;
        }

        void Serialize(std::vector<uint8_t>& buffer) override {
            m_Length = sizeof(PacketHeader) + sizeof(uint32_t) * 6 + sizeof(uint16_t) * 3;
            WriteHeader(buffer);

            BinaryWriter writer(buffer);
            writer.WriteUInt32(Timestamp);
            writer.WriteUInt32(PlayerID);
            writer.WriteUInt32(TargetID);
            writer.WriteUInt16(X);
            writer.WriteUInt16(Y);
            writer.WriteUInt16(Direction);
            writer.WriteUInt32(static_cast<uint32_t>(Action));
            writer.WriteUInt32(Data);
        }

        bool Deserialize(const uint8_t* data, uint16_t length) override {
            if (!ReadHeader(data, length)) return false;

            BinaryReader reader(data + sizeof(PacketHeader), length - sizeof(PacketHeader));
            Timestamp = reader.ReadUInt32();
            PlayerID = reader.ReadUInt32();
            TargetID = reader.ReadUInt32();
            X = reader.ReadUInt16();
            Y = reader.ReadUInt16();
            Direction = reader.ReadUInt16();
            Action = static_cast<ActionType>(reader.ReadUInt32());
            Data = reader.ReadUInt32();
            return true;
        }

    public:
        uint32_t Timestamp;
        uint32_t PlayerID;
        uint32_t TargetID;
        uint16_t X, Y;
        uint16_t Direction;
        ActionType Action;
        uint32_t Data;          // Action-specific data
    };

    // Packet factory for creating packets by type
    class ConquerPacketFactory {
    public:
        static std::unique_ptr<ConquerPacket> CreatePacket(ConquerPacketType type) {
            switch (type) {
            case ConquerPacketType::MsgConnect:
                return std::make_unique<MsgConnect>();
            case ConquerPacketType::MsgAccount:
                return std::make_unique<MsgAccount>();
            case ConquerPacketType::MsgWalk:
                return std::make_unique<MsgWalk>();
            case ConquerPacketType::MsgUserInfo:
                return std::make_unique<MsgUserInfo>();
            case ConquerPacketType::MsgItem:
                return std::make_unique<MsgItem>();
            case ConquerPacketType::MsgTalk:
                return std::make_unique<MsgTalk>();
            case ConquerPacketType::MsgAction:
                return std::make_unique<MsgAction>();
            default:
                CH_CORE_WARN("Unknown packet type: {}", static_cast<uint16_t>(type));
                return nullptr;
            }
        }

        static ConquerPacketType GetPacketType(const uint8_t* data, uint16_t length) {
            if (length < sizeof(ConquerPacket::PacketHeader)) {
                return static_cast<ConquerPacketType>(0);
            }

            const ConquerPacket::PacketHeader* header =
                reinterpret_cast<const ConquerPacket::PacketHeader*>(data);
            return static_cast<ConquerPacketType>(header->Type);
        }
    };

} // namespace Cherry
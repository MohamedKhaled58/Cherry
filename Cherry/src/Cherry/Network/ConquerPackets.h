#pragma once
#include <cstdint>

namespace Cherry {
    enum class PacketType : uint16_t {
        // Authentication
        MsgAccount = 1051,
        MsgConnect = 1052,

        // Character
        MsgCharacter = 1004,
        MsgUserInfo = 1006,

        // Movement
        MsgWalk = 1005,
        MsgAction = 1010,

        // Items
        MsgItem = 1009,
        MsgItemInfo = 1008,

        // Combat
        MsgInteract = 1022,
        MsgMagicEffect = 1012,

        // Chat
        MsgTalk = 1004,

        // Custom packets...
    };

    class ConquerPacket {
    public:
        virtual ~ConquerPacket() = default;
        virtual PacketType GetType() const = 0;
        virtual void Encode(NetworkWriter& writer) = 0;
        virtual bool Decode(NetworkReader& reader) = 0;
        virtual uint16_t GetLength() const = 0;
    };

    // Example: Walk packet
    class MsgWalk : public ConquerPacket {
    public:
        PacketType GetType() const override { return PacketType::MsgWalk; }

        void Encode(NetworkWriter& writer) override {
            writer.WriteUInt16(GetLength());
            writer.WriteUInt16(static_cast<uint16_t>(GetType()));
            writer.WriteUInt32(Timestamp);
            writer.WriteUInt32(PlayerID);
            writer.WriteUInt16(Direction);
            writer.WriteUInt16(X);
            writer.WriteUInt16(Y);
        }

        uint32_t Timestamp;
        uint32_t PlayerID;
        uint16_t Direction;
        uint16_t X, Y;
    };
}
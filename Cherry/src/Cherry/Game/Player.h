#pragma once
#include <string>
#include <unordered_map>
// Cherry/src/Cherry/Game/Player.h
namespace Cherry {
    struct PlayerData {
        uint32_t PlayerID;
        std::string Name;
        uint32_t Level;
        uint32_t Experience;

        // Conquer Online specific
        uint32_t Class;         // Trojan, Warrior, Archer, etc.
        uint32_t Profession;    // First profession
        uint32_t ProfessionLvl;
        uint32_t Silver;
        uint32_t ConquerPoints;

        // Position
        uint32_t MapID;
        uint16_t X, Y;
        uint8_t Direction;

        // Status
        uint32_t Health;
        uint32_t MaxHealth;
        uint32_t Mana;
        uint32_t MaxMana;

        // Equipment (18 slots like CO)
        struct Equipment {
            uint32_t Headgear;
            uint32_t Necklace;
            uint32_t Armor;
            uint32_t RightWeapon;
            uint32_t LeftWeapon;
            uint32_t Ring;
            uint32_t Gourd;
            uint32_t Boots;
            uint32_t Garment;
            // ... other slots
        } Equipment;
    };

    class PlayerManager {
    public:
        static PlayerManager& Get();

        bool LoadPlayer(uint32_t playerID);
        void SavePlayer(uint32_t playerID);
        PlayerData* GetPlayer(uint32_t playerID);

        // Movement
        void MovePlayer(uint32_t playerID, uint16_t x, uint16_t y);
        bool CanMoveTo(uint32_t mapID, uint16_t x, uint16_t y);

    private:
        std::unordered_map<uint32_t, PlayerData> m_Players;
    };
}
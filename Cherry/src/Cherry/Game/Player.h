#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Network/ConquerPackets.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace Cherry {

    // Conquer Online class types
    enum class ConquerClass : uint8_t {
        Trojan = 10,
        Warrior = 20,
        Archer = 40,
        Ninja = 50,
        Monk = 60,
        Pirate = 70,
        DragonWarrior = 80,
        // Add more classes as needed
    };

    // Equipment positions (18 slots like CO)
    enum class EquipPosition : uint8_t {
        Headgear = 1,
        Necklace = 2,
        Armor = 3,
        RightWeapon = 4,
        LeftWeapon = 5,
        Ring = 6,
        Gourd = 7,
        Boots = 8,
        Garment = 9,
        AttackTalisman = 10,
        DefenseTalisman = 11,
        Steed = 12,
        RightWeaponAccessory = 15,
        LeftWeaponAccessory = 16,
        SteedArmor = 17,
        Crop = 18
    };

    // Player status flags
    enum class PlayerStatus : uint32_t {
        None = 0,
        Dead = 1 << 0,
        Invisible = 1 << 1,
        Poisoned = 1 << 2,
        Flying = 1 << 3,
        Team = 1 << 4,
        Superman = 1 << 5,
        Cyclone = 1 << 6,
        Stigma = 1 << 7,
        Shield = 1 << 8,
        Star = 1 << 9,
        Accuracy = 1 << 10,
        MagicShield = 1 << 11
    };

    // Equipment item structure
    struct EquipmentItem {
        uint32_t ItemID = 0;
        uint32_t Type = 0;          // Item type
        uint8_t Plus = 0;           // Enhancement level (+1, +2, etc.)
        uint8_t Durability = 100;
        uint8_t MaxDurability = 100;
        uint32_t Socket1 = 0;       // Gem socket 1
        uint32_t Socket2 = 0;       // Gem socket 2
        bool IsBlessed = false;
        uint32_t Color = 3;         // Item color

        bool IsValid() const { return ItemID != 0; }
        void Clear() {
            ItemID = 0; Type = 0; Plus = 0;
            Durability = 100; MaxDurability = 100;
            Socket1 = 0; Socket2 = 0; IsBlessed = false; Color = 3;
        }
    };

    // Player data structure (comprehensive like CO)
    struct PlayerData {
        // Identity
        uint32_t PlayerID = 0;
        std::string Name;
        std::string SpouseName;

        // Appearance
        uint32_t Lookface = 0;      // Base avatar model
        uint16_t Hair = 0;          // Hair style
        uint32_t Transformation = 0; // Current transformation

        // Class & Level
        ConquerClass Class = ConquerClass::Trojan;
        uint16_t Level = 1;
        uint32_t Experience = 0;
        uint32_t ExperienceToNext = 1000;

        // Attributes
        uint16_t Strength = 10;
        uint16_t Agility = 10;
        uint16_t Vitality = 10;
        uint16_t Spirit = 10;
        uint16_t AttributePoints = 0;

        // Health & Mana
        uint32_t Health = 100;
        uint32_t MaxHealth = 100;
        uint32_t Mana = 100;
        uint32_t MaxMana = 100;

        // Resources
        uint32_t Silver = 0;
        uint32_t ConquerPoints = 0;
        uint32_t PkPoints = 0;

        // Position
        uint32_t MapID = 1002;
        uint16_t X = 300;
        uint16_t Y = 300;
        uint8_t Direction = 0;      // 0-7 (8 directions)

        // Status
        PlayerStatus Status = PlayerStatus::None;
        uint32_t Energy = 100;      // Stamina/Energy

        // Equipment (18 slots)
        std::unordered_map<EquipPosition, EquipmentItem> Equipment;

        // Inventory
        std::vector<EquipmentItem> Inventory; // 40 slots typically

        // Skills & Magic
        std::unordered_map<uint16_t, uint16_t> Skills; // SkillID -> Level

        // Statistics
        uint32_t TotalOnlineTime = 0;
        uint32_t LoginCount = 0;

        // Methods
        void Clear() {
            PlayerID = 0; Name.clear(); SpouseName.clear();
            Lookface = 0; Hair = 0; Transformation = 0;
            Class = ConquerClass::Trojan; Level = 1; Experience = 0;
            Strength = 10; Agility = 10; Vitality = 10; Spirit = 10;
            AttributePoints = 0; Health = 100; MaxHealth = 100;
            Mana = 100; MaxMana = 100; Silver = 0; ConquerPoints = 0;
            PkPoints = 0; MapID = 1002; X = 300; Y = 300; Direction = 0;
            Status = PlayerStatus::None; Energy = 100;
            Equipment.clear(); Inventory.clear(); Skills.clear();
            TotalOnlineTime = 0; LoginCount = 0;
        }

        bool HasEquipment(EquipPosition pos) const {
            auto it = Equipment.find(pos);
            return it != Equipment.end() && it->second.IsValid();
        }

        const EquipmentItem* GetEquipment(EquipPosition pos) const {
            auto it = Equipment.find(pos);
            return (it != Equipment.end()) ? &it->second : nullptr;
        }

        void SetEquipment(EquipPosition pos, const EquipmentItem& item) {
            Equipment[pos] = item;
        }

        void RemoveEquipment(EquipPosition pos) {
            auto it = Equipment.find(pos);
            if (it != Equipment.end()) {
                it->second.Clear();
            }
        }

        // Calculate battle power (simplified)
        uint32_t GetBattlePower() const {
            uint32_t power = Level * 100;
            power += Strength * 5;
            power += Agility * 3;
            power += Vitality * 4;
            power += Spirit * 2;

            // Add equipment bonuses
            for (const auto& [pos, item] : Equipment) {
                if (item.IsValid()) {
                    power += item.Plus * 500; // Enhancement bonus
                }
            }

            return power;
        }
    };

    // Player manager class
    class PlayerManager {
    public:
        static PlayerManager& Get() {
            static PlayerManager instance;
            return instance;
        }

        // Player data management
        bool LoadPlayer(uint32_t playerID);
        bool SavePlayer(uint32_t playerID);
        bool CreatePlayer(const std::string& name, ConquerClass playerClass, uint32_t& newPlayerID);
        bool DeletePlayer(uint32_t playerID);

        // Player access
        PlayerData* GetPlayer(uint32_t playerID);
        const PlayerData* GetPlayer(uint32_t playerID) const;
        PlayerData* GetPlayerByName(const std::string& name);

        // Local player (current user)
        void SetLocalPlayer(uint32_t playerID) { m_LocalPlayerID = playerID; }
        uint32_t GetLocalPlayerID() const { return m_LocalPlayerID; }
        PlayerData* GetLocalPlayer() { return GetPlayer(m_LocalPlayerID); }

        // Player list management
        std::vector<uint32_t> GetPlayersInMap(uint32_t mapID) const;
        std::vector<uint32_t> GetPlayersInRange(uint32_t mapID, uint16_t x, uint16_t y, uint16_t range) const;

        // Movement
        bool MovePlayer(uint32_t playerID, uint16_t x, uint16_t y, uint8_t direction);
        bool TeleportPlayer(uint32_t playerID, uint32_t mapID, uint16_t x, uint16_t y);

        // Equipment
        bool EquipItem(uint32_t playerID, const EquipmentItem& item, EquipPosition position);
        bool UnequipItem(uint32_t playerID, EquipPosition position);
        bool CanEquip(uint32_t playerID, const EquipmentItem& item, EquipPosition position) const;

        // Statistics
        size_t GetPlayerCount() const { return m_Players.size(); }
        void PrintPlayerInfo(uint32_t playerID) const;

    private:
        PlayerManager() = default;
        ~PlayerManager() = default;

        bool ValidatePlayerData(const PlayerData& player) const;
        uint32_t GenerateNewPlayerID();

    private:
        std::unordered_map<uint32_t, PlayerData> m_Players;
        uint32_t m_LocalPlayerID = 0;
        uint32_t m_NextPlayerID = 1000000; // Start player IDs from 1M
        mutable std::mutex m_PlayersMutex;
    };

    // Global access
    extern PlayerManager& g_PlayerManager;

} // namespace Cherry
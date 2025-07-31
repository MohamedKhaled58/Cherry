#pragma once
#include <string>
#include <unordered_map>
namespace Cherry {
    enum class ItemType : uint8_t {
        Weapon = 1,
        Armor = 2,
        Helmet = 3,
        Necklace = 4,
        Ring = 5,
        Boots = 6,
        // ... other types
    };

    struct ItemData {
        uint32_t ID;
        ItemType Type;
        uint8_t RequiredLevel;
        uint8_t RequiredClass;

        // Stats
        uint16_t MinAttack;
        uint16_t MaxAttack;
        uint16_t Defense;
        uint16_t Dodge;
        uint16_t MagicAttack;
        uint16_t MagicDefense;

        // Gem sockets (like CO)
        uint8_t SocketCount;
        uint32_t Socket1, Socket2;

        // Enhancement
        uint8_t Plus;           // +1, +2, etc.
        bool IsBlessed;
        uint8_t Durability;
        uint8_t MaxDurability;

        std::string Name;
        std::string Description;
    };

    class ItemManager {
    public:
        static ItemManager& Get();

        ItemData* GetItemData(uint32_t itemID);
        bool LoadItemDatabase(const std::string& filename);

        // Equipment validation
        bool CanEquip(uint32_t playerID, uint32_t itemID);
        bool EquipItem(uint32_t playerID, uint32_t itemID, uint8_t position);

    private:
        std::unordered_map<uint32_t, ItemData> m_ItemDatabase;
    };
}
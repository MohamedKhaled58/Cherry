// Cherry/src/Cherry/Game/Player.cpp
#include "CHpch.h"
#include "Player.h"
#include "Cherry/Resources/PackageManager.h"
#include <algorithm>
#include <fstream>

namespace Cherry {

    // Global instance
    PlayerManager& g_PlayerManager = PlayerManager::Get();

    bool PlayerManager::LoadPlayer(uint32_t playerID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        // Check if already loaded
        if (m_Players.find(playerID) != m_Players.end()) {
            return true;
        }

        // Try loading from package first
        std::string filename = "players/" + std::to_string(playerID) + ".dat";
        uint32_t size;
        void* data = g_PackageManager.LoadFile(filename, size);

        if (data) {
            // Load from package data
            PlayerData player;
            if (size >= sizeof(PlayerData)) {
                memcpy(&player, data, sizeof(PlayerData));
                if (ValidatePlayerData(player)) {
                    m_Players[playerID] = player;
                    free(data);
                    CH_CORE_INFO("Loaded player {} from package", playerID);
                    return true;
                }
            }
            free(data);
        }

        // Fall back to file system
        std::string filepath = "data/players/" + std::to_string(playerID) + ".dat";
        std::ifstream file(filepath, std::ios::binary);
        if (file.is_open()) {
            PlayerData player;
            file.read(reinterpret_cast<char*>(&player), sizeof(PlayerData));

            if (file.good() && ValidatePlayerData(player)) {
                m_Players[playerID] = player;
                file.close();
                CH_CORE_INFO("Loaded player {} from file", playerID);
                return true;
            }
            file.close();
        }

        CH_CORE_ERROR("Failed to load player {}", playerID);
        return false;
    }

    bool PlayerManager::SavePlayer(uint32_t playerID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            CH_CORE_ERROR("Player {} not found for saving", playerID);
            return false;
        }

        // Ensure directory exists
        std::filesystem::create_directories("data/players");

        std::string filepath = "data/players/" + std::to_string(playerID) + ".dat";
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            CH_CORE_ERROR("Failed to open file for saving player {}", playerID);
            return false;
        }

        file.write(reinterpret_cast<const char*>(&it->second), sizeof(PlayerData));
        file.close();

        if (file.good()) {
            CH_CORE_INFO("Saved player {} to file", playerID);
            return true;
        }
        else {
            CH_CORE_ERROR("Failed to write player {} data", playerID);
            return false;
        }
    }

    bool PlayerManager::CreatePlayer(const std::string& name, ConquerClass playerClass, uint32_t& newPlayerID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        // Check if name already exists
        for (const auto& [id, player] : m_Players) {
            if (player.Name == name) {
                CH_CORE_ERROR("Player name '{}' already exists", name);
                return false;
            }
        }

        newPlayerID = GenerateNewPlayerID();

        PlayerData newPlayer;
        newPlayer.Clear();
        newPlayer.PlayerID = newPlayerID;
        newPlayer.Name = name;
        newPlayer.Class = playerClass;

        // Set default stats based on class
        switch (playerClass) {
        case ConquerClass::Trojan:
            newPlayer.Strength = 15;
            newPlayer.Agility = 8;
            newPlayer.Vitality = 12;
            newPlayer.Spirit = 5;
            break;
        case ConquerClass::Warrior:
            newPlayer.Strength = 12;
            newPlayer.Agility = 10;
            newPlayer.Vitality = 15;
            newPlayer.Spirit = 3;
            break;
        case ConquerClass::Archer:
            newPlayer.Strength = 8;
            newPlayer.Agility = 15;
            newPlayer.Vitality = 10;
            newPlayer.Spirit = 7;
            break;
        default:
            newPlayer.Strength = 10;
            newPlayer.Agility = 10;
            newPlayer.Vitality = 10;
            newPlayer.Spirit = 10;
            break;
        }

        // Calculate initial health/mana
        newPlayer.MaxHealth = 100 + newPlayer.Vitality * 24;
        newPlayer.Health = newPlayer.MaxHealth;
        newPlayer.MaxMana = 100 + newPlayer.Spirit * 5;
        newPlayer.Mana = newPlayer.MaxMana;

        m_Players[newPlayerID] = newPlayer;

        CH_CORE_INFO("Created new player: {} (ID: {}, Class: {})",
            name, newPlayerID, static_cast<int>(playerClass));

        return SavePlayer(newPlayerID);
    }

    PlayerData* PlayerManager::GetPlayer(uint32_t playerID) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);
        auto it = m_Players.find(playerID);
        return (it != m_Players.end()) ? &it->second : nullptr;
    }

    const PlayerData* PlayerManager::GetPlayer(uint32_t playerID) const {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);
        auto it = m_Players.find(playerID);
        return (it != m_Players.end()) ? &it->second : nullptr;
    }

    PlayerData* PlayerManager::GetPlayerByName(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);
        for (auto& [id, player] : m_Players) {
            if (player.Name == name) {
                return &player;
            }
        }
        return nullptr;
    }

    std::vector<uint32_t> PlayerManager::GetPlayersInMap(uint32_t mapID) const {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);
        std::vector<uint32_t> result;

        for (const auto& [id, player] : m_Players) {
            if (player.MapID == mapID) {
                result.push_back(id);
            }
        }

        return result;
    }

    std::vector<uint32_t> PlayerManager::GetPlayersInRange(uint32_t mapID, uint16_t x, uint16_t y, uint16_t range) const {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);
        std::vector<uint32_t> result;

        for (const auto& [id, player] : m_Players) {
            if (player.MapID == mapID) {
                int dx = static_cast<int>(player.X) - static_cast<int>(x);
                int dy = static_cast<int>(player.Y) - static_cast<int>(y);
                int distance = static_cast<int>(sqrt(dx * dx + dy * dy));

                if (distance <= range) {
                    result.push_back(id);
                }
            }
        }

        return result;
    }

    bool PlayerManager::MovePlayer(uint32_t playerID, uint16_t x, uint16_t y, uint8_t direction) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            return false;
        }

        // TODO: Validate movement with map collision

        it->second.X = x;
        it->second.Y = y;
        it->second.Direction = direction;

        return true;
    }

    bool PlayerManager::TeleportPlayer(uint32_t playerID, uint32_t mapID, uint16_t x, uint16_t y) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            return false;
        }

        it->second.MapID = mapID;
        it->second.X = x;
        it->second.Y = y;

        CH_CORE_INFO("Teleported player {} to map {} at ({}, {})",
            playerID, mapID, x, y);

        return true;
    }

    bool PlayerManager::EquipItem(uint32_t playerID, const EquipmentItem& item, EquipPosition position) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            return false;
        }

        if (!CanEquip(playerID, item, position)) {
            return false;
        }

        it->second.SetEquipment(position, item);
        CH_CORE_TRACE("Player {} equipped item {} at position {}",
            playerID, item.ItemID, static_cast<int>(position));

        return true;
    }

    bool PlayerManager::UnequipItem(uint32_t playerID, EquipPosition position) {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            return false;
        }

        it->second.RemoveEquipment(position);
        CH_CORE_TRACE("Player {} unequipped item from position {}",
            playerID, static_cast<int>(position));

        return true;
    }

    bool PlayerManager::CanEquip(uint32_t playerID, const EquipmentItem& item, EquipPosition position) const {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            return false;
        }

        const PlayerData& player = it->second;

        // TODO: Implement item type checking, level requirements, etc.
        // For now, just check if item is valid
        return item.IsValid();
    }

    void PlayerManager::PrintPlayerInfo(uint32_t playerID) const {
        std::lock_guard<std::mutex> lock(m_PlayersMutex);

        auto it = m_Players.find(playerID);
        if (it == m_Players.end()) {
            CH_CORE_INFO("Player {} not found", playerID);
            return;
        }

        const PlayerData& player = it->second;
        CH_CORE_INFO("=== Player Info: {} ===", player.Name);
        CH_CORE_INFO("ID: {}, Level: {}, Class: {}",
            player.PlayerID, player.Level, static_cast<int>(player.Class));
        CH_CORE_INFO("Position: Map {}, ({}, {})",
            player.MapID, player.X, player.Y);
        CH_CORE_INFO("Health: {}/{}, Mana: {}/{}",
            player.Health, player.MaxHealth, player.Mana, player.MaxMana);
        CH_CORE_INFO("Stats: STR={}, AGI={}, VIT={}, SPI={}",
            player.Strength, player.Agility, player.Vitality, player.Spirit);
        CH_CORE_INFO("Resources: Silver={}, CPs={}",
            player.Silver, player.ConquerPoints);
        CH_CORE_INFO("Battle Power: {}", player.GetBattlePower());
    }

    bool PlayerManager::ValidatePlayerData(const PlayerData& player) const {
        return player.PlayerID > 0 &&
            !player.Name.empty() &&
            player.Level > 0 &&
            player.Level <= 140 && // Max level check
            player.MaxHealth > 0 &&
            player.MaxMana > 0;
    }

    uint32_t PlayerManager::GenerateNewPlayerID() {
        return m_NextPlayerID++;
    }

} // namespace Cherry

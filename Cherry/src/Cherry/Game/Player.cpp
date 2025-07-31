// Cherry/src/Cherry/Game/Player.cpp
#include "CHpch.h"
#include "Player.h"
#include "Cherry/Resources/PackageManager.h"
#include <algorithm>
#include <fstream>

// JSON serialization includes
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

namespace Cherry {

    // Global instance
    PlayerManager& g_PlayerManager = PlayerManager::Get();

    // JSON Serialization Helper Functions
    namespace {
        rapidjson::Value PlayerDataToJson(const PlayerData& player, rapidjson::Document::AllocatorType& allocator) {
            rapidjson::Value json(rapidjson::kObjectType);
            
            // Basic identity
            json.AddMember("PlayerID", player.PlayerID, allocator);
            json.AddMember("Name", rapidjson::Value(player.Name.c_str(), allocator), allocator);
            json.AddMember("SpouseName", rapidjson::Value(player.SpouseName.c_str(), allocator), allocator);
            
            // Appearance
            json.AddMember("Lookface", player.Lookface, allocator);
            json.AddMember("Hair", player.Hair, allocator);
            json.AddMember("Transformation", player.Transformation, allocator);
            
            // Class & Level
            json.AddMember("Class", static_cast<uint32_t>(player.Class), allocator);
            json.AddMember("Level", player.Level, allocator);
            json.AddMember("Experience", player.Experience, allocator);
            json.AddMember("ExperienceToNext", player.ExperienceToNext, allocator);
            
            // Attributes
            json.AddMember("Strength", player.Strength, allocator);
            json.AddMember("Agility", player.Agility, allocator);
            json.AddMember("Vitality", player.Vitality, allocator);
            json.AddMember("Spirit", player.Spirit, allocator);
            json.AddMember("AttributePoints", player.AttributePoints, allocator);
            
            // Health & Mana
            json.AddMember("Health", player.Health, allocator);
            json.AddMember("MaxHealth", player.MaxHealth, allocator);
            json.AddMember("Mana", player.Mana, allocator);
            json.AddMember("MaxMana", player.MaxMana, allocator);
            
            // Resources
            json.AddMember("Silver", player.Silver, allocator);
            json.AddMember("ConquerPoints", player.ConquerPoints, allocator);
            json.AddMember("PkPoints", player.PkPoints, allocator);
            
            // Position
            json.AddMember("MapID", player.MapID, allocator);
            json.AddMember("X", player.X, allocator);
            json.AddMember("Y", player.Y, allocator);
            json.AddMember("Direction", player.Direction, allocator);
            
            // Status
            json.AddMember("Status", static_cast<uint32_t>(player.Status), allocator);
            json.AddMember("Energy", player.Energy, allocator);
            
            // Equipment
            rapidjson::Value equipmentArray(rapidjson::kArrayType);
            for (const auto& [pos, item] : player.Equipment) {
                rapidjson::Value equipItem(rapidjson::kObjectType);
                equipItem.AddMember("Position", static_cast<uint32_t>(pos), allocator);
                equipItem.AddMember("ItemID", item.ItemID, allocator);
                equipItem.AddMember("Type", item.Type, allocator);
                equipItem.AddMember("Plus", item.Plus, allocator);
                equipItem.AddMember("Durability", item.Durability, allocator);
                equipItem.AddMember("MaxDurability", item.MaxDurability, allocator);
                equipItem.AddMember("Socket1", item.Socket1, allocator);
                equipItem.AddMember("Socket2", item.Socket2, allocator);
                equipItem.AddMember("IsBlessed", item.IsBlessed, allocator);
                equipItem.AddMember("Color", item.Color, allocator);
                equipmentArray.PushBack(equipItem, allocator);
            }
            json.AddMember("Equipment", equipmentArray, allocator);
            
            // Inventory
            rapidjson::Value inventoryArray(rapidjson::kArrayType);
            for (const auto& item : player.Inventory) {
                rapidjson::Value invItem(rapidjson::kObjectType);
                invItem.AddMember("ItemID", item.ItemID, allocator);
                invItem.AddMember("Type", item.Type, allocator);
                invItem.AddMember("Plus", item.Plus, allocator);
                invItem.AddMember("Durability", item.Durability, allocator);
                invItem.AddMember("MaxDurability", item.MaxDurability, allocator);
                invItem.AddMember("Socket1", item.Socket1, allocator);
                invItem.AddMember("Socket2", item.Socket2, allocator);
                invItem.AddMember("IsBlessed", item.IsBlessed, allocator);
                invItem.AddMember("Color", item.Color, allocator);
                inventoryArray.PushBack(invItem, allocator);
            }
            json.AddMember("Inventory", inventoryArray, allocator);
            
            // Skills
            rapidjson::Value skillsArray(rapidjson::kArrayType);
            for (const auto& [skillId, level] : player.Skills) {
                rapidjson::Value skill(rapidjson::kObjectType);
                skill.AddMember("SkillID", skillId, allocator);
                skill.AddMember("Level", level, allocator);
                skillsArray.PushBack(skill, allocator);
            }
            json.AddMember("Skills", skillsArray, allocator);
            
            // Statistics
            json.AddMember("TotalOnlineTime", player.TotalOnlineTime, allocator);
            json.AddMember("LoginCount", player.LoginCount, allocator);
            
            return json;
        }
        
        bool JsonToPlayerData(const rapidjson::Value& json, PlayerData& player) {
            if (!json.IsObject()) return false;
            
            try {
                // Basic identity
                if (json.HasMember("PlayerID") && json["PlayerID"].IsUint()) 
                    player.PlayerID = json["PlayerID"].GetUint();
                if (json.HasMember("Name") && json["Name"].IsString()) 
                    player.Name = json["Name"].GetString();
                if (json.HasMember("SpouseName") && json["SpouseName"].IsString()) 
                    player.SpouseName = json["SpouseName"].GetString();
                
                // Appearance
                if (json.HasMember("Lookface") && json["Lookface"].IsUint()) 
                    player.Lookface = json["Lookface"].GetUint();
                if (json.HasMember("Hair") && json["Hair"].IsUint()) 
                    player.Hair = json["Hair"].GetUint();
                if (json.HasMember("Transformation") && json["Transformation"].IsUint()) 
                    player.Transformation = json["Transformation"].GetUint();
                
                // Class & Level
                if (json.HasMember("Class") && json["Class"].IsUint()) 
                    player.Class = static_cast<ConquerClass>(json["Class"].GetUint());
                if (json.HasMember("Level") && json["Level"].IsUint()) 
                    player.Level = json["Level"].GetUint();
                if (json.HasMember("Experience") && json["Experience"].IsUint()) 
                    player.Experience = json["Experience"].GetUint();
                if (json.HasMember("ExperienceToNext") && json["ExperienceToNext"].IsUint()) 
                    player.ExperienceToNext = json["ExperienceToNext"].GetUint();
                
                // Attributes
                if (json.HasMember("Strength") && json["Strength"].IsUint()) 
                    player.Strength = json["Strength"].GetUint();
                if (json.HasMember("Agility") && json["Agility"].IsUint()) 
                    player.Agility = json["Agility"].GetUint();
                if (json.HasMember("Vitality") && json["Vitality"].IsUint()) 
                    player.Vitality = json["Vitality"].GetUint();
                if (json.HasMember("Spirit") && json["Spirit"].IsUint()) 
                    player.Spirit = json["Spirit"].GetUint();
                if (json.HasMember("AttributePoints") && json["AttributePoints"].IsUint()) 
                    player.AttributePoints = json["AttributePoints"].GetUint();
                
                // Health & Mana
                if (json.HasMember("Health") && json["Health"].IsUint()) 
                    player.Health = json["Health"].GetUint();
                if (json.HasMember("MaxHealth") && json["MaxHealth"].IsUint()) 
                    player.MaxHealth = json["MaxHealth"].GetUint();
                if (json.HasMember("Mana") && json["Mana"].IsUint()) 
                    player.Mana = json["Mana"].GetUint();
                if (json.HasMember("MaxMana") && json["MaxMana"].IsUint()) 
                    player.MaxMana = json["MaxMana"].GetUint();
                
                // Resources
                if (json.HasMember("Silver") && json["Silver"].IsUint()) 
                    player.Silver = json["Silver"].GetUint();
                if (json.HasMember("ConquerPoints") && json["ConquerPoints"].IsUint()) 
                    player.ConquerPoints = json["ConquerPoints"].GetUint();
                if (json.HasMember("PkPoints") && json["PkPoints"].IsUint()) 
                    player.PkPoints = json["PkPoints"].GetUint();
                
                // Position
                if (json.HasMember("MapID") && json["MapID"].IsUint()) 
                    player.MapID = json["MapID"].GetUint();
                if (json.HasMember("X") && json["X"].IsUint()) 
                    player.X = json["X"].GetUint();
                if (json.HasMember("Y") && json["Y"].IsUint()) 
                    player.Y = json["Y"].GetUint();
                if (json.HasMember("Direction") && json["Direction"].IsUint()) 
                    player.Direction = json["Direction"].GetUint();
                
                // Status
                if (json.HasMember("Status") && json["Status"].IsUint()) 
                    player.Status = static_cast<PlayerStatus>(json["Status"].GetUint());
                if (json.HasMember("Energy") && json["Energy"].IsUint()) 
                    player.Energy = json["Energy"].GetUint();
                
                // Equipment
                if (json.HasMember("Equipment") && json["Equipment"].IsArray()) {
                    player.Equipment.clear();
                    for (const auto& equipJson : json["Equipment"].GetArray()) {
                        if (!equipJson.IsObject()) continue;
                        
                        EquipPosition pos = static_cast<EquipPosition>(equipJson["Position"].GetUint());
                        EquipmentItem item;
                        item.ItemID = equipJson["ItemID"].GetUint();
                        item.Type = equipJson["Type"].GetUint();
                        item.Plus = equipJson["Plus"].GetUint();
                        item.Durability = equipJson["Durability"].GetUint();
                        item.MaxDurability = equipJson["MaxDurability"].GetUint();
                        item.Socket1 = equipJson["Socket1"].GetUint();
                        item.Socket2 = equipJson["Socket2"].GetUint();
                        item.IsBlessed = equipJson["IsBlessed"].GetBool();
                        item.Color = equipJson["Color"].GetUint();
                        
                        player.Equipment[pos] = item;
                    }
                }
                
                // Inventory
                if (json.HasMember("Inventory") && json["Inventory"].IsArray()) {
                    player.Inventory.clear();
                    for (const auto& itemJson : json["Inventory"].GetArray()) {
                        if (!itemJson.IsObject()) continue;
                        
                        EquipmentItem item;
                        item.ItemID = itemJson["ItemID"].GetUint();
                        item.Type = itemJson["Type"].GetUint();
                        item.Plus = itemJson["Plus"].GetUint();
                        item.Durability = itemJson["Durability"].GetUint();
                        item.MaxDurability = itemJson["MaxDurability"].GetUint();
                        item.Socket1 = itemJson["Socket1"].GetUint();
                        item.Socket2 = itemJson["Socket2"].GetUint();
                        item.IsBlessed = itemJson["IsBlessed"].GetBool();
                        item.Color = itemJson["Color"].GetUint();
                        
                        player.Inventory.push_back(item);
                    }
                }
                
                // Skills
                if (json.HasMember("Skills") && json["Skills"].IsArray()) {
                    player.Skills.clear();
                    for (const auto& skillJson : json["Skills"].GetArray()) {
                        if (!skillJson.IsObject()) continue;
                        
                        uint16_t skillId = skillJson["SkillID"].GetUint();
                        uint16_t level = skillJson["Level"].GetUint();
                        player.Skills[skillId] = level;
                    }
                }
                
                // Statistics
                if (json.HasMember("TotalOnlineTime") && json["TotalOnlineTime"].IsUint()) 
                    player.TotalOnlineTime = json["TotalOnlineTime"].GetUint();
                if (json.HasMember("LoginCount") && json["LoginCount"].IsUint()) 
                    player.LoginCount = json["LoginCount"].GetUint();
                
                return true;
            }
            catch (...) {
                return false;
            }
        }
    }

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

        // Try JSON format first (new safe format)
        std::string jsonFilepath = "data/players/" + std::to_string(playerID) + ".json";
        std::ifstream jsonFile(jsonFilepath);
        if (jsonFile.is_open()) {
            std::string jsonContent((std::istreambuf_iterator<char>(jsonFile)),
                                  std::istreambuf_iterator<char>());
            jsonFile.close();
            
            rapidjson::Document doc;
            doc.Parse(jsonContent.c_str());
            
            if (!doc.HasParseError() && doc.IsObject()) {
                PlayerData player;
                if (JsonToPlayerData(doc, player) && ValidatePlayerData(player)) {
                    m_Players[playerID] = player;
                    CH_CORE_INFO("Loaded player {} from JSON file", playerID);
                    return true;
                }
            }
        }
        
        // Fall back to legacy binary format (unsafe - for backward compatibility)
        std::string binaryFilepath = "data/players/" + std::to_string(playerID) + ".dat";
        std::ifstream binaryFile(binaryFilepath, std::ios::binary);
        if (binaryFile.is_open()) {
            PlayerData player;
            binaryFile.read(reinterpret_cast<char*>(&player), sizeof(PlayerData));

            if (binaryFile.good() && ValidatePlayerData(player)) {
                m_Players[playerID] = player;
                binaryFile.close();
                
                // Convert to JSON format and delete binary file
                SavePlayer(playerID); // This will now save as JSON
                std::filesystem::remove(binaryFilepath);
                
                CH_CORE_INFO("Migrated player {} from binary to JSON format", playerID);
                return true;
            }
            binaryFile.close();
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

        // Save as JSON (safe format)
        std::string filepath = "data/players/" + std::to_string(playerID) + ".json";
        
        try {
            // Create JSON document
            rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();
            
            // Convert player data to JSON
            rapidjson::Value playerJson = PlayerDataToJson(it->second, allocator);
            doc.CopyFrom(playerJson, allocator);
            
            // Write to file with pretty formatting
            std::ofstream file(filepath);
            if (!file.is_open()) {
                CH_CORE_ERROR("Failed to open file for saving player {}", playerID);
                return false;
            }
            
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            
            file << buffer.GetString();
            file.close();
            
            if (file.good()) {
                CH_CORE_INFO("Saved player {} to JSON file", playerID);
                return true;
            }
            else {
                CH_CORE_ERROR("Failed to write player {} JSON data", playerID);
                return false;
            }
        }
        catch (const std::exception& e) {
            CH_CORE_ERROR("Exception while saving player {}: {}", playerID, e.what());
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

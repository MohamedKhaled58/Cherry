#pragma once
#include "UIManager.h"
#include "Cherry/Core/Core.h"
namespace Cherry {
    class ConquerUI : public UIElement {
    public:
        ConquerUI();

        // Main game interface
        void ShowCharacterStatus(uint32_t playerID);
        void ShowInventory();
        void ShowEquipment();
        void ShowSkills();
        void ShowMap();

        // Chat system
        void AddChatMessage(const std::string& sender, const std::string& message,
            uint32_t channel);

        // Item tooltips
        void ShowItemTooltip(uint32_t itemID, const glm::vec2& position);

        void OnUpdate(float deltaTime) override;
        void OnRender(UIRenderer& renderer) override;

    private:
        // UI Windows
        REF(UIWindow) m_StatusWindow;
        REF(UIWindow) m_InventoryWindow;
        REF(UIWindow) m_EquipmentWindow;
        REF(UIWindow) m_ChatWindow;
        REF(UIWindow) m_SkillWindow;

        // Game state
        uint32_t m_CurrentPlayerID = 0;
    };
}
// Cherry/src/Cherry/UI/UIManager.h
#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Events/Event.h"
#include "Cherry/Renderer/Texture.h"
#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace Cherry {

    // Forward declarations
    class UIWindow;
    class UIElement;
    class UILayout;
    class UITheme;
    class UIRenderer;

    enum class UIAlignment {
        TopLeft, TopCenter, TopRight,
        MiddleLeft, MiddleCenter, MiddleRight,
        BottomLeft, BottomCenter, BottomRight
    };

    enum class UIAnchor {
        None = 0,
        Left = 1 << 0,
        Right = 1 << 1,
        Top = 1 << 2,
        Bottom = 1 << 3,
        All = Left | Right | Top | Bottom
    };

    struct UIRect {
        float X, Y, Width, Height;

        UIRect() : X(0), Y(0), Width(0), Height(0) {}
        UIRect(float x, float y, float w, float h) : X(x), Y(y), Width(w), Height(h) {}

        bool Contains(float x, float y) const {
            return x >= X && x <= (X + Width) && y >= Y && y <= (Y + Height);
        }

        glm::vec2 GetCenter() const {
            return { X + Width * 0.5f, Y + Height * 0.5f };
        }
    };

    struct UIStyle {
        // Colors
        glm::vec4 BackgroundColor = { 0.2f, 0.2f, 0.2f, 0.9f };
        glm::vec4 BorderColor = { 0.5f, 0.5f, 0.5f, 1.0f };
        glm::vec4 TextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 HoverColor = { 0.3f, 0.3f, 0.3f, 0.9f };
        glm::vec4 ActiveColor = { 0.4f, 0.4f, 0.4f, 0.9f };

        // Sizes
        float BorderWidth = 1.0f;
        float CornerRadius = 0.0f;
        glm::vec4 Padding = { 5.0f, 5.0f, 5.0f, 5.0f }; // left, top, right, bottom
        glm::vec4 Margin = { 0.0f, 0.0f, 0.0f, 0.0f };

        // Text
        float FontSize = 14.0f;
        std::string FontFamily = "default";

        // Textures
        REF(Texture2D) BackgroundTexture;
        REF(Texture2D) BorderTexture;
    };

    struct UIEvent {
        enum Type {
            MouseEnter, MouseLeave, MouseDown, MouseUp, Click,
            KeyDown, KeyUp, TextInput,
            Focus, Blur,
            ValueChanged, WindowClose
        };

        Type EventType;
        glm::vec2 MousePosition;
        int MouseButton = -1;
        int KeyCode = -1;
        std::string Text;
        UIElement* Source = nullptr;
    };

    // Base UI element class
    class UIElement {
    public:
        UIElement(const std::string& id = "");
        virtual ~UIElement() = default;

        // Properties
        void SetID(const std::string& id) { m_ID = id; }
        const std::string& GetID() const { return m_ID; }

        void SetVisible(bool visible) { m_Visible = visible; }
        bool IsVisible() const { return m_Visible; }

        void SetEnabled(bool enabled) { m_Enabled = enabled; }
        bool IsEnabled() const { return m_Enabled; }

        // Transform
        void SetPosition(const glm::vec2& position) { m_Position = position; UpdateTransform(); }
        void SetSize(const glm::vec2& size) { m_Size = size; UpdateTransform(); }
        void SetRect(const UIRect& rect) {
            m_Position = { rect.X, rect.Y };
            m_Size = { rect.Width, rect.Height };
            UpdateTransform();
        }

        const glm::vec2& GetPosition() const { return m_Position; }
        const glm::vec2& GetSize() const { return m_Size; }
        UIRect GetRect() const { return { m_Position.x, m_Position.y, m_Size.x, m_Size.y }; }

        // Anchoring
        void SetAnchor(UIAnchor anchor) { m_Anchor = anchor; }
        UIAnchor GetAnchor() const { return m_Anchor; }

        // Style
        void SetStyle(const UIStyle& style) { m_Style = style; }
        UIStyle& GetStyle() { return m_Style; }
        const UIStyle& GetStyle() const { return m_Style; }

        // Hierarchy
        void AddChild(REF(UIElement) child);
        void RemoveChild(REF(UIElement) child);
        void RemoveChild(const std::string& id);
        REF(UIElement) GetChild(const std::string& id);
        const std::vector<REF(UIElement)>& GetChildren() const { return m_Children; }
        void SetParent(UIElement* parent) { m_Parent = parent; }
        UIElement* GetParent() const { return m_Parent; }

        // Event handling
        void SetEventHandler(UIEvent::Type eventType, std::function<void(const UIEvent&)> handler);
        void TriggerEvent(const UIEvent& event);

        // Virtual methods
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnRender(UIRenderer& renderer) {}
        virtual bool OnEvent(const UIEvent& event) { return false; }
        virtual void OnLayout() {}

        // Hit testing
        virtual bool Contains(const glm::vec2& point) const;

    protected:
        virtual void UpdateTransform();

    protected:
        std::string m_ID;
        bool m_Visible = true;
        bool m_Enabled = true;

        glm::vec2 m_Position = { 0.0f, 0.0f };
        glm::vec2 m_Size = { 100.0f, 100.0f };
        UIAnchor m_Anchor = UIAnchor::None;

        UIStyle m_Style;

        // Hierarchy
        UIElement* m_Parent = nullptr;
        std::vector<REF(UIElement)> m_Children;

        // Event handlers
        std::unordered_map<UIEvent::Type, std::function<void(const UIEvent&)>> m_EventHandlers;
    };

    // UI Window class
    class UIWindow : public UIElement {
    public:
        UIWindow(const std::string& title, const UIRect& rect);

        void SetTitle(const std::string& title) { m_Title = title; }
        const std::string& GetTitle() const { return m_Title; }

        void SetResizable(bool resizable) { m_Resizable = resizable; }
        bool IsResizable() const { return m_Resizable; }

        void SetMovable(bool movable) { m_Movable = movable; }
        bool IsMovable() const { return m_Movable; }

        void SetModal(bool modal) { m_Modal = modal; }
        bool IsModal() const { return m_Modal; }

        void Close();

        // Window-specific rendering
        void OnRender(UIRenderer& renderer) override;
        bool OnEvent(const UIEvent& event) override;

    private:
        std::string m_Title;
        bool m_Resizable = true;
        bool m_Movable = true;
        bool m_Modal = false;
        bool m_IsDragging = false;
        bool m_IsResizing = false;
        glm::vec2 m_DragOffset;
    };

    // UI Button class
    class UIButton : public UIElement {
    public:
        UIButton(const std::string& text, const glm::vec2& position, const glm::vec2& size);

        void SetText(const std::string& text) { m_Text = text; }
        const std::string& GetText() const { return m_Text; }

        void SetIcon(REF(Texture2D) icon) { m_Icon = icon; }
        REF(Texture2D) GetIcon() const { return m_Icon; }

        void OnRender(UIRenderer& renderer) override;
        bool OnEvent(const UIEvent& event) override;

    private:
        std::string m_Text;
        REF(Texture2D) m_Icon;
        bool m_IsHovered = false;
        bool m_IsPressed = false;
    };

    // UI Label class
    class UILabel : public UIElement {
    public:
        UILabel(const std::string& text, const glm::vec2& position);

        void SetText(const std::string& text) { m_Text = text; }
        const std::string& GetText() const { return m_Text; }

        void SetAlignment(UIAlignment alignment) { m_Alignment = alignment; }
        UIAlignment GetAlignment() const { return m_Alignment; }

        void OnRender(UIRenderer& renderer) override;

    private:
        std::string m_Text;
        UIAlignment m_Alignment = UIAlignment::TopLeft;
    };

    // UI Image class
    class UIImage : public UIElement {
    public:
        UIImage(REF(Texture2D) texture, const glm::vec2& position, const glm::vec2& size);

        void SetTexture(REF(Texture2D) texture) { m_Texture = texture; }
        REF(Texture2D) GetTexture() const { return m_Texture; }

        void SetTint(const glm::vec4& tint) { m_Tint = tint; }
        const glm::vec4& GetTint() const { return m_Tint; }

        void OnRender(UIRenderer& renderer) override;

    private:
        REF(Texture2D) m_Texture;
        glm::vec4 m_Tint = { 1.0f, 1.0f, 1.0f, 1.0f };
    };

    // UI Panel class (container)
    class UIPanel : public UIElement {
    public:
        UIPanel(const UIRect& rect);

        void SetScrollable(bool scrollable) { m_Scrollable = scrollable; }
        bool IsScrollable() const { return m_Scrollable; }

        void SetScrollOffset(const glm::vec2& offset) { m_ScrollOffset = offset; }
        const glm::vec2& GetScrollOffset() const { return m_ScrollOffset; }

        void OnRender(UIRenderer& renderer) override;
        bool OnEvent(const UIEvent& event) override;

    private:
        bool m_Scrollable = false;
        glm::vec2 m_ScrollOffset = { 0.0f, 0.0f };
        glm::vec2 m_ContentSize = { 0.0f, 0.0f };
    };

    // UI Layout system
    class UILayout {
    public:
        virtual ~UILayout() = default;
        virtual void ApplyLayout(UIElement* container) = 0;
    };

    class UIHorizontalLayout : public UILayout {
    public:
        UIHorizontalLayout(float spacing = 5.0f) : m_Spacing(spacing) {}
        void ApplyLayout(UIElement* container) override;

    private:
        float m_Spacing;
    };

    class UIVerticalLayout : public UILayout {
    public:
        UIVerticalLayout(float spacing = 5.0f) : m_Spacing(spacing) {}
        void ApplyLayout(UIElement* container) override;

    private:
        float m_Spacing;
    };

    class UIGridLayout : public UILayout {
    public:
        UIGridLayout(int columns, int rows, float spacing = 5.0f)
            : m_Columns(columns), m_Rows(rows), m_Spacing(spacing) {
        }
        void ApplyLayout(UIElement* container) override;

    private:
        int m_Columns, m_Rows;
        float m_Spacing;
    };

    // MMO-specific UI elements
    class UIHealthBar : public UIElement {
    public:
        UIHealthBar(const glm::vec2& position, const glm::vec2& size);

        void SetHealth(float current, float max) {
            m_CurrentHealth = current;
            m_MaxHealth = max;
            m_HealthPercentage = max > 0 ? current / max : 0.0f;
        }

        void SetColors(const glm::vec4& healthColor, const glm::vec4& bgColor) {
            m_HealthColor = healthColor;
            m_BackgroundColor = bgColor;
        }

        void OnRender(UIRenderer& renderer) override;

    private:
        float m_CurrentHealth = 100.0f;
        float m_MaxHealth = 100.0f;
        float m_HealthPercentage = 1.0f;
        glm::vec4 m_HealthColor = { 0.0f, 1.0f, 0.0f, 1.0f };
        glm::vec4 m_BackgroundColor = { 0.2f, 0.2f, 0.2f, 0.8f };
    };

    class UIMinimap : public UIElement {
    public:
        UIMinimap(const glm::vec2& position, const glm::vec2& size);

        void SetWorldBounds(const glm::vec2& worldMin, const glm::vec2& worldMax) {
            m_WorldMin = worldMin;
            m_WorldMax = worldMax;
        }

        void SetPlayerPosition(const glm::vec2& position) { m_PlayerPosition = position; }
        void AddMarker(const std::string& id, const glm::vec2& worldPos, const glm::vec4& color);
        void RemoveMarker(const std::string& id);

        void OnRender(UIRenderer& renderer) override;

    private:
        glm::vec2 m_WorldMin, m_WorldMax;
        glm::vec2 m_PlayerPosition;

        struct MinimapMarker {
            glm::vec2 WorldPosition;
            glm::vec4 Color;
        };
        std::unordered_map<std::string, MinimapMarker> m_Markers;

        glm::vec2 WorldToMinimap(const glm::vec2& worldPos) const;
    };

    class UIInventorySlot : public UIElement {
    public:
        UIInventorySlot(int slotIndex, const glm::vec2& position, const glm::vec2& size);

        void SetItem(uint32_t itemID, REF(Texture2D) icon, uint32_t quantity = 1);
        void ClearItem();
        bool HasItem() const { return m_ItemID != 0; }
        uint32_t GetItemID() const { return m_ItemID; }
        uint32_t GetQuantity() const { return m_Quantity; }

        void OnRender(UIRenderer& renderer) override;
        bool OnEvent(const UIEvent& event) override;

    private:
        int m_SlotIndex;
        uint32_t m_ItemID = 0;
        uint32_t m_Quantity = 0;
        REF(Texture2D) m_ItemIcon;
        bool m_IsHighlighted = false;
    };

    class UIChatWindow : public UIPanel {
    public:
        UIChatWindow(const UIRect& rect);

        void AddMessage(const std::string& sender, const std::string& message, const glm::vec4& color);
        void ClearMessages();
        void SetMaxMessages(int maxMessages) { m_MaxMessages = maxMessages; }

        void OnRender(UIRenderer& renderer) override;

    private:
        struct ChatMessage {
            std::string Sender;
            std::string Message;
            glm::vec4 Color;
            float Timestamp;
        };

        std::vector<ChatMessage> m_Messages;
        int m_MaxMessages = 100;
        float m_ScrollPosition = 0.0f;
    };

    // Main UI Manager
    class UIManager {
    public:
        static UIManager& Get() {
            static UIManager instance;
            return instance;
        }

        void Initialize();
        void Shutdown();

        // Window management
        REF(UIWindow) CreateWindow(const std::string& id, const std::string& title, const UIRect& rect);
        void DestroyWindow(const std::string& id);
        REF(UIWindow) GetWindow(const std::string& id);
        void ShowWindow(const std::string& id);
        void HideWindow(const std::string& id);

        // Global UI elements
        void AddElement(REF(UIElement) element);
        void RemoveElement(const std::string& id);
        REF(UIElement) GetElement(const std::string& id);

        // Theme management
        void LoadTheme(const std::string& themePath);
        void SetTheme(REF(UITheme) theme);
        REF(UITheme) GetCurrentTheme() const { return m_CurrentTheme; }

        // Update and rendering
        void OnUpdate(float deltaTime);
        void OnRender();
        void OnEvent(Event& e);

        // Input handling
        void SetFocusedElement(REF(UIElement) element) { m_FocusedElement = element; }
        REF(UIElement) GetFocusedElement() const { return m_FocusedElement; }

        // Utility functions
        REF(UIElement) GetElementAt(const glm::vec2& position);
        void BringToFront(REF(UIElement) element);
        void SendToBack(REF(UIElement) element);

        // Screen resolution handling
        void OnScreenResize(uint32_t width, uint32_t height);
        glm::vec2 GetScreenSize() const { return m_ScreenSize; }

    private:
        UIManager() = default;
        ~UIManager() = default;

        void UpdateElementPositions();
        void HandleMouseEvent(const glm::vec2& mousePos, int button, bool pressed);
        void HandleKeyEvent(int keyCode, bool pressed);

    private:
        std::unordered_map<std::string, REF(UIWindow)> m_Windows;
        std::vector<REF(UIElement)> m_GlobalElements;
        std::vector<REF(UIElement)> m_RenderOrder;

        REF(UIElement) m_FocusedElement;
        REF(UIElement) m_HoveredElement;

        REF(UITheme) m_CurrentTheme;
        REF(UIRenderer) m_Renderer;

        glm::vec2 m_ScreenSize = { 1280.0f, 720.0f };
        glm::vec2 m_LastMousePosition = { 0.0f, 0.0f };
    };

    // UI Theme system
    class UITheme {
    public:
        UITheme(const std::string& name) : m_Name(name) {}

        void SetElementStyle(const std::string& elementType, const UIStyle& style);
        UIStyle GetElementStyle(const std::string& elementType) const;
        bool HasElementStyle(const std::string& elementType) const;

        void LoadFromFile(const std::string& filePath);
        void SaveToFile(const std::string& filePath) const;

        const std::string& GetName() const { return m_Name; }

    private:
        std::string m_Name;
        std::unordered_map<std::string, UIStyle> m_ElementStyles;
    };

} // namespace Cherry
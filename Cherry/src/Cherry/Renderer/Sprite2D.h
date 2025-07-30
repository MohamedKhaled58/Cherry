#pragma once
#include <vector>
#include <Cherry/Core/Core.h>
#include <string>

namespace Cherry {
    class Sprite2D {
    public:
        // Advanced sprite features
        void SetAnimation(const std::string& animationName);
        void SetFrame(uint32_t frame);
        void Play(const std::string& animation, bool loop = true);
        void Stop();

        // Blending modes for effects
        enum class BlendMode {
            Normal, Additive, Multiply, Screen, Overlay
        };
        void SetBlendMode(BlendMode mode);

        // Multi-texture support for complex sprites
        void SetTexture(uint32_t slot, REF(Texture2D) texture);

        // Transform animations
        void SetTransform(const Transform2D& transform);

    private:
        AnimationController m_AnimController;
        std::vector<REF(Texture2D)> m_Textures;
        BlendMode m_BlendMode = BlendMode::Normal;
        Transform2D m_Transform;
    };
}

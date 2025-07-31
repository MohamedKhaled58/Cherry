#pragma once
#include <vector>
#include <unordered_map>
#include <string>
namespace Cherry {
    struct KeyFrame {
        int Frame;
        std::vector<float> Params;
    };

    struct AnimationKey {
        std::vector<KeyFrame> AlphaKeys;
        std::vector<KeyFrame> DrawKeys;
        std::vector<KeyFrame> TextureKeys;

        bool ProcessAlpha(uint32_t frame, uint32_t totalFrames, float& result);
        bool ProcessDraw(uint32_t frame, bool& result);
        bool ProcessTexture(uint32_t frame, int& textureIndex);
    };

    class AnimationController {
    public:
        void LoadAnimation(const std::string& filename);
        void PlayAnimation(const std::string& name, bool loop = true);
        void Update(float deltaTime);

    private:
        std::unordered_map<std::string, AnimationKey> m_Animations;
        std::string m_CurrentAnimation;
        float m_CurrentTime = 0.0f;
        bool m_IsPlaying = false;
        bool m_IsLooping = false;
    };
}
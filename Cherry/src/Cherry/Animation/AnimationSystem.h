#pragma once
#include <string>
#include <memory>
#include <Cherry/Core/TimeStep.h>
#include <unordered_map>
#include <Cherry/Scene/Entity.h>
#include <Cherry/Scene/Components.h>

namespace Cherry {
    class AnimationSystem {
    public:
        // Skeletal animation for characters
        AnimationHandle LoadAnimation(const std::string& path);
        void PlayAnimation(Entity entity, AnimationHandle animation);

        // State machine for complex animations
        void SetAnimationStateMachine(Entity entity,
            std::unique_ptr<AnimationStateMachine> fsm);

        // Blending between animations
        void BlendToAnimation(Entity entity, AnimationHandle target, float blendTime);

        // Update all animations
        void Update(TimeStep ts);

    private:
        std::unordered_map<Entity, AnimationComponent> m_AnimatedEntities;
        std::vector<Animation> m_Animations;
    };
}
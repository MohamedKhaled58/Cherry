#include "CHpch.h"
#include "AnimationSystem.h"

namespace Cherry {

    // ===== EnhancedAnimationKey Implementation =====
    
    void EnhancedAnimationKey::Clear() {
        m_Tracks.clear();
        m_Duration = 0;
    }
    
    bool EnhancedAnimationKey::ProcessAlpha(uint32_t frame, uint32_t totalFrames, float& result) const {
        // Stub implementation - would process alpha track
        result = 1.0f;
        return true;
    }
    
    bool EnhancedAnimationKey::ProcessDraw(uint32_t frame, bool& result) const {
        // Stub implementation - would process draw/visibility track
        result = true;
        return true;
    }
    
    bool EnhancedAnimationKey::ProcessTexture(uint32_t frame, int& result) const {
        // Stub implementation - would process texture switching track
        result = 0;
        return true;
    }
    
    void EnhancedAnimationKey::EvaluateAllTracks(uint32_t frame, uint32_t totalFrames, std::unordered_map<std::string, std::any>& results) const {
        // Stub implementation - would evaluate all animation tracks
    }
    
    bool EnhancedAnimationKey::SaveToFile(const std::string& filepath) const {
        // Stub implementation - would save animation to file
        return true;
    }
    
    bool EnhancedAnimationKey::LoadFromFile(const std::string& filepath) {
        // Stub implementation - would load animation from file
        return true;
    }

    // ===== AnimationController Implementation =====
    
    void AnimationController::LoadAnimation(const std::string& name, const std::string& filepath) {
        // Stub implementation - would load from file
        // For now, create a basic animation
        auto animation = std::make_shared<EnhancedAnimationKey>(name);
        m_Animations[name] = animation;
    }
    
    void AnimationController::AddAnimation(const std::string& name, REF(EnhancedAnimationKey) animation) {
        m_Animations[name] = animation;
    }
    
    void AnimationController::RemoveAnimation(const std::string& name) {
        m_Animations.erase(name);
    }
    
    void AnimationController::PlayAnimation(const std::string& name, bool loop) {
        auto it = m_Animations.find(name);
        if (it != m_Animations.end()) {
            m_CurrentAnimationName = name;
            m_CurrentTime = 0.0f;
            m_IsPlaying = true;
            m_IsLooping = loop;
        }
    }
    
    void AnimationController::StopAnimation() {
        m_IsPlaying = false;
        m_CurrentTime = 0.0f;
    }
    
    void AnimationController::PauseAnimation() {
        m_IsPlaying = false;
    }
    
    void AnimationController::ResumeAnimation() {
        if (!m_CurrentAnimationName.empty()) {
            m_IsPlaying = true;
        }
    }
    
    void AnimationController::Update(float deltaTime) {
        if (!m_IsPlaying || m_CurrentAnimationName.empty()) {
            return;
        }
        
        m_CurrentTime += deltaTime * m_PlaybackSpeed;
        
        auto it = m_Animations.find(m_CurrentAnimationName);
        if (it != m_Animations.end()) {
            auto& animation = it->second;
            float duration = (float)animation->GetDuration() / 30.0f; // Assuming 30 FPS
            
            if (m_CurrentTime >= duration) {
                if (m_IsLooping) {
                    m_CurrentTime = 0.0f;
                    if (m_OnAnimationLooped) {
                        m_OnAnimationLooped(m_CurrentAnimationName);
                    }
                } else {
                    m_IsPlaying = false;
                    if (m_OnAnimationFinished) {
                        m_OnAnimationFinished(m_CurrentAnimationName);
                    }
                }
            }
            
            // Apply animation to entity (stub implementation)
            ApplyAnimationToEntity(m_CurrentAnimationName, m_CurrentTime);
        }
        
        // Update blending
        if (m_IsBlending) {
            UpdateBlending(deltaTime);
        }
    }
    
    void AnimationController::BlendToAnimation(const std::string& targetAnimation, float blendTime) {
        // Stub implementation for animation blending
        m_IsBlending = true;
        m_BlendDuration = blendTime;
        m_BlendElapsed = 0.0f;
    }
    
    void AnimationController::SetBlendMode(const std::string& animation, float weight) {
        // Stub implementation for blend modes
    }
    
    void AnimationController::SetStateMachine(std::unique_ptr<AnimationStateMachine> stateMachine) {
        m_StateMachine = std::move(stateMachine);
    }
    
    void AnimationController::ApplyAnimationToEntity(const std::string& animationName, float time, float weight) {
        // Stub implementation - would apply animation data to entity components
        // This would typically modify transform, material, or other components
    }
    
    void AnimationController::UpdateBlending(float deltaTime) {
        if (!m_IsBlending) return;
        
        m_BlendElapsed += deltaTime;
        if (m_BlendElapsed >= m_BlendDuration) {
            m_IsBlending = false;
            m_BlendElapsed = 0.0f;
        }
    }

    // ===== EnhancedAnimationSystem Implementation =====
    
    void EnhancedAnimationSystem::Initialize() {
        m_GlobalTimeScale = 1.0f;
        m_GlobalPaused = false;
        m_Stats = {};
        m_LastUpdateTime = std::chrono::steady_clock::now();
    }
    
    void EnhancedAnimationSystem::Shutdown() {
        m_LoadedAnimations.clear();
        m_Controllers.clear();
        m_Presets.clear();
    }
    
    REF(EnhancedAnimationKey) EnhancedAnimationSystem::LoadAnimation(const std::string& name, const std::string& filepath) {
        // Check if already loaded
        auto it = m_LoadedAnimations.find(name);
        if (it != m_LoadedAnimations.end()) {
            return it->second;
        }
        
        // Create new animation (stub implementation)
        auto animation = std::make_shared<EnhancedAnimationKey>(name);
        m_LoadedAnimations[name] = animation;
        return animation;
    }
    
    REF(EnhancedAnimationKey) EnhancedAnimationSystem::CreateAnimation(const std::string& name, uint32_t duration) {
        auto animation = std::make_shared<EnhancedAnimationKey>(name);
        animation->SetDuration(duration);
        m_LoadedAnimations[name] = animation;
        return animation;
    }
    
    void EnhancedAnimationSystem::UnloadAnimation(const std::string& name) {
        m_LoadedAnimations.erase(name);
    }
    
    AnimationController* EnhancedAnimationSystem::CreateController(Entity entity) {
        auto controller = std::make_unique<AnimationController>(entity);
        auto* controllerPtr = controller.get();
        m_Controllers[entity] = std::move(controller);
        return controllerPtr;
    }
    
    void EnhancedAnimationSystem::DestroyController(Entity entity) {
        m_Controllers.erase(entity);
    }
    
    AnimationController* EnhancedAnimationSystem::GetController(Entity entity) {
        auto it = m_Controllers.find(entity);
        return (it != m_Controllers.end()) ? it->second.get() : nullptr;
    }
    
    void EnhancedAnimationSystem::Update(TimeStep deltaTime) {
        if (m_GlobalPaused) return;
        
        float scaledDeltaTime = deltaTime.GetSeconds() * m_GlobalTimeScale;
        
        // Update all controllers
        for (auto& [entity, controller] : m_Controllers) {
            if (controller) {
                controller->Update(scaledDeltaTime);
            }
        }
        
        // Update statistics
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_LastUpdateTime);
        
        std::lock_guard<std::mutex> lock(m_StatsMutex);
        m_Stats.LoadedAnimations = m_LoadedAnimations.size();
        m_Stats.ActiveControllers = m_Controllers.size();
        m_Stats.AverageUpdateTime = elapsed.count() / 1000.0f; // Convert to milliseconds
        
        m_LastUpdateTime = currentTime;
    }

    void EnhancedAnimationSystem::PauseAll() {
        m_GlobalPaused = true;
    }
    
    void EnhancedAnimationSystem::ResumeAll() {
        m_GlobalPaused = false;
    }
    
    void EnhancedAnimationSystem::StopAll() {
        for (auto& [entity, controller] : m_Controllers) {
            if (controller) {
                controller->StopAnimation();
            }
        }
    }
    
    EnhancedAnimationSystem::AnimationStats EnhancedAnimationSystem::GetStats() const {
        std::lock_guard<std::mutex> lock(m_StatsMutex);
        return m_Stats;
    }
    
    void EnhancedAnimationSystem::RegisterAnimationPreset(const std::string& name, std::function<REF(EnhancedAnimationKey)()> factory) {
        m_Presets[name] = factory;
    }
    
    REF(EnhancedAnimationKey) EnhancedAnimationSystem::CreateFromPreset(const std::string& presetName) {
        auto it = m_Presets.find(presetName);
        if (it != m_Presets.end()) {
            return it->second();
        }
        return nullptr;
    }

} // namespace Cherry
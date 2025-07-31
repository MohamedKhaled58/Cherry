// Cherry/src/Cherry/Animation/EnhancedAnimationSystem.h
#pragma once
#include "Cherry/Core/Core.h"
#include "Cherry/Core/TimeStep.h"
#include "Cherry/Animation/KeyFrameSystem.h"
#include <functional>
#include <variant>
#include <glm/glm.hpp>
#include <Cherry/Scene/Entity.h>

namespace Cherry {

    // Enhanced keyframe system with C3Engine compatibility
    template<typename T>
    struct KeyFrame {
        uint32_t Frame;
        T Value;

        // Interpolation type
        enum class InterpolationType {
            None,      // No interpolation (step)
            Linear,    // Linear interpolation
            Smooth,    // Smooth/ease interpolation
            Bezier     // Bezier curve interpolation
        } Interpolation = InterpolationType::Linear;

        // Bezier control points (for Bezier interpolation)
        T BezierControl1 = T{};
        T BezierControl2 = T{};

        KeyFrame() = default;
        KeyFrame(uint32_t frame, const T& value, InterpolationType interp = InterpolationType::Linear)
            : Frame(frame), Value(value), Interpolation(interp) {
        }
    };

    // Specialized keyframe types from C3Engine
    using AlphaKeyFrame = KeyFrame<float>;      // Alpha animation
    using DrawKeyFrame = KeyFrame<bool>;        // Draw/visibility toggle
    using TextureKeyFrame = KeyFrame<int>;      // Texture switching
    using PositionKeyFrame = KeyFrame<glm::vec3>; // Position animation
    using RotationKeyFrame = KeyFrame<glm::vec3>; // Rotation animation
    using ScaleKeyFrame = KeyFrame<glm::vec3>;     // Scale animation
    using ColorKeyFrame = KeyFrame<glm::vec4>;     // Color animation

    // Animation track for specific property
    template<typename T>
    class AnimationTrack {
    public:
        using ValueType = T;
        using KeyFrameType = KeyFrame<T>;
        using InterpolationFunc = std::function<T(const T&, const T&, float)>;

        AnimationTrack() = default;
        explicit AnimationTrack(const std::string& name) : m_Name(name) {}

        // Keyframe management
        void AddKeyFrame(uint32_t frame, const T& value, typename KeyFrameType::InterpolationType interp = KeyFrameType::InterpolationType::Linear);
        void AddKeyFrame(const KeyFrameType& keyframe);
        void RemoveKeyFrame(uint32_t frame);
        void ClearKeyFrames() { m_KeyFrames.clear(); }

        // Value evaluation
        bool Evaluate(uint32_t frame, uint32_t totalFrames, T& result) const;
        T EvaluateAt(float normalizedTime) const; // 0.0 to 1.0

        // Properties
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        size_t GetKeyFrameCount() const { return m_KeyFrames.size(); }
        const std::vector<KeyFrameType>& GetKeyFrames() const { return m_KeyFrames; }

        // Custom interpolation
        void SetInterpolationFunction(InterpolationFunc func) { m_CustomInterpolation = func; }

        // Optimization
        void OptimizeKeyFrames(); // Remove redundant keyframes

    private:
        std::string m_Name;
        std::vector<KeyFrameType> m_KeyFrames;
        InterpolationFunc m_CustomInterpolation;

        T InterpolateValues(const T& from, const T& to, float t, typename KeyFrameType::InterpolationType type) const;
        T BezierInterpolate(const T& p0, const T& p1, const T& p2, const T& p3, float t) const;

        // Find keyframes for interpolation
        std::pair<int, int> FindKeyFrameIndices(uint32_t frame) const;
    };

    // Enhanced animation key (C3Engine compatible)
    class EnhancedAnimationKey {
    public:
        EnhancedAnimationKey() = default;
        explicit EnhancedAnimationKey(const std::string& name) : m_Name(name) {}

        // C3Engine compatibility layer
        void Clear();
        bool ProcessAlpha(uint32_t frame, uint32_t totalFrames, float& result) const;
        bool ProcessDraw(uint32_t frame, bool& result) const;
        bool ProcessTexture(uint32_t frame, int& result) const;

        // Enhanced track system
        template<typename T>
        void AddTrack(const std::string& name, const AnimationTrack<T>& track) {
            m_Tracks[name] = std::make_unique<AnimationTrack<T>>(track);
        }

        template<typename T>
        AnimationTrack<T>* GetTrack(const std::string& name) {
            auto it = m_Tracks.find(name);
            if (it != m_Tracks.end()) {
                return dynamic_cast<AnimationTrack<T>*>(it->second.get());
            }
            return nullptr;
        }

        template<typename T>
        const AnimationTrack<T>* GetTrack(const std::string& name) const {
            auto it = m_Tracks.find(name);
            if (it != m_Tracks.end()) {
                return dynamic_cast<const AnimationTrack<T>*>(it->second.get());
            }
            return nullptr;
        }

        // Evaluation for all tracks
        void EvaluateAllTracks(uint32_t frame, uint32_t totalFrames, std::unordered_map<std::string, std::any>& results) const;

        // Properties
        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        uint32_t GetDuration() const { return m_Duration; }
        void SetDuration(uint32_t duration) { m_Duration = duration; }

        // Serialization
        bool SaveToFile(const std::string& filepath) const;
        bool LoadFromFile(const std::string& filepath);

    private:
        std::string m_Name;
        uint32_t m_Duration = 0;

        // Type-erased track storage
        class IAnimationTrackBase {
        public:
            virtual ~IAnimationTrackBase() = default;
            virtual std::unique_ptr<IAnimationTrackBase> Clone() const = 0;
        };

        template<typename T>
        class AnimationTrackWrapper : public IAnimationTrackBase {
        public:
            AnimationTrack<T> Track;

            AnimationTrackWrapper(const AnimationTrack<T>& track) : Track(track) {}

            std::unique_ptr<IAnimationTrackBase> Clone() const override {
                return std::make_unique<AnimationTrackWrapper<T>>(Track);
            }
        };

        std::unordered_map<std::string, std::unique_ptr<IAnimationTrackBase>> m_Tracks;

        // Legacy C3Engine tracks for compatibility
        AnimationTrack<float> m_AlphaTrack;
        AnimationTrack<bool> m_DrawTrack;
        AnimationTrack<int> m_TextureTrack;
    };

    // Animation state machine for complex animations
    class AnimationStateMachine {
    public:
        struct AnimationState {
            std::string Name;
            REF(EnhancedAnimationKey) Animation;
            bool IsLooping = false;
            float PlaybackSpeed = 1.0f;

            // State callbacks
            std::function<void()> OnEnter;
            std::function<void()> OnExit;
            std::function<void(float)> OnUpdate;
        };

        struct Transition {
            std::string FromState;
            std::string ToState;
            std::function<bool()> Condition;
            float BlendTime = 0.0f; // Time to blend between animations
        };

        AnimationStateMachine() = default;

        // State management
        void AddState(const std::string& name, REF(EnhancedAnimationKey) animation, bool isLooping = false);
        void AddState(const AnimationState& state);
        void RemoveState(const std::string& name);
        void SetInitialState(const std::string& name);

        // Transition management
        void AddTransition(const std::string& from, const std::string& to, std::function<bool()> condition, float blendTime = 0.0f);
        void AddTransition(const Transition& transition);

        // Playback control
        void Play();
        void Stop();
        void Pause();
        void Resume();
        bool IsPlaying() const { return m_IsPlaying; }

        void SetCurrentState(const std::string& name);
        const std::string& GetCurrentState() const { return m_CurrentStateName; }

        // Update
        void Update(float deltaTime);

        // Events
        void SetOnStateChanged(std::function<void(const std::string&, const std::string&)> callback) {
            m_OnStateChanged = callback;
        }

    private:
        std::unordered_map<std::string, AnimationState> m_States;
        std::vector<Transition> m_Transitions;

        std::string m_CurrentStateName;
        std::string m_PreviousStateName;

        float m_CurrentTime = 0.0f;
        float m_BlendTime = 0.0f;
        float m_BlendDuration = 0.0f;
        bool m_IsBlending = false;
        bool m_IsPlaying = false;
        bool m_IsPaused = false;

        std::function<void(const std::string&, const std::string&)> m_OnStateChanged;

        void ChangeState(const std::string& newState, float blendTime = 0.0f);
        void UpdateBlending(float deltaTime);
        bool CheckTransitions();
    };

    // Animation controller for entities
    class AnimationController {
    public:
        AnimationController() = default;
        explicit AnimationController(Entity entity) : m_Entity(entity) {}

        // Animation loading and management
        void LoadAnimation(const std::string& name, const std::string& filepath);
        void AddAnimation(const std::string& name, REF(EnhancedAnimationKey) animation);
        void RemoveAnimation(const std::string& name);

        // Playback control
        void PlayAnimation(const std::string& name, bool loop = true);
        void StopAnimation();
        void PauseAnimation();
        void ResumeAnimation();

        // Blending
        void BlendToAnimation(const std::string& targetAnimation, float blendTime);
        void SetBlendMode(const std::string& animation, float weight);

        // State machine
        void SetStateMachine(std::unique_ptr<AnimationStateMachine> stateMachine);
        AnimationStateMachine* GetStateMachine() const { return m_StateMachine.get(); }

        // Update
        void Update(float deltaTime);

        // Properties
        void SetPlaybackSpeed(float speed) { m_PlaybackSpeed = speed; }
        float GetPlaybackSpeed() const { return m_PlaybackSpeed; }

        void SetCurrentTime(float time) { m_CurrentTime = time; }
        float GetCurrentTime() const { return m_CurrentTime; }

        const std::string& GetCurrentAnimation() const { return m_CurrentAnimationName; }
        bool IsPlaying() const { return m_IsPlaying; }
        bool IsLooping() const { return m_IsLooping; }

        // Events
        void SetOnAnimationFinished(std::function<void(const std::string&)> callback) {
            m_OnAnimationFinished = callback;
        }

        void SetOnAnimationLooped(std::function<void(const std::string&)> callback) {
            m_OnAnimationLooped = callback;
        }

    private:
        Entity m_Entity;
        std::unordered_map<std::string, REF(EnhancedAnimationKey)> m_Animations;
        std::unique_ptr<AnimationStateMachine> m_StateMachine;

        std::string m_CurrentAnimationName;
        float m_CurrentTime = 0.0f;
        float m_PlaybackSpeed = 1.0f;
        bool m_IsPlaying = false;
        bool m_IsLooping = false;

        // Blending
        struct BlendState {
            std::string AnimationName;
            float Weight = 0.0f;
            float Time = 0.0f;
        };
        std::vector<BlendState> m_BlendStates;
        bool m_IsBlending = false;
        float m_BlendDuration = 0.0f;
        float m_BlendElapsed = 0.0f;

        // Events
        std::function<void(const std::string&)> m_OnAnimationFinished;
        std::function<void(const std::string&)> m_OnAnimationLooped;

        void ApplyAnimationToEntity(const std::string& animationName, float time, float weight = 1.0f);
        void UpdateBlending(float deltaTime);
    };

    // Enhanced animation system (main system)
    class EnhancedAnimationSystem {
    public:
        static EnhancedAnimationSystem& Get() {
            static EnhancedAnimationSystem instance;
            return instance;
        }

        void Initialize();
        void Shutdown();

        // Animation resource management
        REF(EnhancedAnimationKey) LoadAnimation(const std::string& name, const std::string& filepath);
        REF(EnhancedAnimationKey) CreateAnimation(const std::string& name, uint32_t duration);
        void UnloadAnimation(const std::string& name);

        // Controller management
        AnimationController* CreateController(Entity entity);
        void DestroyController(Entity entity);
        AnimationController* GetController(Entity entity);

        // System update
        void Update(TimeStep deltaTime);

        // Global animation settings
        void SetGlobalTimeScale(float timeScale) { m_GlobalTimeScale = timeScale; }
        float GetGlobalTimeScale() const { return m_GlobalTimeScale; }

        void PauseAll();
        void ResumeAll();
        void StopAll();

        // Statistics
        struct AnimationStats {
            size_t LoadedAnimations = 0;
            size_t ActiveControllers = 0;
            size_t PlayingAnimations = 0;
            float AverageUpdateTime = 0.0f;
        };

        AnimationStats GetStats() const;

        // Presets and templates
        void RegisterAnimationPreset(const std::string& name, std::function<REF(EnhancedAnimationKey)()> factory);
        REF(EnhancedAnimationKey) CreateFromPreset(const std::string& presetName);

    private:
        EnhancedAnimationSystem() = default;
        ~EnhancedAnimationSystem() = default;

        std::unordered_map<std::string, REF(EnhancedAnimationKey)> m_LoadedAnimations;
        std::unordered_map<Entity, std::unique_ptr<AnimationController>> m_Controllers;
        std::unordered_map<std::string, std::function<REF(EnhancedAnimationKey)()>> m_Presets;

        float m_GlobalTimeScale = 1.0f;
        bool m_GlobalPaused = false;

        // Performance tracking
        mutable std::mutex m_StatsMutex;
        AnimationStats m_Stats;
        std::chrono::steady_clock::time_point m_LastUpdateTime;
    };

    // Utility functions and interpolation helpers
    namespace AnimationUtils {
        // Interpolation functions
        template<typename T>
        T LinearInterpolate(const T& a, const T& b, float t) {
            return a + (b - a) * t;
        }

        template<typename T>
        T SmoothInterpolate(const T& a, const T& b, float t) {
            t = t * t * (3.0f - 2.0f * t); // Smoothstep
            return LinearInterpolate(a, b, t);
        }

        template<typename T>
        T BezierInterpolate(const T& p0, const T& p1, const T& p2, const T& p3, float t) {
            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float uuu = uu * u;
            float ttt = tt * t;

            return uuu * p0 + 3.0f * uu * t * p1 + 3.0f * u * tt * p2 + ttt * p3;
        }

        // Easing functions
        float EaseInQuad(float t);
        float EaseOutQuad(float t);
        float EaseInOutQuad(float t);
        float EaseInCubic(float t);
        float EaseOutCubic(float t);
        float EaseInOutCubic(float t);

        // Animation curve evaluation
        float EvaluateCurve(float t, const std::vector<glm::vec2>& controlPoints);

        // Time conversion utilities
        uint32_t SecondsToFrames(float seconds, float frameRate = 30.0f);
        float FramesToSeconds(uint32_t frames, float frameRate = 30.0f);

        // Animation file I/O
        bool SaveAnimationToFile(const EnhancedAnimationKey& animation, const std::string& filepath);
        REF(EnhancedAnimationKey) LoadAnimationFromFile(const std::string& filepath);

        // C3Engine compatibility
        REF(EnhancedAnimationKey) ConvertFromC3Key(const std::string& filepath);
    }

    // Template implementations
    template<typename T>
    void AnimationTrack<T>::AddKeyFrame(uint32_t frame, const T& value, typename KeyFrameType::InterpolationType interp) {
        KeyFrameType keyframe(frame, value, interp);
        AddKeyFrame(keyframe);
    }

    template<typename T>
    void AnimationTrack<T>::AddKeyFrame(const KeyFrameType& keyframe) {
        // Insert in sorted order
        auto it = std::lower_bound(m_KeyFrames.begin(), m_KeyFrames.end(), keyframe,
            [](const KeyFrameType& a, const KeyFrameType& b) {
                return a.Frame < b.Frame;
            });
        m_KeyFrames.insert(it, keyframe);
    }

    template<typename T>
    bool AnimationTrack<T>::Evaluate(uint32_t frame, uint32_t totalFrames, T& result) const {
        if (m_KeyFrames.empty()) return false;

        auto indices = FindKeyFrameIndices(frame);
        int startIndex = indices.first;
        int endIndex = indices.second;

        if (startIndex == -1 && endIndex > -1) {
            result = m_KeyFrames[endIndex].Value;
            return true;
        }

        if (startIndex > -1 && endIndex == -1) {
            result = m_KeyFrames[startIndex].Value;
            return true;
        }

        if (startIndex > -1 && endIndex > -1) {
            const auto& startKey = m_KeyFrames[startIndex];
            const auto& endKey = m_KeyFrames[endIndex];

            float t = static_cast<float>(frame - startKey.Frame) /
                static_cast<float>(endKey.Frame - startKey.Frame);

            result = InterpolateValues(startKey.Value, endKey.Value, t, startKey.Interpolation);
            return true;
        }

        return false;
    }

    template<typename T>
    T AnimationTrack<T>::InterpolateValues(const T& from, const T& to, float t,
        typename KeyFrameType::InterpolationType type) const {
        if (m_CustomInterpolation) {
            return m_CustomInterpolation(from, to, t);
        }

        switch (type) {
        case KeyFrameType::InterpolationType::None:
            return from;
        case KeyFrameType::InterpolationType::Linear:
            return AnimationUtils::LinearInterpolate(from, to, t);
        case KeyFrameType::InterpolationType::Smooth:
            return AnimationUtils::SmoothInterpolate(from, to, t);
        case KeyFrameType::InterpolationType::Bezier:
            // Note: For bezier, you'd need control points from the keyframes
            return AnimationUtils::SmoothInterpolate(from, to, t);
        default:
            return AnimationUtils::LinearInterpolate(from, to, t);
        }
    }

} // namespace Cherry
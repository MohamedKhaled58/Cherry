#pragma once
#include "Cherry/Core/Core.h"
#include "Scene.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <Cherry.h>

namespace Cherry {

    class SceneManager {
    public:
        static SceneManager& Get() {
            static SceneManager instance;
            return instance;
        }

        // Scene management
        void LoadScene(const std::string& name, const std::string& filepath);
        void UnloadScene(const std::string& name);
        void SetActiveScene(const std::string& name);
        REF(Scene) GetActiveScene() const { return m_ActiveScene; }
        REF(Scene) GetScene(const std::string& name);

        // Scene transitions
        void TransitionToScene(const std::string& sceneName, float fadeTime = 1.0f);
        bool IsTransitioning() const { return m_IsTransitioning; }

        // Update
        void OnUpdate(TimeStep ts);

    private:
        SceneManager() = default;
        ~SceneManager() = default;

        std::unordered_map<std::string, REF(Scene)> m_Scenes;
        REF(Scene) m_ActiveScene;
        std::string m_ActiveSceneName;

        // Scene transition
        bool m_IsTransitioning = false;
        std::string m_NextScene;
        float m_TransitionTime = 0.0f;
        float m_TransitionDuration = 1.0f;
    };

} // namespace Cher
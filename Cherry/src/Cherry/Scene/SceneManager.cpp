// Cherry/src/Cherry/Scene/SceneManager.cpp
#include "CHpch.h"
#include "SceneManager.h"

namespace Cherry {

    void SceneManager::LoadScene(const std::string& name, const std::string& filepath) {
        auto scene = CREATE_REF(Scene);

        // TODO: Implement scene loading from file
        // For now, just create an empty scene

        m_Scenes[name] = scene;
        CH_CORE_INFO("Scene '{}' loaded from '{}'", name, filepath);
    }

    void SceneManager::UnloadScene(const std::string& name) {
        auto it = m_Scenes.find(name);
        if (it != m_Scenes.end()) {
            if (m_ActiveScene == it->second) {
                m_ActiveScene = nullptr;
                m_ActiveSceneName.clear();
            }
            m_Scenes.erase(it);
            CH_CORE_INFO("Scene '{}' unloaded", name);
        }
    }

    void SceneManager::SetActiveScene(const std::string& name) {
        auto it = m_Scenes.find(name);
        if (it != m_Scenes.end()) {
            m_ActiveScene = it->second;
            m_ActiveSceneName = name;
            CH_CORE_INFO("Active scene set to '{}'", name);
        }
        else {
            CH_CORE_WARN("Scene '{}' not found", name);
        }
    }

    REF(Scene) SceneManager::GetScene(const std::string& name) {
        auto it = m_Scenes.find(name);
        return it != m_Scenes.end() ? it->second : nullptr;
    }

    void SceneManager::TransitionToScene(const std::string& sceneName, float fadeTime) {
        if (m_IsTransitioning) {
            CH_CORE_WARN("Scene transition already in progress");
            return;
        }

        auto it = m_Scenes.find(sceneName);
        if (it == m_Scenes.end()) {
            CH_CORE_ERROR("Scene '{}' not found for transition", sceneName);
            return;
        }

        m_IsTransitioning = true;
        m_NextScene = sceneName;
        m_TransitionDuration = fadeTime;
        m_TransitionTime = 0.0f;

        CH_CORE_INFO("Starting transition to scene '{}'", sceneName);
    }

    void SceneManager::OnUpdate(TimeStep ts) {
        if (m_IsTransitioning) {
            m_TransitionTime += ts;

            if (m_TransitionTime >= m_TransitionDuration) {
                // Complete transition
                SetActiveScene(m_NextScene);
                m_IsTransitioning = false;
                m_TransitionTime = 0.0f;
                CH_CORE_INFO("Scene transition completed");
            }
        }

        // Update active scene
        if (m_ActiveScene) {
            m_ActiveScene->OnUpdateRuntime(ts);
        }
    }

} // namespace Cherry
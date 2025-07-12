#include "SceneManager.h"
#include "../Core/Logger.h"
#include "../Renderer/D3D11Renderer.h"

namespace GameEngine {
namespace Scene {

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

SceneManager::~SceneManager() {
    UnloadAllScenes();
}

std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name) {
    // Check if scene already exists
    if (m_scenes.find(name) != m_scenes.end()) {
        LOG_WARNING("Scene '" << name << "' already exists");
        return m_scenes[name];
    }

    // Create new scene
    auto scene = std::make_shared<Scene>(name);
    m_scenes[name] = scene;

    LOG_INFO("Scene created: " << name);

    // Set as active scene if it's the first one
    if (!m_activeScene) {
        SetActiveScene(scene);
    }

    return scene;
}

bool SceneManager::LoadScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        LOG_ERROR("Cannot load scene '" << name << "' - scene not found");
        return false;
    }

    // Call OnLoad for the scene
    it->second->OnLoad();

    LOG_INFO("Scene loaded: " << name);
    return true;
}

bool SceneManager::UnloadScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        LOG_WARNING("Cannot unload scene '" << name << "' - scene not found");
        return false;
    }

    // If this is the active scene, clear it
    if (m_activeScene == it->second) {
        m_activeScene.reset();
    }

    // Call OnUnload for the scene
    it->second->OnUnload();

    // Remove scene
    m_scenes.erase(it);

    LOG_INFO("Scene unloaded: " << name);
    return true;
}

void SceneManager::UnloadAllScenes() {
    LOG_INFO("Unloading all scenes");

    // Clear active scene
    m_activeScene.reset();

    // Unload all scenes
    for (auto& [name, scene] : m_scenes) {
        if (scene) {
            scene->OnUnload();
        }
    }

    // Clear scenes map
    m_scenes.clear();

    LOG_INFO("All scenes unloaded");
}

bool SceneManager::SetActiveScene(const std::string& name) {
    auto scene = FindScene(name);
    return SetActiveScene(scene);
}

bool SceneManager::SetActiveScene(std::shared_ptr<Scene> scene) {
    if (!scene) {
        LOG_ERROR("Cannot set null scene as active");
        return false;
    }

    // Check if scene is in our collection
    bool found = false;
    for (const auto& [name, scenePtr] : m_scenes) {
        if (scenePtr == scene) {
            found = true;
            break;
        }
    }

    if (!found) {
        LOG_ERROR("Cannot set scene as active - scene not managed by SceneManager");
        return false;
    }

    m_activeScene = scene;
    LOG_INFO("Active scene set to: " << scene->GetName());
    return true;
}

std::shared_ptr<Scene> SceneManager::FindScene(const std::string& name) const {
    auto it = m_scenes.find(name);
    return (it != m_scenes.end()) ? it->second : nullptr;
}

std::vector<std::string> SceneManager::GetSceneNames() const {
    std::vector<std::string> names;
    names.reserve(m_scenes.size());

    for (const auto& [name, scene] : m_scenes) {
        names.push_back(name);
    }

    return names;
}

void SceneManager::Update(float deltaTime) {
    if (m_activeScene && m_activeScene->IsActive()) {
        m_activeScene->Update(deltaTime);
    }
}

void SceneManager::Render(Renderer::D3D11Renderer* renderer) {
    if (m_activeScene && m_activeScene->IsActive() && renderer) {
        m_activeScene->Render(renderer);
    }
}

size_t SceneManager::GetTotalEntityCount() const {
    size_t totalCount = 0;
    for (const auto& [name, scene] : m_scenes) {
        if (scene) {
            totalCount += scene->GetEntityCount();
        }
    }
    return totalCount;
}

} // namespace Scene
} // namespace GameEngine

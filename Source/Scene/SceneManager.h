#pragma once

#include "Scene.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace GameEngine {

// Forward declarations
namespace Renderer { class D3D11Renderer; }

namespace Scene {

class SceneManager {
public:
    static SceneManager& GetInstance();

    ~SceneManager();

    // Scene management
    std::shared_ptr<Scene> CreateScene(const std::string& name);
    bool LoadScene(const std::string& name);
    bool UnloadScene(const std::string& name);
    void UnloadAllScenes();

    // Active scene
    std::shared_ptr<Scene> GetActiveScene() const { return m_activeScene; }
    bool SetActiveScene(const std::string& name);
    bool SetActiveScene(std::shared_ptr<Scene> scene);

    // Scene queries
    std::shared_ptr<Scene> FindScene(const std::string& name) const;
    std::vector<std::string> GetSceneNames() const;

    // Scene lifecycle
    void Update(float deltaTime);
    void Render(Renderer::D3D11Renderer* renderer);

    // Statistics
    size_t GetSceneCount() const { return m_scenes.size(); }
    size_t GetTotalEntityCount() const;

private:
    SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::shared_ptr<Scene> m_activeScene;
};

// Convenience macro
#define SCENE_MANAGER SceneManager::GetInstance()

} // namespace Scene
} // namespace GameEngine

#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include "Entity.h"

namespace GameEngine {

// Forward declarations
namespace Renderer { class D3D11Renderer; }

namespace Scene {

class Scene {
public:
    Scene(const std::string& name = "Scene");
    virtual ~Scene();

    // Scene properties
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }

    // Entity management
    Entity* CreateEntity(const std::string& name = "Entity");
    Entity* CreateEntity(EntityID id, const std::string& name = "Entity");

    bool DestroyEntity(EntityID id);
    bool DestroyEntity(Entity* entity);
    void DestroyAllEntities();

    // Entity queries
    Entity* FindEntity(EntityID id) const;
    Entity* FindEntityByName(const std::string& name) const;
    std::vector<Entity*> FindEntitiesByName(const std::string& name) const;

    template<typename T>
    std::vector<Entity*> FindEntitiesWithComponent() const;

    // Get all entities
    const std::vector<std::unique_ptr<Entity>>& GetAllEntities() const { return m_entities; }
    std::vector<Entity*> GetRootEntities() const;

    // Scene lifecycle
    virtual void OnLoad() {}
    virtual void OnUnload() {}
    virtual void Update(float deltaTime);
    virtual void Render(Renderer::D3D11Renderer* renderer);

    // Entity iteration
    template<typename Func>
    void ForEachEntity(Func func);

    template<typename Func>
    void ForEachActiveEntity(Func func);

    // Statistics
    size_t GetEntityCount() const { return m_entities.size(); }
    size_t GetActiveEntityCount() const;

protected:
    std::string m_name;
    bool m_active;

    // Entity storage
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::unordered_map<EntityID, Entity*> m_entityLookup;
    std::queue<EntityID> m_pendingDestroy;

    // ID generation
    EntityID m_nextEntityID;

    // Internal methods
    EntityID GenerateEntityID();
    void ProcessPendingDestroy();

    // Entity registration
    void RegisterEntity(Entity* entity);
    void UnregisterEntity(Entity* entity);

    friend class Entity;
};

// Template implementations
template<typename T>
std::vector<Entity*> Scene::FindEntitiesWithComponent() const {
    std::vector<Entity*> result;

    for (const auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsDestroyed()) {
            if (entity->HasComponent<T>()) {
                result.push_back(entity.get());
            }
        }
    }

    return result;
}

template<typename Func>
void Scene::ForEachEntity(Func func) {
    for (const auto& entity : m_entities) {
        if (entity && !entity->IsDestroyed()) {
            func(entity.get());
        }
    }
}

template<typename Func>
void Scene::ForEachActiveEntity(Func func) {
    for (const auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsDestroyed()) {
            func(entity.get());
        }
    }
}

} // namespace Scene
} // namespace GameEngine

#include "Scene.h"
#include "Component.h"
#include "../Core/Logger.h"
#include "../Renderer/D3D11Renderer.h"

namespace GameEngine {
namespace Scene {

Scene::Scene(const std::string& name)
    : m_name(name)
    , m_active(true)
    , m_nextEntityID(1) // Start from 1, 0 is INVALID_ENTITY_ID
{
    LOG_INFO("Scene created: " << m_name);
}

Scene::~Scene() {
    LOG_INFO("Scene destroyed: " << m_name);
    DestroyAllEntities();
}

Entity* Scene::CreateEntity(const std::string& name) {
    EntityID id = GenerateEntityID();
    return CreateEntity(id, name);
}

Entity* Scene::CreateEntity(EntityID id, const std::string& name) {
    // Check if ID is already in use
    if (m_entityLookup.find(id) != m_entityLookup.end()) {
        LOG_ERROR("Entity ID " << id << " is already in use");
        return nullptr;
    }

    // Create the entity
    auto entity = std::make_unique<Entity>(id, name);
    Entity* entityPtr = entity.get();

    // Set scene reference
    entityPtr->SetScene(this);

    // Register entity
    RegisterEntity(entityPtr);

    // Store entity
    m_entities.push_back(std::move(entity));

    LOG_DEBUG("Entity created in scene: " << name << " (ID: " << id << ")");

    return entityPtr;
}

bool Scene::DestroyEntity(EntityID id) {
    Entity* entity = FindEntity(id);
    return DestroyEntity(entity);
}

bool Scene::DestroyEntity(Entity* entity) {
    if (!entity) return false;

    // Mark for destruction
    entity->Destroy();
    m_pendingDestroy.push(entity->GetID());

    return true;
}

void Scene::DestroyAllEntities() {
    LOG_INFO("Destroying all entities in scene: " << m_name);

    // Mark all entities for destruction
    for (auto& entity : m_entities) {
        if (entity && !entity->IsDestroyed()) {
            entity->Destroy();
        }
    }

    // Clear all containers
    m_entities.clear();
    m_entityLookup.clear();

    // Clear pending destroy queue
    while (!m_pendingDestroy.empty()) {
        m_pendingDestroy.pop();
    }

    LOG_INFO("All entities destroyed in scene: " << m_name);
}

Entity* Scene::FindEntity(EntityID id) const {
    auto it = m_entityLookup.find(id);
    return (it != m_entityLookup.end()) ? it->second : nullptr;
}

Entity* Scene::FindEntityByName(const std::string& name) const {
    for (const auto& entity : m_entities) {
        if (entity && entity->GetName() == name && !entity->IsDestroyed()) {
            return entity.get();
        }
    }
    return nullptr;
}

std::vector<Entity*> Scene::FindEntitiesByName(const std::string& name) const {
    std::vector<Entity*> result;

    for (const auto& entity : m_entities) {
        if (entity && entity->GetName() == name && !entity->IsDestroyed()) {
            result.push_back(entity.get());
        }
    }

    return result;
}

std::vector<Entity*> Scene::GetRootEntities() const {
    std::vector<Entity*> result;

    for (const auto& entity : m_entities) {
        if (entity && !entity->IsDestroyed() && !entity->GetParent()) {
            result.push_back(entity.get());
        }
    }

    return result;
}

void Scene::Update(float deltaTime) {
    if (!m_active) return;

    // Process entities pending destruction
    ProcessPendingDestroy();

    // Update all active entities
    for (const auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsDestroyed()) {
            // Call OnStart for entities that haven't started yet
            if (!entity->m_started) {
                entity->OnStart();
                entity->m_started = true;

                // Start all components
                for (auto& [type, components] : entity->m_components) {
                    for (auto& component : components) {
                        if (component && component->IsEnabled()) {
                            component->OnStart();
                        }
                    }
                }
            }

            // Update entity
            entity->OnUpdate(deltaTime);

            // Update all enabled components
            for (auto& [type, components] : entity->m_components) {
                for (auto& component : components) {
                    if (component && component->IsEnabled()) {
                        component->OnUpdate(deltaTime);
                    }
                }
            }
        }
    }
}

void Scene::Render(Renderer::D3D11Renderer* renderer) {
    if (!m_active || !renderer) return;

    // Render all active entities
    // This is a basic implementation - in a real engine you'd have
    // a proper render queue with sorting, frustum culling, etc.

    for (const auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsDestroyed()) {
            // Here you would render mesh components, etc.
            // For now, this is just a placeholder
        }
    }
}

size_t Scene::GetActiveEntityCount() const {
    size_t count = 0;
    for (const auto& entity : m_entities) {
        if (entity && entity->IsActive() && !entity->IsDestroyed()) {
            count++;
        }
    }
    return count;
}

EntityID Scene::GenerateEntityID() {
    return m_nextEntityID++;
}

void Scene::ProcessPendingDestroy() {
    while (!m_pendingDestroy.empty()) {
        EntityID id = m_pendingDestroy.front();
        m_pendingDestroy.pop();

        // Find and remove entity
        auto it = std::find_if(m_entities.begin(), m_entities.end(),
            [id](const std::unique_ptr<Entity>& entity) {
                return entity && entity->GetID() == id;
            });

        if (it != m_entities.end()) {
            Entity* entity = it->get();

            // Unregister entity
            UnregisterEntity(entity);

            // Remove from container (this will call the destructor)
            m_entities.erase(it);

            LOG_DEBUG("Entity destroyed and removed from scene: ID " << id);
        }
    }
}

void Scene::RegisterEntity(Entity* entity) {
    if (entity) {
        m_entityLookup[entity->GetID()] = entity;
    }
}

void Scene::UnregisterEntity(Entity* entity) {
    if (entity) {
        auto it = m_entityLookup.find(entity->GetID());
        if (it != m_entityLookup.end()) {
            m_entityLookup.erase(it);
        }
    }
}

} // namespace Scene
} // namespace GameEngine

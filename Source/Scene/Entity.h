#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string>
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"

namespace GameEngine {
namespace Scene {

// Forward declarations
class Component;
class Transform;
class Scene;

using EntityID = std::uint32_t;
constexpr EntityID INVALID_ENTITY_ID = 0;

class Entity {
public:
    Entity(EntityID id, const std::string& name = "Entity");
    virtual ~Entity();

    // Entity identification
    EntityID GetID() const { return m_id; }
    const std::string& GetName() const { return m_name; }
    void SetName(const std::string& name) { m_name = name; }

    // Entity state
    bool IsActive() const { return m_active; }
    void SetActive(bool active);

    bool IsDestroyed() const { return m_destroyed; }
    void Destroy();

    // Transform access (every entity has a transform)
    Transform* GetTransform() const { return m_transform.get(); }

    // Component management
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args);

    template<typename T>
    T* GetComponent() const;

    template<typename T>
    bool HasComponent() const;

    template<typename T>
    bool RemoveComponent();

    void RemoveAllComponents();

    // Get all components of a specific type
    template<typename T>
    std::vector<T*> GetComponents() const;

    // Hierarchy management
    Entity* GetParent() const { return m_parent; }
    void SetParent(Entity* parent);

    const std::vector<Entity*>& GetChildren() const { return m_children; }
    void AddChild(Entity* child);
    void RemoveChild(Entity* child);

    // Hierarchy queries
    Entity* FindChild(const std::string& name) const;
    Entity* FindChildRecursive(const std::string& name) const;
    std::vector<Entity*> GetChildrenRecursive() const;

    // Scene reference
    Scene* GetScene() const { return m_scene; }
    void SetScene(Scene* scene) { m_scene = scene; }

    // Lifecycle callbacks
    virtual void OnAwake() {}
    virtual void OnStart() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnDestroy() {}

    // Component events
    virtual void OnComponentAdded(Component* component) {}
    virtual void OnComponentRemoved(Component* component) {}

protected:
    EntityID m_id;
    std::string m_name;
    bool m_active;
    bool m_destroyed;
    bool m_started;

    // Transform component (always present)
    std::unique_ptr<Transform> m_transform;

    // Component storage
    std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> m_components;

    // Hierarchy
    Entity* m_parent;
    std::vector<Entity*> m_children;
    Scene* m_scene;

    // Internal methods
    void SetActiveRecursive(bool active);
    void DestroyRecursive();

    friend class Scene;
};

// Template implementations
template<typename T, typename... Args>
T* Entity::AddComponent(Args&&... args) {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    T* componentPtr = component.get();

    // Set component's entity reference
    componentPtr->SetEntity(this);

    // Store component
    auto typeIndex = std::type_index(typeid(T));
    m_components[typeIndex].push_back(std::move(component));

    // Notify
    OnComponentAdded(componentPtr);

    return componentPtr;
}

template<typename T>
T* Entity::GetComponent() const {
    auto typeIndex = std::type_index(typeid(T));
    auto it = m_components.find(typeIndex);

    if (it != m_components.end() && !it->second.empty()) {
        return static_cast<T*>(it->second[0].get());
    }

    return nullptr;
}

template<typename T>
bool Entity::HasComponent() const {
    return GetComponent<T>() != nullptr;
}

template<typename T>
bool Entity::RemoveComponent() {
    auto typeIndex = std::type_index(typeid(T));
    auto it = m_components.find(typeIndex);

    if (it != m_components.end() && !it->second.empty()) {
        Component* component = it->second[0].get();

        // Notify before removal
        OnComponentRemoved(component);

        // Remove component
        it->second.erase(it->second.begin());

        // Remove empty entry
        if (it->second.empty()) {
            m_components.erase(it);
        }

        return true;
    }

    return false;
}

template<typename T>
std::vector<T*> Entity::GetComponents() const {
    std::vector<T*> components;
    auto typeIndex = std::type_index(typeid(T));
    auto it = m_components.find(typeIndex);

    if (it != m_components.end()) {
        components.reserve(it->second.size());
        for (const auto& component : it->second) {
            components.push_back(static_cast<T*>(component.get()));
        }
    }

    return components;
}

} // namespace Scene
} // namespace GameEngine

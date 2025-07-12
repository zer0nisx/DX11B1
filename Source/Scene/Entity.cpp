#include "Entity.h"
#include "Component.h"
#include "Transform.h"
#include "Scene.h"
#include "../Core/Logger.h"

namespace GameEngine {
namespace Scene {

Entity::Entity(EntityID id, const std::string& name)
    : m_id(id)
    , m_name(name)
    , m_active(true)
    , m_destroyed(false)
    , m_started(false)
    , m_parent(nullptr)
    , m_scene(nullptr)
{
    // Every entity has a transform component
    m_transform = std::make_unique<Transform>();
    m_transform->SetEntity(this);

    LOG_DEBUG("Entity created: " << m_name << " (ID: " << m_id << ")");
}

Entity::~Entity() {
    LOG_DEBUG("Entity destroyed: " << m_name << " (ID: " << m_id << ")");

    // Remove from parent
    if (m_parent) {
        m_parent->RemoveChild(this);
    }

    // Destroy all children
    for (auto child : m_children) {
        if (child && !child->IsDestroyed()) {
            child->Destroy();
        }
    }

    // Clean up components
    RemoveAllComponents();
}

void Entity::SetActive(bool active) {
    if (m_active != active) {
        m_active = active;
        SetActiveRecursive(active);
    }
}

void Entity::Destroy() {
    if (!m_destroyed) {
        m_destroyed = true;
        OnDestroy();
        DestroyRecursive();

        LOG_DEBUG("Entity marked for destruction: " << m_name);
    }
}

void Entity::SetParent(Entity* parent) {
    if (m_parent == parent) return;

    // Remove from current parent
    if (m_parent) {
        m_parent->RemoveChild(this);
    }

    // Set new parent
    m_parent = parent;

    // Add to new parent
    if (m_parent) {
        m_parent->AddChild(this);
    }

    // Update transform hierarchy
    if (m_transform) {
        m_transform->MarkDirty();
    }
}

void Entity::AddChild(Entity* child) {
    if (!child || child == this) return;

    // Check if already a child
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it == m_children.end()) {
        m_children.push_back(child);

        // Update child's parent (avoid infinite recursion)
        if (child->m_parent != this) {
            child->m_parent = this;
        }
    }
}

void Entity::RemoveChild(Entity* child) {
    if (!child) return;

    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        m_children.erase(it);

        // Clear child's parent reference
        if (child->m_parent == this) {
            child->m_parent = nullptr;
        }
    }
}

Entity* Entity::FindChild(const std::string& name) const {
    for (Entity* child : m_children) {
        if (child && child->GetName() == name) {
            return child;
        }
    }
    return nullptr;
}

Entity* Entity::FindChildRecursive(const std::string& name) const {
    // Check direct children first
    Entity* result = FindChild(name);
    if (result) return result;

    // Check children's children
    for (Entity* child : m_children) {
        if (child) {
            result = child->FindChildRecursive(name);
            if (result) return result;
        }
    }

    return nullptr;
}

std::vector<Entity*> Entity::GetChildrenRecursive() const {
    std::vector<Entity*> result;

    for (Entity* child : m_children) {
        if (child) {
            result.push_back(child);

            // Add grandchildren
            auto grandchildren = child->GetChildrenRecursive();
            result.insert(result.end(), grandchildren.begin(), grandchildren.end());
        }
    }

    return result;
}

void Entity::RemoveAllComponents() {
    // Notify components before removal
    for (auto& [type, components] : m_components) {
        for (auto& component : components) {
            if (component) {
                OnComponentRemoved(component.get());
                component->OnDestroy();
            }
        }
    }

    // Clear all components
    m_components.clear();
}

void Entity::SetActiveRecursive(bool active) {
    // Apply to all children
    for (Entity* child : m_children) {
        if (child && !child->IsDestroyed()) {
            child->SetActiveRecursive(active);
        }
    }
}

void Entity::DestroyRecursive() {
    // Mark all children for destruction
    for (Entity* child : m_children) {
        if (child && !child->IsDestroyed()) {
            child->Destroy();
        }
    }
}

} // namespace Scene
} // namespace GameEngine

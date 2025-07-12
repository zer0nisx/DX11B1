#pragma once

#include <string>

namespace GameEngine {
namespace Scene {

// Forward declarations
class Entity;

class Component {
public:
    Component() : m_entity(nullptr), m_enabled(true) {}
    virtual ~Component() = default;

    // Component state
    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled);

    // Entity reference
    Entity* GetEntity() const { return m_entity; }
    void SetEntity(Entity* entity) { m_entity = entity; }

    // Component type information
    virtual std::string GetTypeName() const = 0;

    // Lifecycle callbacks
    virtual void OnAwake() {}
    virtual void OnStart() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnDestroy() {}

    // Component events
    virtual void OnEnabled() {}
    virtual void OnDisabled() {}

protected:
    Entity* m_entity;
    bool m_enabled;

    friend class Entity;
};

// Macro to help implement component type names
#define COMPONENT_TYPE(className) \
    virtual std::string GetTypeName() const override { return #className; }

} // namespace Scene
} // namespace GameEngine

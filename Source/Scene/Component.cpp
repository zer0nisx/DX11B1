#include "Component.h"
#include "Entity.h"

namespace GameEngine {
namespace Scene {

void Component::SetEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;

        if (enabled) {
            OnEnabled();
        } else {
            OnDisabled();
        }
    }
}

} // namespace Scene
} // namespace GameEngine

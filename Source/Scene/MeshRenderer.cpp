#include "MeshRenderer.h"
#include "Entity.h"
#include "Transform.h"
#include "../Renderer/D3D11Renderer.h"
#include "../Core/Logger.h"

namespace GameEngine {
namespace Scene {

MeshRenderer::MeshRenderer()
    : m_mesh(nullptr)
    , m_material(nullptr)
    , m_castShadows(true)
    , m_receiveShadows(true)
{
}

void MeshRenderer::OnStart() {
    LOG_DEBUG("MeshRenderer started for entity: " << (GetEntity() ? GetEntity()->GetName() : "Unknown"));
}

void MeshRenderer::OnUpdate(float deltaTime) {
    // MeshRenderer doesn't need to update per frame typically
    // This could be used for animations, LOD calculations, etc.
}

void MeshRenderer::Render(Renderer::D3D11Renderer* renderer) {
    if (!IsEnabled() || !m_mesh || !renderer) {
        return;
    }

    Entity* entity = GetEntity();
    if (!entity || !entity->IsActive()) {
        return;
    }

    Transform* transform = entity->GetTransform();
    if (!transform) {
        return;
    }

    // Get world matrix from transform
    DirectX::XMMATRIX worldMatrix = transform->GetWorldMatrix();

    // Use material if available, otherwise use default material from mesh
    std::shared_ptr<Mesh::Material> materialToUse = m_material ? m_material : m_mesh->GetMaterial();

    // Set material properties before rendering
    if (materialToUse) {
        // Apply material properties to renderer (this would need to be implemented)
        // For now, just render the mesh
    }

    // Render the mesh with the world transform
    m_mesh->Render(renderer, worldMatrix);
}

} // namespace Scene
} // namespace GameEngine

#pragma once

#include "Component.h"
#include "../Mesh/Mesh.h"
#include "../Mesh/Material.h"
#include <memory>

namespace GameEngine {

// Forward declarations
namespace Renderer { class D3D11Renderer; }

namespace Scene {

class MeshRenderer : public Component {
public:
    COMPONENT_TYPE(MeshRenderer)

    MeshRenderer();
    virtual ~MeshRenderer() = default;

    // Mesh and material access
    std::shared_ptr<Mesh::Mesh> GetMesh() const { return m_mesh; }
    void SetMesh(std::shared_ptr<Mesh::Mesh> mesh) { m_mesh = mesh; }

    std::shared_ptr<Mesh::Material> GetMaterial() const { return m_material; }
    void SetMaterial(std::shared_ptr<Mesh::Material> material) { m_material = material; }

    // Rendering properties
    bool IsCastingShadows() const { return m_castShadows; }
    void SetCastShadows(bool castShadows) { m_castShadows = castShadows; }

    bool IsReceivingShadows() const { return m_receiveShadows; }
    void SetReceiveShadows(bool receiveShadows) { m_receiveShadows = receiveShadows; }

    // Render this component
    void Render(Renderer::D3D11Renderer* renderer);

    // Component lifecycle
    virtual void OnStart() override;
    virtual void OnUpdate(float deltaTime) override;

private:
    std::shared_ptr<Mesh::Mesh> m_mesh;
    std::shared_ptr<Mesh::Material> m_material;

    bool m_castShadows;
    bool m_receiveShadows;
};

} // namespace Scene
} // namespace GameEngine

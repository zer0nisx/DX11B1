#pragma once
#include <vector>
#include <memory>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include "Material.h"
#include "../Math/Matrix4.h"

namespace GameEngine {
namespace Mesh {

using Microsoft::WRL::ComPtr;

// Forward declarations
namespace Animation {
    struct Bone;
}

struct SubMesh {
    UINT startIndex;
    UINT indexCount;
    std::shared_ptr<Material> material;

    SubMesh(UINT start, UINT count, std::shared_ptr<Material> mat = nullptr)
        : startIndex(start), indexCount(count), material(mat) {}
};

class Mesh {
public:
    Mesh(const std::string& name = "");
    virtual ~Mesh() = default;

    // Loading and creation
    bool LoadFromFile(const std::string& filename, class Renderer::D3D11Renderer* renderer);
    static std::shared_ptr<Mesh> LoadFromFile(const std::string& filename, class Renderer::D3D11Renderer* renderer);
    bool CreateFromData(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices,
                       Renderer::D3D11Renderer* renderer);
    bool CreateFromSkinnedData(const std::vector<SkinnedVertex>& vertices, const std::vector<UINT>& indices,
                              Renderer::D3D11Renderer* renderer);

    // Rendering
    void Render(Renderer::D3D11Renderer* renderer, const Math::Matrix4& worldMatrix);
    void RenderSubMesh(Renderer::D3D11Renderer* renderer, UINT subMeshIndex, const Math::Matrix4& worldMatrix);

    // Getters
    const std::string& GetName() const { return m_name; }
    UINT GetVertexCount() const { return m_vertexCount; }
    UINT GetIndexCount() const { return m_indexCount; }
    UINT GetSubMeshCount() const { return static_cast<UINT>(m_subMeshes.size()); }
    bool IsAnimated() const { return m_isAnimated; }
    bool IsLoaded() const { return m_isLoaded; }

    // SubMesh management
    void AddSubMesh(UINT startIndex, UINT indexCount, std::shared_ptr<Material> material = nullptr);
    SubMesh& GetSubMesh(UINT index) { return m_subMeshes[index]; }
    const SubMesh& GetSubMesh(UINT index) const { return m_subMeshes[index]; }

    // Material management
    void SetMaterial(std::shared_ptr<Material> material, UINT subMeshIndex = 0);
    std::shared_ptr<Material> GetMaterial(UINT subMeshIndex = 0) const;

    // Animation support (will be implemented later)
    // const std::vector<Animation::Bone>& GetBones() const { return m_bones; }
    // bool HasSkeleton() const { return !m_bones.empty(); }

    // Static mesh creation helpers
    static std::shared_ptr<Mesh> CreateCube(Renderer::D3D11Renderer* renderer, float size = 1.0f);
    static std::shared_ptr<Mesh> CreateSphere(Renderer::D3D11Renderer* renderer, float radius = 1.0f, UINT segments = 16);
    static std::shared_ptr<Mesh> CreatePlane(Renderer::D3D11Renderer* renderer, float width = 1.0f, float height = 1.0f);

protected:
    bool CreateBuffers(Renderer::D3D11Renderer* renderer);
    void CalculateBoundingBox();

private:
    std::string m_name;

    // Vertex data
    std::vector<Vertex> m_vertices;
    std::vector<SkinnedVertex> m_skinnedVertices;
    std::vector<UINT> m_indices;

    // DirectX buffers
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;

    // Mesh info
    UINT m_vertexCount;
    UINT m_indexCount;
    bool m_isAnimated;
    bool m_isLoaded;

    // Sub-meshes and materials
    std::vector<SubMesh> m_subMeshes;

    // Animation data (placeholder for now)
    // std::vector<Animation::Bone> m_bones;

    // Bounding box
    DirectX::XMFLOAT3 m_boundingBoxMin;
    DirectX::XMFLOAT3 m_boundingBoxMax;
};

} // namespace Mesh
} // namespace GameEngine

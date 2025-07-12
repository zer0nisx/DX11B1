#include "Mesh.h"
#include "AssimpLoader.h"
#include "../Renderer/D3D11Renderer.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Mesh {

Mesh::Mesh(const std::string& name)
    : m_name(name)
    , m_vertexCount(0)
    , m_indexCount(0)
    , m_isAnimated(false)
    , m_isLoaded(false)
    , m_boundingBoxMin(-1.0f, -1.0f, -1.0f)
    , m_boundingBoxMax(1.0f, 1.0f, 1.0f)
{
}

bool Mesh::LoadFromFile(const std::string& filename, Renderer::D3D11Renderer* renderer) {
    AssimpLoader loader;
    std::shared_ptr<Mesh> tempMesh;

    if (!loader.LoadMesh(filename, tempMesh, renderer)) {
        LOG_ERROR("Failed to load mesh from file: " << filename);
        return false;
    }

    // Copy data from loaded mesh to this mesh
    if (tempMesh) {
        *this = *tempMesh;
        m_name = filename;
        m_isLoaded = true;
        return true;
    }

    return false;
}

std::shared_ptr<Mesh> Mesh::LoadFromFile(const std::string& filename, Renderer::D3D11Renderer* renderer) {
    AssimpLoader loader;
    std::shared_ptr<Mesh> mesh;

    if (loader.LoadMesh(filename, mesh, renderer)) {
        return mesh;
    }

    LOG_ERROR("Failed to load mesh from file: " << filename);
    return nullptr;
}

bool Mesh::CreateFromData(const std::vector<Vertex>& vertices, const std::vector<UINT>& indices,
                         Renderer::D3D11Renderer* renderer) {
    if (vertices.empty() || indices.empty() || !renderer) {
        LOG_ERROR("Invalid mesh data or renderer");
        return false;
    }

    m_vertices = vertices;
    m_indices = indices;
    m_vertexCount = static_cast<UINT>(vertices.size());
    m_indexCount = static_cast<UINT>(indices.size());
    m_isAnimated = false;

    if (!CreateBuffers(renderer)) {
        LOG_ERROR("Failed to create mesh buffers");
        return false;
    }

    CalculateBoundingBox();

    // Create default submesh if none exist
    if (m_subMeshes.empty()) {
        AddSubMesh(0, m_indexCount);
    }

    m_isLoaded = true;
    LOG_INFO("Created static mesh '" << m_name << "' with " << m_vertexCount << " vertices and " << m_indexCount << " indices");

    return true;
}

bool Mesh::CreateFromSkinnedData(const std::vector<SkinnedVertex>& vertices, const std::vector<UINT>& indices,
                                Renderer::D3D11Renderer* renderer) {
    if (vertices.empty() || indices.empty() || !renderer) {
        LOG_ERROR("Invalid skinned mesh data or renderer");
        return false;
    }

    m_skinnedVertices = vertices;
    m_indices = indices;
    m_vertexCount = static_cast<UINT>(vertices.size());
    m_indexCount = static_cast<UINT>(indices.size());
    m_isAnimated = true;

    if (!CreateBuffers(renderer)) {
        LOG_ERROR("Failed to create skinned mesh buffers");
        return false;
    }

    // Calculate bounding box from skinned vertices
    if (!m_skinnedVertices.empty()) {
        auto& firstVertex = m_skinnedVertices[0];
        m_boundingBoxMin = firstVertex.position;
        m_boundingBoxMax = firstVertex.position;

        for (const auto& vertex : m_skinnedVertices) {
            m_boundingBoxMin.x = std::min(m_boundingBoxMin.x, vertex.position.x);
            m_boundingBoxMin.y = std::min(m_boundingBoxMin.y, vertex.position.y);
            m_boundingBoxMin.z = std::min(m_boundingBoxMin.z, vertex.position.z);

            m_boundingBoxMax.x = std::max(m_boundingBoxMax.x, vertex.position.x);
            m_boundingBoxMax.y = std::max(m_boundingBoxMax.y, vertex.position.y);
            m_boundingBoxMax.z = std::max(m_boundingBoxMax.z, vertex.position.z);
        }
    }

    // Create default submesh if none exist
    if (m_subMeshes.empty()) {
        AddSubMesh(0, m_indexCount);
    }

    m_isLoaded = true;
    LOG_INFO("Created skinned mesh '" << m_name << "' with " << m_vertexCount << " vertices and " << m_indexCount << " indices");

    return true;
}

void Mesh::Render(Renderer::D3D11Renderer* renderer, const Math::Matrix4& worldMatrix) {
    if (!m_isLoaded || !renderer) {
        return;
    }

    // Update constant buffer with world matrix
    // Note: View and Projection matrices should be set by the rendering system
    // For now, we'll assume they're already set

    // Set vertex buffer
    UINT stride = m_isAnimated ? sizeof(SkinnedVertex) : sizeof(Vertex);
    renderer->SetVertexBuffer(m_vertexBuffer.Get(), stride);

    // Set index buffer
    renderer->SetIndexBuffer(m_indexBuffer.Get());

    // Set primitive topology
    renderer->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Render all submeshes
    for (UINT i = 0; i < m_subMeshes.size(); i++) {
        RenderSubMesh(renderer, i, worldMatrix);
    }
}

void Mesh::RenderSubMesh(Renderer::D3D11Renderer* renderer, UINT subMeshIndex, const Math::Matrix4& worldMatrix) {
    if (subMeshIndex >= m_subMeshes.size() || !renderer) {
        return;
    }

    const SubMesh& subMesh = m_subMeshes[subMeshIndex];

    // Apply material if available
    if (subMesh.material) {
        subMesh.material->Apply(renderer->GetContext());
    }

    // Draw the submesh
    renderer->DrawIndexed(subMesh.indexCount, subMesh.startIndex);
}

void Mesh::AddSubMesh(UINT startIndex, UINT indexCount, std::shared_ptr<Material> material) {
    if (startIndex + indexCount > m_indexCount) {
        LOG_ERROR("SubMesh indices out of range");
        return;
    }

    if (!material) {
        material = std::make_shared<Material>("DefaultMaterial_" + std::to_string(m_subMeshes.size()));
    }

    m_subMeshes.emplace_back(startIndex, indexCount, material);
}

void Mesh::SetMaterial(std::shared_ptr<Material> material, UINT subMeshIndex) {
    if (subMeshIndex >= m_subMeshes.size()) {
        LOG_ERROR("SubMesh index out of range");
        return;
    }

    m_subMeshes[subMeshIndex].material = material;
}

std::shared_ptr<Material> Mesh::GetMaterial(UINT subMeshIndex) const {
    if (subMeshIndex >= m_subMeshes.size()) {
        return nullptr;
    }

    return m_subMeshes[subMeshIndex].material;
}

bool Mesh::CreateBuffers(Renderer::D3D11Renderer* renderer) {
    // Create vertex buffer
    if (m_isAnimated) {
        m_vertexBuffer = renderer->CreateVertexBuffer(
            m_skinnedVertices.data(),
            static_cast<UINT>(m_skinnedVertices.size() * sizeof(SkinnedVertex))
        );
    }
    else {
        m_vertexBuffer = renderer->CreateVertexBuffer(
            m_vertices.data(),
            static_cast<UINT>(m_vertices.size() * sizeof(Vertex))
        );
    }

    if (!m_vertexBuffer) {
        LOG_ERROR("Failed to create vertex buffer");
        return false;
    }

    // Create index buffer
    m_indexBuffer = renderer->CreateIndexBuffer(m_indices.data(), static_cast<UINT>(m_indices.size()));
    if (!m_indexBuffer) {
        LOG_ERROR("Failed to create index buffer");
        return false;
    }

    return true;
}

void Mesh::CalculateBoundingBox() {
    if (m_vertices.empty()) {
        return;
    }

    auto& firstVertex = m_vertices[0];
    m_boundingBoxMin = firstVertex.position;
    m_boundingBoxMax = firstVertex.position;

    for (const auto& vertex : m_vertices) {
        m_boundingBoxMin.x = std::min(m_boundingBoxMin.x, vertex.position.x);
        m_boundingBoxMin.y = std::min(m_boundingBoxMin.y, vertex.position.y);
        m_boundingBoxMin.z = std::min(m_boundingBoxMin.z, vertex.position.z);

        m_boundingBoxMax.x = std::max(m_boundingBoxMax.x, vertex.position.x);
        m_boundingBoxMax.y = std::max(m_boundingBoxMax.y, vertex.position.y);
        m_boundingBoxMax.z = std::max(m_boundingBoxMax.z, vertex.position.z);
    }
}

// Static mesh creation helpers
std::shared_ptr<Mesh> Mesh::CreateCube(Renderer::D3D11Renderer* renderer, float size) {
    float halfSize = size * 0.5f;

    std::vector<Vertex> vertices = {
        // Front face
        {DirectX::XMFLOAT3(-halfSize, -halfSize, -halfSize), DirectX::XMFLOAT3(0, 0, -1), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3(-halfSize,  halfSize, -halfSize), DirectX::XMFLOAT3(0, 0, -1), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3( halfSize,  halfSize, -halfSize), DirectX::XMFLOAT3(0, 0, -1), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3( halfSize, -halfSize, -halfSize), DirectX::XMFLOAT3(0, 0, -1), DirectX::XMFLOAT2(1, 1)},

        // Back face
        {DirectX::XMFLOAT3( halfSize, -halfSize,  halfSize), DirectX::XMFLOAT3(0, 0, 1), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3( halfSize,  halfSize,  halfSize), DirectX::XMFLOAT3(0, 0, 1), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3(-halfSize,  halfSize,  halfSize), DirectX::XMFLOAT3(0, 0, 1), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3(-halfSize, -halfSize,  halfSize), DirectX::XMFLOAT3(0, 0, 1), DirectX::XMFLOAT2(1, 1)},

        // Left face
        {DirectX::XMFLOAT3(-halfSize, -halfSize,  halfSize), DirectX::XMFLOAT3(-1, 0, 0), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3(-halfSize,  halfSize,  halfSize), DirectX::XMFLOAT3(-1, 0, 0), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3(-halfSize,  halfSize, -halfSize), DirectX::XMFLOAT3(-1, 0, 0), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3(-halfSize, -halfSize, -halfSize), DirectX::XMFLOAT3(-1, 0, 0), DirectX::XMFLOAT2(1, 1)},

        // Right face
        {DirectX::XMFLOAT3( halfSize, -halfSize, -halfSize), DirectX::XMFLOAT3(1, 0, 0), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3( halfSize,  halfSize, -halfSize), DirectX::XMFLOAT3(1, 0, 0), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3( halfSize,  halfSize,  halfSize), DirectX::XMFLOAT3(1, 0, 0), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3( halfSize, -halfSize,  halfSize), DirectX::XMFLOAT3(1, 0, 0), DirectX::XMFLOAT2(1, 1)},

        // Top face
        {DirectX::XMFLOAT3(-halfSize,  halfSize, -halfSize), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3(-halfSize,  halfSize,  halfSize), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3( halfSize,  halfSize,  halfSize), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3( halfSize,  halfSize, -halfSize), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(1, 1)},

        // Bottom face
        {DirectX::XMFLOAT3(-halfSize, -halfSize,  halfSize), DirectX::XMFLOAT3(0, -1, 0), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3(-halfSize, -halfSize, -halfSize), DirectX::XMFLOAT3(0, -1, 0), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3( halfSize, -halfSize, -halfSize), DirectX::XMFLOAT3(0, -1, 0), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3( halfSize, -halfSize,  halfSize), DirectX::XMFLOAT3(0, -1, 0), DirectX::XMFLOAT2(1, 1)}
    };

    std::vector<UINT> indices = {
        // Front face
        0, 1, 2, 0, 2, 3,
        // Back face
        4, 5, 6, 4, 6, 7,
        // Left face
        8, 9, 10, 8, 10, 11,
        // Right face
        12, 13, 14, 12, 14, 15,
        // Top face
        16, 17, 18, 16, 18, 19,
        // Bottom face
        20, 21, 22, 20, 22, 23
    };

    auto mesh = std::make_shared<Mesh>("Cube");
    if (mesh->CreateFromData(vertices, indices, renderer)) {
        return mesh;
    }

    return nullptr;
}

std::shared_ptr<Mesh> Mesh::CreateSphere(Renderer::D3D11Renderer* renderer, float radius, UINT segments) {
    std::vector<Vertex> vertices;
    std::vector<UINT> indices;

    // Generate sphere vertices
    for (UINT lat = 0; lat <= segments; lat++) {
        float theta = static_cast<float>(lat) * DirectX::XM_PI / static_cast<float>(segments);
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (UINT lon = 0; lon <= segments; lon++) {
            float phi = static_cast<float>(lon) * 2.0f * DirectX::XM_PI / static_cast<float>(segments);
            float sinPhi = sinf(phi);
            float cosPhi = cosf(phi);

            DirectX::XMFLOAT3 position(
                radius * sinTheta * cosPhi,
                radius * cosTheta,
                radius * sinTheta * sinPhi
            );

            DirectX::XMFLOAT3 normal(
                sinTheta * cosPhi,
                cosTheta,
                sinTheta * sinPhi
            );

            DirectX::XMFLOAT2 texCoord(
                static_cast<float>(lon) / static_cast<float>(segments),
                static_cast<float>(lat) / static_cast<float>(segments)
            );

            vertices.emplace_back(position, normal, texCoord);
        }
    }

    // Generate sphere indices
    for (UINT lat = 0; lat < segments; lat++) {
        for (UINT lon = 0; lon < segments; lon++) {
            UINT current = lat * (segments + 1) + lon;
            UINT next = current + segments + 1;

            // First triangle
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);

            // Second triangle
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }

    auto mesh = std::make_shared<Mesh>("Sphere");
    if (mesh->CreateFromData(vertices, indices, renderer)) {
        return mesh;
    }

    return nullptr;
}

std::shared_ptr<Mesh> Mesh::CreatePlane(Renderer::D3D11Renderer* renderer, float width, float height) {
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    std::vector<Vertex> vertices = {
        {DirectX::XMFLOAT3(-halfWidth, 0, -halfHeight), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(0, 1)},
        {DirectX::XMFLOAT3(-halfWidth, 0,  halfHeight), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(0, 0)},
        {DirectX::XMFLOAT3( halfWidth, 0,  halfHeight), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(1, 0)},
        {DirectX::XMFLOAT3( halfWidth, 0, -halfHeight), DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT2(1, 1)}
    };

    std::vector<UINT> indices = {
        0, 1, 2,
        0, 2, 3
    };

    auto mesh = std::make_shared<Mesh>("Plane");
    if (mesh->CreateFromData(vertices, indices, renderer)) {
        return mesh;
    }

    return nullptr;
}

} // namespace Mesh
} // namespace GameEngine

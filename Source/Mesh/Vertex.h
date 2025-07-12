#pragma once
#include <DirectXMath.h>

namespace GameEngine {
namespace Mesh {

// Basic vertex structure for static meshes
struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;

    Vertex() = default;

    Vertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& norm, const DirectX::XMFLOAT2& tex)
        : position(pos), normal(norm), texCoord(tex) {}

    Vertex(float px, float py, float pz, float nx, float ny, float nz, float tu, float tv)
        : position(px, py, pz), normal(nx, ny, nz), texCoord(tu, tv) {}
};

// Skinned vertex structure for animated meshes
struct SkinnedVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
    DirectX::XMFLOAT4 boneWeights;
    DirectX::XMUINT4 boneIndices;

    SkinnedVertex() = default;

    SkinnedVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& norm, const DirectX::XMFLOAT2& tex)
        : position(pos), normal(norm), texCoord(tex)
        , boneWeights(0.0f, 0.0f, 0.0f, 0.0f)
        , boneIndices(0, 0, 0, 0) {}

    void AddBoneData(unsigned int boneID, float weight) {
        for (int i = 0; i < 4; i++) {
            if (reinterpret_cast<float*>(&boneWeights)[i] == 0.0f) {
                reinterpret_cast<unsigned int*>(&boneIndices)[i] = boneID;
                reinterpret_cast<float*>(&boneWeights)[i] = weight;
                return;
            }
        }
        // If all 4 slots are used, find the smallest weight and replace it
        int minIndex = 0;
        for (int i = 1; i < 4; i++) {
            if (reinterpret_cast<float*>(&boneWeights)[i] < reinterpret_cast<float*>(&boneWeights)[minIndex]) {
                minIndex = i;
            }
        }
        if (weight > reinterpret_cast<float*>(&boneWeights)[minIndex]) {
            reinterpret_cast<unsigned int*>(&boneIndices)[minIndex] = boneID;
            reinterpret_cast<float*>(&boneWeights)[minIndex] = weight;
        }
    }

    void NormalizeWeights() {
        float totalWeight = boneWeights.x + boneWeights.y + boneWeights.z + boneWeights.w;
        if (totalWeight > 0.0f) {
            boneWeights.x /= totalWeight;
            boneWeights.y /= totalWeight;
            boneWeights.z /= totalWeight;
            boneWeights.w /= totalWeight;
        }
    }
};

// Input layout descriptions for DirectX
static const D3D11_INPUT_ELEMENT_DESC VertexInputLayout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

static const D3D11_INPUT_ELEMENT_DESC SkinnedVertexInputLayout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

static constexpr UINT VertexInputLayoutCount = sizeof(VertexInputLayout) / sizeof(D3D11_INPUT_ELEMENT_DESC);
static constexpr UINT SkinnedVertexInputLayoutCount = sizeof(SkinnedVertexInputLayout) / sizeof(D3D11_INPUT_ELEMENT_DESC);

} // namespace Mesh
} // namespace GameEngine

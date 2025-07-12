#pragma once

#ifdef ASSIMP_ENABLED
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Mesh.h"
#include "Vertex.h"
#include "Material.h"
#include "../Math/Matrix4.h"

namespace GameEngine {
namespace Mesh {

// Forward declarations
class Mesh;
class Material;

#ifdef ASSIMP_ENABLED

struct BoneInfo {
    std::string name;
    DirectX::XMMATRIX offsetMatrix;
    int parentIndex;

    BoneInfo() : parentIndex(-1), offsetMatrix(DirectX::XMMatrixIdentity()) {}
};

struct AnimationData {
    std::string name;
    float duration;
    float ticksPerSecond;
    // Animation channels will be added later
};

class AssimpLoader {
public:
    AssimpLoader();
    ~AssimpLoader();

    // Main loading function
    bool LoadMesh(const std::string& filename, std::shared_ptr<Mesh>& outMesh,
                  Renderer::D3D11Renderer* renderer);

    // Static utility functions
    static DirectX::XMMATRIX ConvertMatrix(const aiMatrix4x4& matrix);
    static DirectX::XMFLOAT3 ConvertVector3(const aiVector3D& vector);
    static DirectX::XMFLOAT2 ConvertVector2(const aiVector3D& vector);

    // Getters for loaded data
    const std::vector<BoneInfo>& GetBones() const { return m_bones; }
    const std::vector<AnimationData>& GetAnimations() const { return m_animations; }
    bool HasAnimations() const { return !m_animations.empty(); }

private:
    // Processing functions
    void ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<Mesh> mesh,
                     Renderer::D3D11Renderer* renderer);

    void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices,
                     std::vector<SkinnedVertex>& skinnedVertices, std::vector<UINT>& indices,
                     bool& hasAnimations);

    void ProcessBones(aiMesh* mesh, std::vector<SkinnedVertex>& vertices);
    void ProcessAnimations(const aiScene* scene);
    void ProcessMaterials(const aiScene* scene, const std::string& directory,
                         std::vector<std::shared_ptr<Material>>& materials,
                         Renderer::D3D11Renderer* renderer);

    // Bone management
    int FindBone(const std::string& boneName);
    void AddBone(const std::string& boneName, const DirectX::XMMATRIX& offsetMatrix);

    // Helper functions
    std::string GetDirectory(const std::string& filepath);
    std::shared_ptr<Material> LoadMaterial(aiMaterial* mat, const std::string& directory,
                                          Renderer::D3D11Renderer* renderer);

private:
    Assimp::Importer m_importer;
    std::vector<BoneInfo> m_bones;
    std::vector<AnimationData> m_animations;
    std::unordered_map<std::string, int> m_boneMapping;

    // Current processing state
    std::string m_currentDirectory;
    bool m_hasAnimations;
};

#else

// Dummy implementation when Assimp is not available
class AssimpLoader {
public:
    AssimpLoader() = default;
    ~AssimpLoader() = default;

    bool LoadMesh(const std::string& filename, std::shared_ptr<Mesh>& outMesh,
                  Renderer::D3D11Renderer* renderer) {
        // Log error about missing Assimp
        return false;
    }

    bool HasAnimations() const { return false; }
};

#endif // ASSIMP_ENABLED

} // namespace Mesh
} // namespace GameEngine

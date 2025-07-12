#include "AssimpLoader.h"
#include "../Core/Logger.h"
#include "../Renderer/D3D11Renderer.h"
#include <filesystem>

namespace GameEngine {
namespace Mesh {

#ifdef ASSIMP_ENABLED

AssimpLoader::AssimpLoader()
    : m_hasAnimations(false)
{
    // Configure Assimp importer
    m_importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
        aiPrimitiveType_POINT | aiPrimitiveType_LINE);
}

AssimpLoader::~AssimpLoader() {
    // Assimp cleanup is handled automatically
}

bool AssimpLoader::LoadMesh(const std::string& filename, std::shared_ptr<Mesh>& outMesh,
                           Renderer::D3D11Renderer* renderer) {
    if (!renderer) {
        LOG_ERROR("Renderer is null");
        return false;
    }

    LOG_INFO("Loading mesh from file: " << filename);

    // Clear previous data
    m_bones.clear();
    m_animations.clear();
    m_boneMapping.clear();
    m_hasAnimations = false;
    m_currentDirectory = GetDirectory(filename);

    // Load the file
    const aiScene* scene = m_importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_LimitBoneWeights |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_SplitLargeMeshes |
        aiProcess_ValidateDataStructure |
        aiProcess_OptimizeMeshes
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG_ERROR("Assimp error: " << m_importer.GetErrorString());
        return false;
    }

    // Create mesh object
    std::string meshName = std::filesystem::path(filename).stem().string();
    outMesh = std::make_shared<Mesh>(meshName);

    // Process animations first to determine if this is an animated mesh
    ProcessAnimations(scene);

    // Process the scene
    ProcessNode(scene->mRootNode, scene, outMesh, renderer);

    if (m_hasAnimations) {
        LOG_INFO("Loaded animated mesh '" << meshName << "' with " << m_bones.size() << " bones and "
                << m_animations.size() << " animations");
    } else {
        LOG_INFO("Loaded static mesh '" << meshName << "'");
    }

    return true;
}

void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<Mesh> mesh,
                              Renderer::D3D11Renderer* renderer) {
    // Process all meshes in current node
    for (UINT i = 0; i < node->mNumMeshes; i++) {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<Vertex> vertices;
        std::vector<SkinnedVertex> skinnedVertices;
        std::vector<UINT> indices;
        bool hasAnimations = false;

        ProcessMesh(assimpMesh, scene, vertices, skinnedVertices, indices, hasAnimations);

        // Create the mesh with appropriate data
        if (hasAnimations && !skinnedVertices.empty()) {
            mesh->CreateFromSkinnedData(skinnedVertices, indices, renderer);
            m_hasAnimations = true;
        } else if (!vertices.empty()) {
            mesh->CreateFromData(vertices, indices, renderer);
        }

        // Process materials
        if (assimpMesh->mMaterialIndex >= 0) {
            std::vector<std::shared_ptr<Material>> materials;
            ProcessMaterials(scene, m_currentDirectory, materials, renderer);

            if (assimpMesh->mMaterialIndex < materials.size()) {
                mesh->SetMaterial(materials[assimpMesh->mMaterialIndex]);
            }
        }
    }

    // Process children recursively
    for (UINT i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, mesh, renderer);
    }
}

void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices,
                              std::vector<SkinnedVertex>& skinnedVertices, std::vector<UINT>& indices,
                              bool& hasAnimations) {

    hasAnimations = (mesh->mNumBones > 0);

    if (hasAnimations) {
        // Process skinned vertices
        skinnedVertices.reserve(mesh->mNumVertices);

        for (UINT i = 0; i < mesh->mNumVertices; i++) {
            SkinnedVertex vertex;

            // Position
            vertex.position = ConvertVector3(mesh->mVertices[i]);

            // Normal
            if (mesh->mNormals) {
                vertex.normal = ConvertVector3(mesh->mNormals[i]);
            } else {
                vertex.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            }

            // Texture coordinates
            if (mesh->mTextureCoords[0]) {
                vertex.texCoord = ConvertVector2(mesh->mTextureCoords[0][i]);
            } else {
                vertex.texCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
            }

            // Initialize bone data
            vertex.boneWeights = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
            vertex.boneIndices = DirectX::XMUINT4(0, 0, 0, 0);

            skinnedVertices.push_back(vertex);
        }

        // Process bones
        ProcessBones(mesh, skinnedVertices);
    } else {
        // Process regular vertices
        vertices.reserve(mesh->mNumVertices);

        for (UINT i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            // Position
            vertex.position = ConvertVector3(mesh->mVertices[i]);

            // Normal
            if (mesh->mNormals) {
                vertex.normal = ConvertVector3(mesh->mNormals[i]);
            } else {
                vertex.normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
            }

            // Texture coordinates
            if (mesh->mTextureCoords[0]) {
                vertex.texCoord = ConvertVector2(mesh->mTextureCoords[0][i]);
            } else {
                vertex.texCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }
    }

    // Process indices
    indices.reserve(mesh->mNumFaces * 3);
    for (UINT i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
}

void AssimpLoader::ProcessBones(aiMesh* mesh, std::vector<SkinnedVertex>& vertices) {
    for (UINT i = 0; i < mesh->mNumBones; i++) {
        aiBone* bone = mesh->mBones[i];
        std::string boneName = bone->mName.data;

        int boneIndex = FindBone(boneName);
        if (boneIndex == -1) {
            // Add new bone
            DirectX::XMMATRIX offsetMatrix = ConvertMatrix(bone->mOffsetMatrix);
            AddBone(boneName, offsetMatrix);
            boneIndex = static_cast<int>(m_bones.size()) - 1;
        }

        // Apply bone weights to vertices
        for (UINT j = 0; j < bone->mNumWeights; j++) {
            UINT vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;

            if (vertexID < vertices.size()) {
                vertices[vertexID].AddBoneData(boneIndex, weight);
            }
        }
    }

    // Normalize bone weights
    for (auto& vertex : vertices) {
        vertex.NormalizeWeights();
    }
}

void AssimpLoader::ProcessAnimations(const aiScene* scene) {
    m_animations.reserve(scene->mNumAnimations);

    for (UINT i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* animation = scene->mAnimations[i];

        AnimationData animData;
        animData.name = animation->mName.data;
        animData.duration = static_cast<float>(animation->mDuration);
        animData.ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);

        if (animData.ticksPerSecond == 0.0f) {
            animData.ticksPerSecond = 25.0f; // Default ticks per second
        }

        m_animations.push_back(animData);

        LOG_INFO("Found animation: " << animData.name << " (Duration: " <<
                animData.duration << ", TPS: " << animData.ticksPerSecond << ")");
    }
}

void AssimpLoader::ProcessMaterials(const aiScene* scene, const std::string& directory,
                                   std::vector<std::shared_ptr<Material>>& materials,
                                   Renderer::D3D11Renderer* renderer) {
    materials.reserve(scene->mNumMaterials);

    for (UINT i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        materials.push_back(LoadMaterial(mat, directory, renderer));
    }
}

int AssimpLoader::FindBone(const std::string& boneName) {
    auto it = m_boneMapping.find(boneName);
    if (it != m_boneMapping.end()) {
        return it->second;
    }
    return -1;
}

void AssimpLoader::AddBone(const std::string& boneName, const DirectX::XMMATRIX& offsetMatrix) {
    BoneInfo bone;
    bone.name = boneName;
    bone.offsetMatrix = offsetMatrix;

    int boneIndex = static_cast<int>(m_bones.size());
    m_bones.push_back(bone);
    m_boneMapping[boneName] = boneIndex;
}

std::string AssimpLoader::GetDirectory(const std::string& filepath) {
    return std::filesystem::path(filepath).parent_path().string();
}

std::shared_ptr<Material> AssimpLoader::LoadMaterial(aiMaterial* mat, const std::string& directory,
                                                    Renderer::D3D11Renderer* renderer) {
    aiString name;
    mat->Get(AI_MATKEY_NAME, name);

    auto material = std::make_shared<Material>(name.C_Str());

    // Load diffuse texture
    aiString texturePath;
    if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
        std::string fullPath = directory + "/" + texturePath.data;
        material->LoadDiffuseTexture(renderer->GetDevice(), fullPath);
    }

    // Load material properties
    aiColor3D color;
    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        material->SetDiffuse(DirectX::XMFLOAT3(color.r, color.g, color.b));
    }

    if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        material->SetAmbient(DirectX::XMFLOAT3(color.r, color.g, color.b));
    }

    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        material->SetSpecular(DirectX::XMFLOAT3(color.r, color.g, color.b));
    }

    float shininess;
    if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        material->SetSpecularPower(shininess);
    }

    return material;
}

// Static utility functions
DirectX::XMMATRIX AssimpLoader::ConvertMatrix(const aiMatrix4x4& matrix) {
    return DirectX::XMMATRIX(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4
    );
}

DirectX::XMFLOAT3 AssimpLoader::ConvertVector3(const aiVector3D& vector) {
    return DirectX::XMFLOAT3(vector.x, vector.y, vector.z);
}

DirectX::XMFLOAT2 AssimpLoader::ConvertVector2(const aiVector3D& vector) {
    return DirectX::XMFLOAT2(vector.x, vector.y);
}

#else

// Implementation when Assimp is not available
AssimpLoader::AssimpLoader() {
    LOG_WARNING("AssimpLoader created but Assimp is not enabled. Model loading will not work.");
}

AssimpLoader::~AssimpLoader() {
}

bool AssimpLoader::LoadMesh(const std::string& filename, std::shared_ptr<Mesh>& outMesh,
                           Renderer::D3D11Renderer* renderer) {
    LOG_ERROR("Cannot load mesh '" << filename << "' - Assimp is not enabled in this build.");
    LOG_INFO("To enable Assimp: clone https://github.com/assimp/assimp.git to Dependencies/assimp/");
    return false;
}

#endif // ASSIMP_ENABLED

} // namespace Mesh
} // namespace GameEngine

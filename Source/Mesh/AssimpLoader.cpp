#include "AssimpLoader.h"
#include "../Core/Logger.h"
#include "../Renderer/D3D11Renderer.h"
#include "../Renderer/Texture.h"
#include <filesystem>
#include <algorithm>

using namespace GameEngine::Mesh;
using namespace GameEngine::Core;

#ifdef ASSIMP_ENABLED

AssimpLoader::AssimpLoader()
    : m_hasAnimations(false)
{
    // Configure Assimp importer for optimal loading
    m_importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    m_importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
        aiPrimitiveType_POINT | aiPrimitiveType_LINE);
}

AssimpLoader::~AssimpLoader() = default;

bool AssimpLoader::LoadMesh(const std::string& filename, std::shared_ptr<Mesh>& outMesh,
                           Renderer::D3D11Renderer* renderer) {
    if (!renderer) {
        Logger::GetInstance().LogError("AssimpLoader::LoadMesh - Renderer is null");
        return false;
    }

    if (!std::filesystem::exists(filename)) {
        Logger::GetInstance().LogError("AssimpLoader::LoadMesh - File not found: " + filename);
        return false;
    }

    Logger::GetInstance().LogInfo("Loading mesh: " + filename);

    // Import the file with post-processing
    const aiScene* scene = m_importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_GenNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_LimitBoneWeights |
        aiProcess_SplitLargeMeshes |
        aiProcess_ValidateDataStructure |
        aiProcess_OptimizeMeshes
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Logger::GetInstance().LogError("AssimpLoader - Assimp error: " + std::string(m_importer.GetErrorString()));
        return false;
    }

    // Store current directory for relative path resolution
    m_currentDirectory = GetDirectory(filename);

    // Initialize mesh
    outMesh = std::make_shared<Mesh>();
    m_bones.clear();
    m_boneMapping.clear();
    m_animations.clear();
    m_hasAnimations = false;

    // Check for animations first
    if (scene->mNumAnimations > 0) {
        m_hasAnimations = true;
        ProcessAnimations(scene);
        Logger::GetInstance().LogInfo("Found " + std::to_string(scene->mNumAnimations) + " animations");
    }

    // Process materials
    std::vector<std::shared_ptr<Material>> materials;
    ProcessMaterials(scene, m_currentDirectory, materials, renderer);

    // Process the root node
    ProcessNode(scene->mRootNode, scene, outMesh, renderer);

    // Set materials on mesh
    outMesh->SetMaterials(materials);

    // If we have animations, setup bone data
    if (m_hasAnimations) {
        outMesh->SetBones(m_bones);
        outMesh->SetAnimated(true);
        Logger::GetInstance().LogInfo("Mesh loaded with " + std::to_string(m_bones.size()) + " bones");
    }

    Logger::GetInstance().LogInfo("Mesh loaded successfully: " + filename);
    return true;
}

void AssimpLoader::ProcessNode(aiNode* node, const aiScene* scene, std::shared_ptr<Mesh> mesh,
                              Renderer::D3D11Renderer* renderer) {
    // Process all meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<Vertex> vertices;
        std::vector<SkinnedVertex> skinnedVertices;
        std::vector<UINT> indices;
        bool hasAnimations = false;

        ProcessMesh(assimpMesh, scene, vertices, skinnedVertices, indices, hasAnimations);

        // Create submesh
        if (hasAnimations && !skinnedVertices.empty()) {
            mesh->AddSkinnedSubmesh(skinnedVertices, indices, assimpMesh->mMaterialIndex, renderer);
        } else if (!vertices.empty()) {
            mesh->AddSubmesh(vertices, indices, assimpMesh->mMaterialIndex, renderer);
        }
    }

    // Process child nodes recursively
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene, mesh, renderer);
    }
}

void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices,
                              std::vector<SkinnedVertex>& skinnedVertices, std::vector<UINT>& indices,
                              bool& hasAnimations) {
    hasAnimations = mesh->HasBones() && m_hasAnimations;

    // Reserve space for efficiency
    if (hasAnimations) {
        skinnedVertices.reserve(mesh->mNumVertices);
    } else {
        vertices.reserve(mesh->mNumVertices);
    }
    indices.reserve(mesh->mNumFaces * 3);

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        if (hasAnimations) {
            SkinnedVertex vertex;

            // Position
            vertex.position = ConvertVector3(mesh->mVertices[i]);

            // Normal
            if (mesh->HasNormals()) {
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
        } else {
            Vertex vertex;

            // Position
            vertex.position = ConvertVector3(mesh->mVertices[i]);

            // Normal
            if (mesh->HasNormals()) {
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

    // Process bone weights for skinned meshes
    if (hasAnimations) {
        ProcessBones(mesh, skinnedVertices);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        if (face.mNumIndices == 3) {  // We only support triangles
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
    }
}

void AssimpLoader::ProcessBones(aiMesh* mesh, std::vector<SkinnedVertex>& vertices) {
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone* bone = mesh->mBones[i];
        std::string boneName = bone->mName.C_Str();

        int boneIndex = FindBone(boneName);
        if (boneIndex == -1) {
            // Add new bone
            DirectX::XMMATRIX offsetMatrix = ConvertMatrix(bone->mOffsetMatrix);
            AddBone(boneName, offsetMatrix);
            boneIndex = static_cast<int>(m_bones.size() - 1);
        }

        // Apply bone weights to vertices
        for (unsigned int j = 0; j < bone->mNumWeights; j++) {
            unsigned int vertexId = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;

            if (vertexId < vertices.size()) {
                SkinnedVertex& vertex = vertices[vertexId];

                // Find first empty weight slot
                float* weights = &vertex.boneWeights.x;
                unsigned int* indices = &vertex.boneIndices.x;

                for (int k = 0; k < 4; k++) {
                    if (weights[k] == 0.0f) {
                        weights[k] = weight;
                        indices[k] = static_cast<unsigned int>(boneIndex);
                        break;
                    }
                }
            }
        }
    }

    // Normalize bone weights
    for (auto& vertex : vertices) {
        float totalWeight = vertex.boneWeights.x + vertex.boneWeights.y +
                           vertex.boneWeights.z + vertex.boneWeights.w;

        if (totalWeight > 0.0f) {
            vertex.boneWeights.x /= totalWeight;
            vertex.boneWeights.y /= totalWeight;
            vertex.boneWeights.z /= totalWeight;
            vertex.boneWeights.w /= totalWeight;
        }
    }
}

void AssimpLoader::ProcessAnimations(const aiScene* scene) {
    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* animation = scene->mAnimations[i];

        AnimationData animData;
        animData.name = animation->mName.C_Str();
        animData.duration = static_cast<float>(animation->mDuration);
        animData.ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);

        if (animData.ticksPerSecond == 0.0f) {
            animData.ticksPerSecond = 25.0f; // Default value
        }

        m_animations.push_back(animData);
    }
}

void AssimpLoader::ProcessMaterials(const aiScene* scene, const std::string& directory,
                                   std::vector<std::shared_ptr<Material>>& materials,
                                   Renderer::D3D11Renderer* renderer) {
    materials.reserve(scene->mNumMaterials);

    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* mat = scene->mMaterials[i];
        auto material = LoadMaterial(mat, directory, renderer);
        materials.push_back(material);
    }
}

std::shared_ptr<Material> AssimpLoader::LoadMaterial(aiMaterial* mat, const std::string& directory,
                                                    Renderer::D3D11Renderer* renderer) {
    auto material = std::make_shared<Material>();

    // Get material name
    aiString name;
    if (mat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
        material->SetName(name.C_Str());
    }

    // Get diffuse color
    aiColor3D color;
    if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        material->SetDiffuseColor(DirectX::XMFLOAT3(color.r, color.g, color.b));
    }

    // Get specular color
    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        material->SetSpecularColor(DirectX::XMFLOAT3(color.r, color.g, color.b));
    }

    // Get shininess
    float shininess;
    if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        material->SetShininess(shininess);
    }

    // Load diffuse texture
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString texturePath;
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            std::string fullPath = directory + "/" + texturePath.C_Str();
            std::wstring wPath(fullPath.begin(), fullPath.end());

            // Try to load texture
            auto& textureManager = Renderer::TextureManager::GetInstance();
            auto texture = textureManager.LoadTexture(wPath, renderer->GetDevice());

            if (texture) {
                material->SetDiffuseTexture(texture);
                Logger::GetInstance().LogInfo("Loaded diffuse texture: " + fullPath);
            } else {
                Logger::GetInstance().LogWarning("Failed to load diffuse texture: " + fullPath);
            }
        }
    }

    // Load normal map
    if (mat->GetTextureCount(aiTextureType_NORMALS) > 0) {
        aiString texturePath;
        if (mat->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
            std::string fullPath = directory + "/" + texturePath.C_Str();
            std::wstring wPath(fullPath.begin(), fullPath.end());

            auto& textureManager = Renderer::TextureManager::GetInstance();
            auto texture = textureManager.LoadTexture(wPath, renderer->GetDevice());

            if (texture) {
                material->SetNormalTexture(texture);
                Logger::GetInstance().LogInfo("Loaded normal texture: " + fullPath);
            }
        }
    }

    return material;
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
    bone.parentIndex = -1; // Will be set up later if needed

    int boneIndex = static_cast<int>(m_bones.size());
    m_bones.push_back(bone);
    m_boneMapping[boneName] = boneIndex;
}

std::string AssimpLoader::GetDirectory(const std::string& filepath) {
    std::filesystem::path path(filepath);
    return path.parent_path().string();
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

#endif // ASSIMP_ENABLED

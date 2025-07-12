#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "Mesh.h"

namespace GameEngine {

// Forward declaration
namespace Renderer {
    class D3D11Renderer;
}

namespace Mesh {

class MeshManager {
public:
    static MeshManager& GetInstance();

    void Initialize(Renderer::D3D11Renderer* renderer);
    void Shutdown();

    // Mesh loading and management
    std::shared_ptr<Mesh> LoadMesh(const std::string& filename);
    std::shared_ptr<Mesh> GetMesh(const std::string& name);
    bool UnloadMesh(const std::string& name);
    void UnloadAllMeshes();

    // Primitive creation (cached)
    std::shared_ptr<Mesh> GetCube(float size = 1.0f);
    std::shared_ptr<Mesh> GetSphere(float radius = 1.0f, UINT segments = 16);
    std::shared_ptr<Mesh> GetPlane(float width = 1.0f, float height = 1.0f);

    // Statistics
    size_t GetLoadedMeshCount() const { return m_meshes.size(); }
    void PrintStatistics() const;

    // Resource management
    void ClearUnusedMeshes(); // Remove meshes with only one reference (the manager)

private:
    MeshManager() = default;
    ~MeshManager() = default;

    // Non-copyable
    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager&) = delete;

    std::string GeneratePrimitiveKey(const std::string& type, float param1, float param2 = 0.0f, UINT param3 = 0);

private:
    Renderer::D3D11Renderer* m_renderer;
    std::unordered_map<std::string, std::weak_ptr<Mesh>> m_meshes;
    bool m_initialized;
};

} // namespace Mesh
} // namespace GameEngine

// Macro for easier access
#define MESH_MANAGER GameEngine::Mesh::MeshManager::GetInstance()

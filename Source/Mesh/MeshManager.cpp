#include "MeshManager.h"
#include "../Renderer/D3D11Renderer.h"
#include "../Core/Logger.h"
#include <sstream>
#include <iomanip>

namespace GameEngine {
namespace Mesh {

MeshManager& MeshManager::GetInstance() {
    static MeshManager instance;
    return instance;
}

void MeshManager::Initialize(Renderer::D3D11Renderer* renderer) {
    if (m_initialized) {
        LOG_WARNING("MeshManager already initialized");
        return;
    }

    if (!renderer) {
        LOG_ERROR("Cannot initialize MeshManager with null renderer");
        return;
    }

    m_renderer = renderer;
    m_initialized = true;

    LOG_INFO("MeshManager initialized successfully");
}

void MeshManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    LOG_INFO("MeshManager shutting down...");

    // Clear all meshes
    UnloadAllMeshes();

    m_renderer = nullptr;
    m_initialized = false;

    LOG_INFO("MeshManager shutdown complete");
}

std::shared_ptr<Mesh> MeshManager::LoadMesh(const std::string& filename) {
    if (!m_initialized) {
        LOG_ERROR("MeshManager not initialized");
        return nullptr;
    }

    // Check if mesh is already loaded
    auto it = m_meshes.find(filename);
    if (it != m_meshes.end()) {
        auto mesh = it->second.lock();
        if (mesh) {
            LOG_DEBUG("Returning cached mesh: " << filename);
            return mesh;
        }
        else {
            // Weak pointer is expired, remove it
            m_meshes.erase(it);
        }
    }

    // Load new mesh
    LOG_INFO("Loading new mesh: " << filename);
    auto mesh = Mesh::CreateFromFile(filename, m_renderer);

    if (mesh) {
        m_meshes[filename] = mesh;
        LOG_INFO("Successfully loaded and cached mesh: " << filename);
        return mesh;
    }

    LOG_ERROR("Failed to load mesh: " << filename);
    return nullptr;
}

std::shared_ptr<Mesh> MeshManager::GetMesh(const std::string& name) {
    auto it = m_meshes.find(name);
    if (it != m_meshes.end()) {
        auto mesh = it->second.lock();
        if (mesh) {
            return mesh;
        }
        else {
            // Clean up expired weak pointer
            m_meshes.erase(it);
        }
    }

    return nullptr;
}

bool MeshManager::UnloadMesh(const std::string& name) {
    auto it = m_meshes.find(name);
    if (it != m_meshes.end()) {
        m_meshes.erase(it);
        LOG_INFO("Unloaded mesh: " << name);
        return true;
    }

    LOG_WARNING("Attempted to unload non-existent mesh: " << name);
    return false;
}

void MeshManager::UnloadAllMeshes() {
    size_t count = m_meshes.size();
    m_meshes.clear();

    if (count > 0) {
        LOG_INFO("Unloaded " << count << " meshes");
    }
}

std::shared_ptr<Mesh> MeshManager::GetCube(float size) {
    std::string key = GeneratePrimitiveKey("cube", size);

    // Check cache
    auto mesh = GetMesh(key);
    if (mesh) {
        return mesh;
    }

    // Create new cube
    mesh = Mesh::CreateCube(m_renderer, size);
    if (mesh) {
        m_meshes[key] = mesh;
        LOG_DEBUG("Created and cached cube with size: " << size);
    }

    return mesh;
}

std::shared_ptr<Mesh> MeshManager::GetSphere(float radius, UINT segments) {
    std::string key = GeneratePrimitiveKey("sphere", radius, 0.0f, segments);

    // Check cache
    auto mesh = GetMesh(key);
    if (mesh) {
        return mesh;
    }

    // Create new sphere
    mesh = Mesh::CreateSphere(m_renderer, radius, segments);
    if (mesh) {
        m_meshes[key] = mesh;
        LOG_DEBUG("Created and cached sphere with radius: " << radius << ", segments: " << segments);
    }

    return mesh;
}

std::shared_ptr<Mesh> MeshManager::GetPlane(float width, float height) {
    std::string key = GeneratePrimitiveKey("plane", width, height);

    // Check cache
    auto mesh = GetMesh(key);
    if (mesh) {
        return mesh;
    }

    // Create new plane
    mesh = Mesh::CreatePlane(m_renderer, width, height);
    if (mesh) {
        m_meshes[key] = mesh;
        LOG_DEBUG("Created and cached plane with size: " << width << "x" << height);
    }

    return mesh;
}

void MeshManager::PrintStatistics() const {
    LOG_INFO("=== MeshManager Statistics ===");
    LOG_INFO("Total cached meshes: " << m_meshes.size());

    size_t loadedCount = 0;
    size_t expiredCount = 0;

    for (const auto& pair : m_meshes) {
        if (pair.second.lock()) {
            loadedCount++;
            LOG_DEBUG("  - " << pair.first << " (loaded)");
        } else {
            expiredCount++;
            LOG_DEBUG("  - " << pair.first << " (expired)");
        }
    }

    LOG_INFO("Active meshes: " << loadedCount);
    LOG_INFO("Expired entries: " << expiredCount);
    LOG_INFO("===============================");
}

void MeshManager::ClearUnusedMeshes() {
    auto it = m_meshes.begin();
    size_t removedCount = 0;

    while (it != m_meshes.end()) {
        if (it->second.expired()) {
            it = m_meshes.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }

    if (removedCount > 0) {
        LOG_INFO("Cleared " << removedCount << " unused mesh entries");
    }
}

std::string MeshManager::GeneratePrimitiveKey(const std::string& type, float param1, float param2, UINT param3) {
    std::stringstream ss;
    ss << type << "_" << std::fixed << std::setprecision(2) << param1;

    if (param2 != 0.0f) {
        ss << "_" << param2;
    }

    if (param3 != 0) {
        ss << "_" << param3;
    }

    return ss.str();
}

} // namespace Mesh
} // namespace GameEngine

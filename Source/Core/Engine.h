#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include "Timer.h"
#include "Logger.h"
#include "Window.h"
#include "../Renderer/D3D11Renderer.h"
#include "../Mesh/MeshManager.h"
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"

namespace GameEngine {
namespace Core {

class Engine {
public:
    static Engine& GetInstance();

    bool Initialize(HINSTANCE hInstance, const std::string& title = "DX11 Game Engine",
                   int width = 1024, int height = 768, bool fullscreen = false);
    void Run();
    void Shutdown();

    // Getters
    Window& GetWindow() { return *m_window; }
    Renderer::D3D11Renderer& GetRenderer() { return *m_renderer; }
    Timer& GetTimer() { return *m_timer; }
    Mesh::MeshManager& GetMeshManager() { return Mesh::MeshManager::GetInstance(); }

    bool IsRunning() const { return m_isRunning; }
    void SetRunning(bool running) { m_isRunning = running; }

    // Camera controls
    void SetCameraPosition(const Math::Vector3& position) { m_cameraPosition = position; UpdateViewMatrix(); }
    void SetCameraTarget(const Math::Vector3& target) { m_cameraTarget = target; UpdateViewMatrix(); }
    void SetCameraUp(const Math::Vector3& up) { m_cameraUp = up; UpdateViewMatrix(); }

    const Math::Vector3& GetCameraPosition() const { return m_cameraPosition; }
    const Math::Vector3& GetCameraTarget() const { return m_cameraTarget; }
    const Math::Vector3& GetCameraUp() const { return m_cameraUp; }

    const Math::Matrix4& GetViewMatrix() const { return m_viewMatrix; }
    const Math::Matrix4& GetProjectionMatrix() const { return m_projectionMatrix; }

protected:
    // Virtual methods for game logic
    virtual bool OnInitialize() { return true; }
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}

    // Event handlers
    virtual void OnWindowResize(int width, int height);
    virtual void OnWindowClose();
    virtual void OnKeyboard(int key, bool isDown);
    virtual void OnMouseMove(int x, int y, bool dragging);
    virtual void OnMouseButton(int button, bool isDown);

private:
    Engine();
    ~Engine();

    // Non-copyable
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void Update();
    void Render();
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

    // Core systems
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer::D3D11Renderer> m_renderer;
    std::unique_ptr<Timer> m_timer;

    // Engine state
    bool m_isRunning;
    bool m_isInitialized;
    HINSTANCE m_hInstance;

    // Camera
    Math::Vector3 m_cameraPosition;
    Math::Vector3 m_cameraTarget;
    Math::Vector3 m_cameraUp;
    Math::Matrix4 m_viewMatrix;
    Math::Matrix4 m_projectionMatrix;

    float m_fov;
    float m_nearPlane;
    float m_farPlane;

    // Performance tracking
    float m_frameTimeAccumulator;
    int m_frameCount;
};

} // namespace Core
} // namespace GameEngine

// Macro for easier access to engine instance
#define ENGINE GameEngine::Core::Engine::GetInstance()

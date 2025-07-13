#include "Engine.h"
#include <iostream>

namespace GameEngine {
namespace Core {

// Removed singleton pattern to allow inheritance

Engine::Engine()
    : m_isRunning(false)
    , m_isInitialized(false)
    , m_hInstance(nullptr)
    , m_cameraPosition(0.0f, 0.0f, -10.0f)
    , m_cameraTarget(0.0f, 0.0f, 0.0f)
    , m_cameraUp(0.0f, 1.0f, 0.0f)
    , m_fov(DirectX::XM_PIDIV4)
    , m_nearPlane(0.1f)
    , m_farPlane(1000.0f)
    , m_frameTimeAccumulator(0.0f)
    , m_frameCount(0)
{
}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize(HINSTANCE hInstance, const std::string& title, int width, int height, bool fullscreen) {
    if (m_isInitialized) {
        LOG_WARNING("Engine already initialized");
        return true;
    }

    m_hInstance = hInstance;

    // Initialize logger
    Logger::GetInstance().Initialize("engine.log", LogLevel::Info);
    LOG_INFO("=== Engine Initialization Started ===");

    // Create and initialize window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(hInstance, title, width, height)) {
        LOG_ERROR("Failed to initialize window");
        return false;
    }

    // Set up window event callbacks
    m_window->OnResize = [this](int w, int h) { OnWindowResize(w, h); };
    m_window->OnClose = [this]() { OnWindowClose(); };
    m_window->OnKeyboard = [this](int key, bool isDown) { OnKeyboard(key, isDown); };
    m_window->OnMouseMove = [this](int x, int y, bool dragging) { OnMouseMove(x, y, dragging); };
    m_window->OnMouseButton = [this](int button, bool isDown) { OnMouseButton(button, isDown); };

    // Create and initialize renderer
    m_renderer = std::make_unique<Renderer::D3D11Renderer>();
    if (!m_renderer->Initialize(m_window->GetHandle(), width, height, fullscreen)) {
        LOG_ERROR("Failed to initialize renderer");
        return false;
    }

    // Create and initialize timer
    m_timer = std::make_unique<Timer>();
    m_timer->Start();

    // Initialize camera matrices
    UpdateViewMatrix();
    UpdateProjectionMatrix();

    // Call derived class initialization
    if (!OnInitialize()) {
        LOG_ERROR("Derived class initialization failed");
        return false;
    }

    m_isInitialized = true;
    m_isRunning = true;

    LOG_INFO("=== Engine Initialization Complete ===");
    return true;
}

void Engine::Run() {
    if (!m_isInitialized) {
        LOG_ERROR("Engine not initialized");
        return;
    }

    LOG_INFO("Starting main loop");

    while (m_isRunning) {
        // Process window messages
        if (!m_window->ProcessMessages()) {
            m_isRunning = false;
            break;
        }

        // Skip frame if window is minimized
        if (m_window->IsMinimized()) {
            Sleep(100);
            continue;
        }

        // Update timer
        m_timer->Update();

        // Update game logic
        Update();

        // Render frame
        Render();

        // Performance tracking
        m_frameTimeAccumulator += m_timer->GetDeltaTime();
        m_frameCount++;

        // Log performance every 60 frames
        if (m_frameCount % 60 == 0) {
            float avgFrameTime = m_frameTimeAccumulator / 60.0f;
            LOG_DEBUG("Avg Frame Time: " << (avgFrameTime * 1000.0f) << "ms, FPS: " << m_timer->GetFPS());
            m_frameTimeAccumulator = 0.0f;
        }
    }

    LOG_INFO("Main loop ended");
}

void Engine::Shutdown() {
    if (!m_isInitialized) {
        return;
    }

    LOG_INFO("=== Engine Shutdown Started ===");

    // Call derived class cleanup
    OnShutdown();

    // Shutdown systems in reverse order
    if (m_renderer) {
        m_renderer->Shutdown();
        m_renderer.reset();
    }

    if (m_window) {
        m_window->Shutdown();
        m_window.reset();
    }

    m_timer.reset();

    m_isInitialized = false;
    m_isRunning = false;

    LOG_INFO("=== Engine Shutdown Complete ===");
    Logger::GetInstance().Shutdown();
}

void Engine::Update() {
    float deltaTime = m_timer->GetDeltaTime();

    // Update window title with FPS
    static float titleUpdateTimer = 0.0f;
    titleUpdateTimer += deltaTime;
    if (titleUpdateTimer >= 1.0f) {
        std::string title = "DX11 Game Engine - FPS: " + std::to_string(static_cast<int>(m_timer->GetFPS()));
        m_window->SetTitle(title);
        titleUpdateTimer = 0.0f;
    }

    // Call derived class update
    OnUpdate(deltaTime);
}

void Engine::Render() {
    // Begin frame
    m_renderer->BeginFrame(0.1f, 0.1f, 0.2f, 1.0f); // Dark blue background

    // Call derived class render
    OnRender();

    // End frame
    m_renderer->EndFrame();
}

void Engine::OnWindowResize(int width, int height) {
    if (m_renderer) {
        m_renderer->Resize(width, height);
        UpdateProjectionMatrix();
    }
    LOG_INFO("Window resized to " << width << "x" << height);
}

void Engine::OnWindowClose() {
    LOG_INFO("Window close requested");
    m_isRunning = false;
}

void Engine::OnKeyboard(int key, bool isDown) {
    if (!isDown) return;

    switch (key) {
        case VK_ESCAPE:
            m_isRunning = false;
            break;

        case VK_F11:
            // Toggle fullscreen (placeholder)
            LOG_INFO("F11 pressed - Fullscreen toggle not implemented");
            break;

        case 'W':
            {
                Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
                m_cameraPosition += forward * 0.5f;
                m_cameraTarget += forward * 0.5f;
                UpdateViewMatrix();
            }
            break;

        case 'S':
            {
                Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
                m_cameraPosition -= forward * 0.5f;
                m_cameraTarget -= forward * 0.5f;
                UpdateViewMatrix();
            }
            break;

        case 'A':
            {
                Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
                Math::Vector3 right = forward.Cross(m_cameraUp).Normalized();
                m_cameraPosition -= right * 0.5f;
                m_cameraTarget -= right * 0.5f;
                UpdateViewMatrix();
            }
            break;

        case 'D':
            {
                Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
                Math::Vector3 right = forward.Cross(m_cameraUp).Normalized();
                m_cameraPosition += right * 0.5f;
                m_cameraTarget += right * 0.5f;
                UpdateViewMatrix();
            }
            break;
    }
}

void Engine::OnMouseMove(int x, int y, bool dragging) {
    // Mouse look implementation can be added here
}

void Engine::OnMouseButton(int button, bool isDown) {
    if (isDown) {
        LOG_DEBUG("Mouse button " << button << " pressed");
    }
}

void Engine::UpdateViewMatrix() {
    m_viewMatrix = Math::Matrix4::LookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);
}

void Engine::UpdateProjectionMatrix() {
    if (m_renderer) {
        float aspectRatio = static_cast<float>(m_renderer->GetWidth()) / static_cast<float>(m_renderer->GetHeight());
        m_projectionMatrix = Math::Matrix4::Perspective(m_fov, aspectRatio, m_nearPlane, m_farPlane);
    }
}

} // namespace Core
} // namespace GameEngine

#include "Engine.h"
#include "ConfigManager.h"
#include <iostream>

namespace GameEngine {
namespace Core {

// Removed singleton pattern to allow inheritance

Engine::Engine()
    : m_isRunning(false)
    , m_isInitialized(false)
    , m_configurationLoaded(false)
    , m_hInstance(nullptr)
    , m_configFile("config.xml")
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

bool Engine::Initialize(HINSTANCE hInstance, const std::string& configFile) {
    if (m_isInitialized) {
        LOG_WARNING("Engine already initialized");
        return true;
    }

    m_hInstance = hInstance;
    m_configFile = configFile;

    // First, initialize basic logger with defaults
    Logger::GetInstance().Initialize("engine.log", LogLevel::Info);
    LOG_INFO("=== Engine Initialization Started ===");

    // Load configuration
    if (!LoadConfiguration(configFile)) {
        LOG_WARNING("Failed to load configuration, using defaults");
    }

    // Get configuration settings
    const auto& graphicsSettings = CONFIG_MANAGER.GetGraphicsSettings();
    const auto& engineSettings = CONFIG_MANAGER.GetEngineSettings();
    const auto& assetSettings = CONFIG_MANAGER.GetAssetSettings();

    // Validate configuration
    if (!ValidateConfiguration()) {
        LOG_WARNING("Configuration validation failed, some settings were corrected");
    }

    // Re-initialize logger with config settings
    LogLevel logLevel = LogLevel::Info;
    if (engineSettings.logLevel == "Debug") logLevel = LogLevel::Debug;
    else if (engineSettings.logLevel == "Warning") logLevel = LogLevel::Warning;
    else if (engineSettings.logLevel == "Error") logLevel = LogLevel::Error;

    Logger::GetInstance().Initialize(engineSettings.logFile, logLevel);
    Logger::GetInstance().SetEnabled(engineSettings.enableLogging);

    LOG_INFO("Configuration loaded from: " + configFile);

    // Create and initialize window
    m_window = std::make_unique<Window>();
    if (!m_window->Initialize(hInstance, "DX11 Game Engine",
                            graphicsSettings.windowWidth,
                            graphicsSettings.windowHeight)) {
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
    if (!m_renderer->Initialize(m_window->GetHandle(),
                              graphicsSettings.windowWidth,
                              graphicsSettings.windowHeight,
                              graphicsSettings.fullscreen)) {
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
    LOG_INFO("Window: " + std::to_string(graphicsSettings.windowWidth) + "x" +
             std::to_string(graphicsSettings.windowHeight) +
             (graphicsSettings.fullscreen ? " (Fullscreen)" : " (Windowed)"));
    LOG_INFO("VSync: " + std::string(graphicsSettings.vsync ? "Enabled" : "Disabled"));
    LOG_INFO("Assets Directory: " + assetSettings.assetsDirectory);

    return true;
}

bool Engine::InitializeManual(HINSTANCE hInstance, const std::string& title, int width, int height, bool fullscreen) {
    if (m_isInitialized) {
        LOG_WARNING("Engine already initialized");
        return true;
    }

    // Set manual configuration values
    auto& config = CONFIG_MANAGER;
    GraphicsSettings graphics = config.GetGraphicsSettings();
    graphics.windowWidth = width;
    graphics.windowHeight = height;
    graphics.fullscreen = fullscreen;
    config.SetGraphicsSettings(graphics);

    // Use the main initialize method
    return Initialize(hInstance, "");
}

bool Engine::LoadConfiguration(const std::string& configFile) {
    bool result = CONFIG_MANAGER.LoadConfig(configFile);
    m_configurationLoaded = result;

    if (result) {
        LOG_INFO("Configuration loaded successfully");
    } else {
        LOG_WARNING("Failed to load configuration, using defaults");
        CONFIG_MANAGER.LoadDefaultConfig();
        m_configurationLoaded = true;
    }

    return result;
}

bool Engine::SaveConfiguration(const std::string& configFile) {
    std::string saveFile = configFile.empty() ? m_configFile : configFile;
    bool result = CONFIG_MANAGER.SaveConfig(saveFile);

    if (result) {
        LOG_INFO("Configuration saved to: " + saveFile);
    } else {
        LOG_ERROR("Failed to save configuration to: " + saveFile);
    }

    return result;
}

void Engine::ReloadConfiguration() {
    LOG_INFO("Reloading configuration...");

    // Save current configuration as backup
    CONFIG_MANAGER.CreateConfigBackup();

    // Reload configuration
    if (LoadConfiguration(m_configFile)) {
        // Validate new configuration
        if (!ValidateConfiguration()) {
            LOG_WARNING("New configuration invalid, restoring backup");
            CONFIG_MANAGER.RestoreConfigBackup();
        } else {
            LOG_INFO("Configuration reloaded successfully");
            OnConfigurationChanged();
        }
    } else {
        LOG_ERROR("Failed to reload configuration");
    }
}

bool Engine::ValidateConfiguration() {
    return CONFIG_MANAGER.ValidateSettings();
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

    // Save configuration before shutdown
    if (m_configurationLoaded && !m_configFile.empty()) {
        SaveConfiguration(m_configFile);
    }

    m_isInitialized = false;
    m_isRunning = false;
    m_configurationLoaded = false;

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

    const auto& inputSettings = CONFIG_MANAGER.GetInputSettings();

    // Check key bindings from configuration
    if (key == CONFIG_MANAGER.GetKeyBinding("Exit", VK_ESCAPE)) {
        m_isRunning = false;
        LOG_INFO("Exit key pressed - shutting down");
    }
    else if (key == CONFIG_MANAGER.GetKeyBinding("Forward", 'W')) {
        Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
        m_cameraPosition += forward * 0.5f;
        m_cameraTarget += forward * 0.5f;
        UpdateViewMatrix();
        LOG_DEBUG("Moving forward");
    }
    else if (key == CONFIG_MANAGER.GetKeyBinding("Backward", 'S')) {
        Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
        m_cameraPosition -= forward * 0.5f;
        m_cameraTarget -= forward * 0.5f;
        UpdateViewMatrix();
        LOG_DEBUG("Moving backward");
    }
    else if (key == CONFIG_MANAGER.GetKeyBinding("Left", 'A')) {
        Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
        Math::Vector3 right = forward.Cross(m_cameraUp).Normalized();
        m_cameraPosition -= right * 0.5f;
        m_cameraTarget -= right * 0.5f;
        UpdateViewMatrix();
        LOG_DEBUG("Moving left");
    }
    else if (key == CONFIG_MANAGER.GetKeyBinding("Right", 'D')) {
        Math::Vector3 forward = (m_cameraTarget - m_cameraPosition).Normalized();
        Math::Vector3 right = forward.Cross(m_cameraUp).Normalized();
        m_cameraPosition += right * 0.5f;
        m_cameraTarget += right * 0.5f;
        UpdateViewMatrix();
        LOG_DEBUG("Moving right");
    }
    else if (key == CONFIG_MANAGER.GetKeyBinding("ShowInfo", VK_SPACE)) {
        LOG_INFO("Camera Position: (" + std::to_string(m_cameraPosition.x) + ", " +
                 std::to_string(m_cameraPosition.y) + ", " + std::to_string(m_cameraPosition.z) + ")");
        LOG_INFO("FPS: " + std::to_string(m_timer->GetFPS()));
    }
    else if (key == CONFIG_MANAGER.GetKeyBinding("ResetRotation", 'R')) {
        m_cameraPosition = Math::Vector3(0.0f, 0.0f, -10.0f);
        m_cameraTarget = Math::Vector3(0.0f, 0.0f, 0.0f);
        m_cameraUp = Math::Vector3(0.0f, 1.0f, 0.0f);
        UpdateViewMatrix();
        LOG_INFO("Camera reset to default position");
    }
    else if (key == VK_F5) {
        // Reload configuration
        ReloadConfiguration();
    }
    else if (key == VK_F11) {
        // Toggle fullscreen (placeholder)
        LOG_INFO("F11 pressed - Fullscreen toggle not implemented yet");
    }

    // Handle additional game-specific keys
    switch (key) {
        case VK_F1:
            // Show help
            LOG_INFO("=== Controls ===");
            LOG_INFO("W/A/S/D: Move camera");
            LOG_INFO("Space: Show info");
            LOG_INFO("R: Reset camera");
            LOG_INFO("F5: Reload configuration");
            LOG_INFO("ESC: Exit");
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

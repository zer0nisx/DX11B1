#include "SettingsInterface.h"
#include "Window.h"
#include "Logger.h"
#include "../Renderer/D3D11Renderer.h"
#include "../Input/InputManager.h"

namespace GameEngine {
namespace Core {

SettingsInterface& SettingsInterface::GetInstance() {
    static SettingsInterface instance;
    return instance;
}

void SettingsInterface::Initialize(Window* window, Renderer::D3D11Renderer* renderer) {
    m_window = window;
    m_renderer = renderer;
    m_initialized = true;

    LOG_INFO("SettingsInterface initialized");
}

void SettingsInterface::ApplyAllSettings() {
    if (!m_initialized) {
        LOG_WARNING("SettingsInterface not initialized, cannot apply settings");
        return;
    }

    ApplyGraphicsSettings();
    ApplyInputSettings();
    ApplyPerformanceSettings();

    LOG_INFO("All settings applied successfully");
}

void SettingsInterface::ApplyGraphicsSettings() {
    if (!m_initialized || !m_renderer) {
        LOG_WARNING("Cannot apply graphics settings - renderer not available");
        return;
    }

    const auto& settings = CONFIG_MANAGER.GetGraphicsSettings();

    // Apply VSync and FPS settings
    m_renderer->SetVSync(settings.vsync);
    m_renderer->SetMaxFPS(settings.maxFPS);

    LOG_INFO("Graphics settings applied - VSync: " << (settings.vsync ? "ON" : "OFF") <<
             ", Max FPS: " << settings.maxFPS);
}

void SettingsInterface::ApplyInputSettings() {
    const auto& settings = CONFIG_MANAGER.GetInputSettings();

    // Apply mouse sensitivity
    auto& inputManager = Input::InputManager::GetInstance();
    inputManager.SetMouseSensitivity(settings.mouseSensitivity);

    LOG_INFO("Input settings applied - Mouse sensitivity: " << settings.mouseSensitivity);
}

void SettingsInterface::ApplyPerformanceSettings() {
    // Performance settings are already applied in ApplyGraphicsSettings
    // This method is kept for future performance-related settings
    LOG_DEBUG("Performance settings applied");
}

void SettingsInterface::ToggleVSync() {
    if (!m_initialized || !m_renderer) {
        LOG_WARNING("Cannot toggle VSync - renderer not available");
        return;
    }

    auto& config = CONFIG_MANAGER;
    auto settings = config.GetGraphicsSettings();
    settings.vsync = !settings.vsync;

    config.SetGraphicsSettings(settings);
    m_renderer->SetVSync(settings.vsync);

    LOG_INFO("VSync toggled: " << (settings.vsync ? "ON" : "OFF"));
}

void SettingsInterface::ToggleFullscreen() {
    if (!m_initialized || !m_window) {
        LOG_WARNING("Cannot toggle fullscreen - window not available");
        return;
    }

    bool isFullscreen = m_window->ToggleFullscreen();

    // Update configuration
    auto& config = CONFIG_MANAGER;
    auto settings = config.GetGraphicsSettings();
    settings.fullscreen = isFullscreen;
    config.SetGraphicsSettings(settings);

    LOG_INFO("Fullscreen toggled: " << (isFullscreen ? "ON" : "OFF"));
}

void SettingsInterface::SetMaxFPS(int fps) {
    if (!m_initialized || !m_renderer) {
        LOG_WARNING("Cannot set max FPS - renderer not available");
        return;
    }

    auto& config = CONFIG_MANAGER;
    auto settings = config.GetGraphicsSettings();
    settings.maxFPS = fps;

    config.SetGraphicsSettings(settings);
    m_renderer->SetMaxFPS(fps);

    LOG_INFO("Max FPS set to: " << fps);
}

void SettingsInterface::SetMouseSensitivity(float sensitivity) {
    auto& config = CONFIG_MANAGER;
    auto settings = config.GetInputSettings();
    settings.mouseSensitivity = sensitivity;

    config.SetInputSettings(settings);

    auto& inputManager = Input::InputManager::GetInstance();
    inputManager.SetMouseSensitivity(sensitivity);

    LOG_INFO("Mouse sensitivity set to: " << sensitivity);
}

bool SettingsInterface::IsVSyncEnabled() const {
    return CONFIG_MANAGER.GetGraphicsSettings().vsync;
}

bool SettingsInterface::IsFullscreen() const {
    if (m_window) {
        return m_window->IsFullscreen();
    }
    return CONFIG_MANAGER.GetGraphicsSettings().fullscreen;
}

int SettingsInterface::GetMaxFPS() const {
    return CONFIG_MANAGER.GetGraphicsSettings().maxFPS;
}

float SettingsInterface::GetMouseSensitivity() const {
    return CONFIG_MANAGER.GetInputSettings().mouseSensitivity;
}

} // namespace Core
} // namespace GameEngine

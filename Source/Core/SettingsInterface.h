#pragma once
#include "ConfigManager.h"

namespace GameEngine {

// Forward declarations
namespace Core { class Window; }
namespace Renderer { class D3D11Renderer; }
namespace Input { class InputManager; }

namespace Core {

class SettingsInterface {
public:
    static SettingsInterface& GetInstance();

    // Apply all settings to engine systems
    void ApplyAllSettings();

    // Apply specific setting categories
    void ApplyGraphicsSettings();
    void ApplyInputSettings();
    void ApplyPerformanceSettings();

    // Initialize with engine systems
    void Initialize(Window* window, Renderer::D3D11Renderer* renderer);

    // Performance settings shortcuts
    void ToggleVSync();
    void ToggleFullscreen();
    void SetMaxFPS(int fps);
    void SetMouseSensitivity(float sensitivity);

    // Get current performance status
    bool IsVSyncEnabled() const;
    bool IsFullscreen() const;
    int GetMaxFPS() const;
    float GetMouseSensitivity() const;

private:
    SettingsInterface() = default;
    ~SettingsInterface() = default;
    SettingsInterface(const SettingsInterface&) = delete;
    SettingsInterface& operator=(const SettingsInterface&) = delete;

    // System references
    Window* m_window = nullptr;
    Renderer::D3D11Renderer* m_renderer = nullptr;
    bool m_initialized = false;
};

// Convenient macro for accessing the settings interface
#define SETTINGS_INTERFACE SettingsInterface::GetInstance()

} // namespace Core
} // namespace GameEngine

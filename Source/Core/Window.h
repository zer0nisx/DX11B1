#pragma once
#include <Windows.h>
#include <string>
#include <functional>

namespace GameEngine {
namespace Core {

class Window {
public:
    Window();
    ~Window();

    bool Initialize(HINSTANCE hInstance, const std::string& title, int width, int height);
    void Shutdown();

    bool ProcessMessages();

    HWND GetHandle() const { return m_hwnd; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    bool IsMinimized() const { return m_isMinimized; }
    bool IsMaximized() const { return m_isMaximized; }
    bool IsActive() const { return m_isActive; }

    void SetTitle(const std::string& title);
    void Resize(int width, int height);

    // Fullscreen functionality
    bool IsFullscreen() const { return m_isFullscreen; }
    bool ToggleFullscreen();
    void SetFullscreen(bool fullscreen);

    // Event callbacks
    std::function<void(int, int)> OnResize;
    std::function<void()> OnClose;
    std::function<void(bool)> OnActivate;
    std::function<void(int, int, bool)> OnMouseMove;
    std::function<void(int, bool)> OnMouseButton;
    std::function<void(int, bool)> OnKeyboard;

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void RegisterWindowClass(HINSTANCE hInstance);
    void CreateWindowHandle(HINSTANCE hInstance, const std::string& title);

    HWND m_hwnd;
    HINSTANCE m_hInstance;
    std::string m_className;
    std::string m_title;

    int m_width;
    int m_height;
    bool m_isMinimized;
    bool m_isMaximized;
    bool m_isActive;
    bool m_isInitialized;

    // Fullscreen state
    bool m_isFullscreen;
    RECT m_windowedRect;  // Store windowed position/size
    DWORD m_windowedStyle;
    DWORD m_windowedExStyle;

    static constexpr const char* DEFAULT_CLASS_NAME = "DX11GameEngineWindow";
};

} // namespace Core
} // namespace GameEngine

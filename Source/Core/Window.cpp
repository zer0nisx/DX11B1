#include "Window.h"
#include "Logger.h"
#include <windowsx.h>

namespace GameEngine {
namespace Core {

Window::Window()
    : m_hwnd(nullptr)
    , m_hInstance(nullptr)
    , m_className(DEFAULT_CLASS_NAME)
    , m_title("DX11 Game Engine")
    , m_width(1024)
    , m_height(768)
    , m_isMinimized(false)
    , m_isMaximized(false)
    , m_isActive(true)
    , m_isInitialized(false)
{
}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(HINSTANCE hInstance, const std::string& title, int width, int height) {
    if (m_isInitialized) {
        LOG_WARNING("Window already initialized");
        return true;
    }

    m_hInstance = hInstance;
    m_title = title;
    m_width = width;
    m_height = height;

    try {
        RegisterWindowClass(hInstance);
        CreateWindowHandle(hInstance, title);

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);

        m_isInitialized = true;

        LOG_INFO("Window initialized successfully - " << m_width << "x" << m_height);
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize window: " << e.what());
        return false;
    }
}

void Window::Shutdown() {
    if (!m_isInitialized) {
        return;
    }

    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    if (m_hInstance) {
        UnregisterClass(m_className.c_str(), m_hInstance);
    }

    m_isInitialized = false;
    LOG_INFO("Window shutdown complete");
}

bool Window::ProcessMessages() {
    MSG msg = {};

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void Window::SetTitle(const std::string& title) {
    m_title = title;
    if (m_hwnd) {
        SetWindowText(m_hwnd, title.c_str());
    }
}

void Window::Resize(int width, int height) {
    if (m_hwnd && !m_isMinimized) {
        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        SetWindowPos(m_hwnd, nullptr, 0, 0,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOMOVE | SWP_NOZORDER);
    }
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* window = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else {
        window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->HandleMessage(uMsg, wParam, lParam);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT Window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            if (OnClose) {
                OnClose();
            }
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            {
                m_width = LOWORD(lParam);
                m_height = HIWORD(lParam);

                m_isMinimized = (wParam == SIZE_MINIMIZED);
                m_isMaximized = (wParam == SIZE_MAXIMIZED);

                if (OnResize && !m_isMinimized) {
                    OnResize(m_width, m_height);
                }
            }
            return 0;

        case WM_ACTIVATE:
            m_isActive = (LOWORD(wParam) != WA_INACTIVE);
            if (OnActivate) {
                OnActivate(m_isActive);
            }
            return 0;

        case WM_MOUSEMOVE:
            if (OnMouseMove) {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                OnMouseMove(x, y, false);
            }
            return 0;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            if (OnMouseButton) {
                int button = 0;
                if (uMsg == WM_LBUTTONDOWN) button = 0;
                else if (uMsg == WM_RBUTTONDOWN) button = 1;
                else if (uMsg == WM_MBUTTONDOWN) button = 2;
                OnMouseButton(button, true);
            }
            return 0;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
            if (OnMouseButton) {
                int button = 0;
                if (uMsg == WM_LBUTTONUP) button = 0;
                else if (uMsg == WM_RBUTTONUP) button = 1;
                else if (uMsg == WM_MBUTTONUP) button = 2;
                OnMouseButton(button, false);
            }
            return 0;

        case WM_KEYDOWN:
        case WM_KEYUP:
            if (OnKeyboard) {
                bool isDown = (uMsg == WM_KEYDOWN);
                OnKeyboard(static_cast<int>(wParam), isDown);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void Window::RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = m_className.c_str();
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Failed to register window class");
    }
}

void Window::CreateWindowHandle(HINSTANCE hInstance, const std::string& title) {
    // Calculate window size to get desired client area
    RECT rect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // Center window on screen
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    m_hwnd = CreateWindowEx(
        0,
        m_className.c_str(),
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        x, y,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!m_hwnd) {
        throw std::runtime_error("Failed to create window");
    }
}

} // namespace Core
} // namespace GameEngine

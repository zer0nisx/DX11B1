#pragma once

#include <Windows.h>
#include <Xinput.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <string>

namespace GameEngine {
namespace Input {

// Key codes for keyboard input
enum class KeyCode {
    // Letters
    A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46, G = 0x47, H = 0x48,
    I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C, M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50,
    Q = 0x51, R = 0x52, S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58,
    Y = 0x59, Z = 0x5A,

    // Numbers
    Num0 = 0x30, Num1 = 0x31, Num2 = 0x32, Num3 = 0x33, Num4 = 0x34,
    Num5 = 0x35, Num6 = 0x36, Num7 = 0x37, Num8 = 0x38, Num9 = 0x39,

    // Function keys
    F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4 = VK_F4, F5 = VK_F5, F6 = VK_F6,
    F7 = VK_F7, F8 = VK_F8, F9 = VK_F9, F10 = VK_F10, F11 = VK_F11, F12 = VK_F12,

    // Special keys
    Space = VK_SPACE,
    Enter = VK_RETURN,
    Escape = VK_ESCAPE,
    Tab = VK_TAB,
    Backspace = VK_BACK,
    Delete = VK_DELETE,

    // Arrow keys
    Left = VK_LEFT, Right = VK_RIGHT, Up = VK_UP, Down = VK_DOWN,

    // Modifier keys
    LeftShift = VK_LSHIFT, RightShift = VK_RSHIFT,
    LeftCtrl = VK_LCONTROL, RightCtrl = VK_RCONTROL,
    LeftAlt = VK_LMENU, RightAlt = VK_RMENU,

    // Other
    Insert = VK_INSERT, Home = VK_HOME, End = VK_END,
    PageUp = VK_PRIOR, PageDown = VK_NEXT,

    // Numpad
    Numpad0 = VK_NUMPAD0, Numpad1 = VK_NUMPAD1, Numpad2 = VK_NUMPAD2,
    Numpad3 = VK_NUMPAD3, Numpad4 = VK_NUMPAD4, Numpad5 = VK_NUMPAD5,
    Numpad6 = VK_NUMPAD6, Numpad7 = VK_NUMPAD7, Numpad8 = VK_NUMPAD8,
    Numpad9 = VK_NUMPAD9,
    NumpadEnter = VK_RETURN,
};

// Mouse buttons
enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,
    X2 = 4
};

// Gamepad buttons
enum class GamepadButton {
    DPadUp = XINPUT_GAMEPAD_DPAD_UP,
    DPadDown = XINPUT_GAMEPAD_DPAD_DOWN,
    DPadLeft = XINPUT_GAMEPAD_DPAD_LEFT,
    DPadRight = XINPUT_GAMEPAD_DPAD_RIGHT,
    Start = XINPUT_GAMEPAD_START,
    Back = XINPUT_GAMEPAD_BACK,
    LeftThumb = XINPUT_GAMEPAD_LEFT_THUMB,
    RightThumb = XINPUT_GAMEPAD_RIGHT_THUMB,
    LeftShoulder = XINPUT_GAMEPAD_LEFT_SHOULDER,
    RightShoulder = XINPUT_GAMEPAD_RIGHT_SHOULDER,
    A = XINPUT_GAMEPAD_A,
    B = XINPUT_GAMEPAD_B,
    X = XINPUT_GAMEPAD_X,
    Y = XINPUT_GAMEPAD_Y
};

// Input states
enum class InputState {
    None = 0,
    Pressed = 1,
    Held = 2,
    Released = 3
};

// Mouse state structure
struct MouseState {
    int x, y;           // Position
    int deltaX, deltaY; // Movement delta
    int wheelDelta;     // Scroll wheel delta
    bool buttons[5];    // Button states

    MouseState() : x(0), y(0), deltaX(0), deltaY(0), wheelDelta(0) {
        for (int i = 0; i < 5; i++) buttons[i] = false;
    }
};

// Gamepad state structure
struct GamepadState {
    bool connected;
    bool buttons[14];
    float leftTrigger, rightTrigger;
    float leftStickX, leftStickY;
    float rightStickX, rightStickY;

    GamepadState() : connected(false), leftTrigger(0), rightTrigger(0),
                    leftStickX(0), leftStickY(0), rightStickX(0), rightStickY(0) {
        for (int i = 0; i < 14; i++) buttons[i] = false;
    }
};

// Input event callbacks
using KeyCallback = std::function<void(KeyCode key, InputState state)>;
using MouseButtonCallback = std::function<void(MouseButton button, InputState state, int x, int y)>;
using MouseMoveCallback = std::function<void(int x, int y, int deltaX, int deltaY)>;
using MouseWheelCallback = std::function<void(int delta)>;
using GamepadButtonCallback = std::function<void(int gamepadIndex, GamepadButton button, InputState state)>;

class InputManager {
public:
    static InputManager& GetInstance();

    ~InputManager();

    // Initialization
    bool Initialize(HWND hwnd);
    void Shutdown();

    // Update (call every frame)
    void Update();

    // Keyboard input
    bool IsKeyDown(KeyCode key) const;
    bool IsKeyUp(KeyCode key) const;
    bool IsKeyPressed(KeyCode key) const;  // Pressed this frame
    bool IsKeyReleased(KeyCode key) const; // Released this frame

    // Mouse input
    bool IsMouseButtonDown(MouseButton button) const;
    bool IsMouseButtonUp(MouseButton button) const;
    bool IsMouseButtonPressed(MouseButton button) const;
    bool IsMouseButtonReleased(MouseButton button) const;

    const MouseState& GetMouseState() const { return m_currentMouseState; }
    void GetMousePosition(int& x, int& y) const;
    void GetMouseDelta(int& deltaX, int& deltaY) const;
    int GetMouseWheelDelta() const;

    // Mouse cursor control
    void ShowCursor(bool show);
    void SetCursorPosition(int x, int y);
    void LockCursor(bool lock);

    // Gamepad input
    bool IsGamepadConnected(int gamepadIndex) const;
    bool IsGamepadButtonDown(int gamepadIndex, GamepadButton button) const;
    bool IsGamepadButtonPressed(int gamepadIndex, GamepadButton button) const;
    bool IsGamepadButtonReleased(int gamepadIndex, GamepadButton button) const;

    float GetGamepadTrigger(int gamepadIndex, bool leftTrigger) const;
    void GetGamepadLeftStick(int gamepadIndex, float& x, float& y) const;
    void GetGamepadRightStick(int gamepadIndex, float& x, float& y) const;

    // Vibration
    void SetGamepadVibration(int gamepadIndex, float leftMotor, float rightMotor);

    // Event callbacks
    void SetKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
    void SetMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
    void SetMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallback = callback; }
    void SetMouseWheelCallback(MouseWheelCallback callback) { m_mouseWheelCallback = callback; }
    void SetGamepadButtonCallback(GamepadButtonCallback callback) { m_gamepadButtonCallback = callback; }

    // Window message handling
    void HandleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);

    // Input binding system
    void BindKey(const std::string& action, KeyCode key);
    void BindMouseButton(const std::string& action, MouseButton button);
    void BindGamepadButton(const std::string& action, int gamepadIndex, GamepadButton button);

    bool IsActionPressed(const std::string& action) const;
    bool IsActionHeld(const std::string& action) const;
    bool IsActionReleased(const std::string& action) const;

private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Internal state
    HWND m_hwnd;
    bool m_initialized;

    // Keyboard state
    bool m_currentKeyStates[256];
    bool m_previousKeyStates[256];

    // Mouse state
    MouseState m_currentMouseState;
    MouseState m_previousMouseState;
    bool m_cursorLocked;
    bool m_cursorVisible;

    // Gamepad state (support up to 4 gamepads)
    static constexpr int MAX_GAMEPADS = 4;
    GamepadState m_currentGamepadStates[MAX_GAMEPADS];
    GamepadState m_previousGamepadStates[MAX_GAMEPADS];

    // Callbacks
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    MouseMoveCallback m_mouseMoveCallback;
    MouseWheelCallback m_mouseWheelCallback;
    GamepadButtonCallback m_gamepadButtonCallback;

    // Input binding system
    struct InputBinding {
        enum Type { Key, MouseBtn, GamepadBtn } type;
        union {
            KeyCode key;
            MouseButton mouseButton;
            struct {
                int gamepadIndex;
                GamepadButton button;
            } gamepad;
        };
    };

    std::unordered_map<std::string, std::vector<InputBinding>> m_inputBindings;

    // Internal methods
    void UpdateKeyboardState();
    void UpdateMouseState();
    void UpdateGamepadState();

    InputState GetKeyState(KeyCode key) const;
    InputState GetMouseButtonState(MouseButton button) const;
    InputState GetGamepadButtonState(int gamepadIndex, GamepadButton button) const;

    void TriggerKeyCallback(KeyCode key, InputState state);
    void TriggerMouseButtonCallback(MouseButton button, InputState state);
    void TriggerGamepadButtonCallback(int gamepadIndex, GamepadButton button, InputState state);

    // Utility functions
    float NormalizeStickValue(SHORT value, SHORT deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) const;
    float NormalizeTriggerValue(BYTE value) const;
};

// Convenience macros
#define INPUT InputManager::GetInstance()

} // namespace Input
} // namespace GameEngine

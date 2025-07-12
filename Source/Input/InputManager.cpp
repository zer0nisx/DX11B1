#include "InputManager.h"
#include "../Core/Logger.h"
#include <algorithm>

#pragma comment(lib, "xinput.lib")

namespace GameEngine {
namespace Input {

InputManager& InputManager::GetInstance() {
    static InputManager instance;
    return instance;
}

InputManager::~InputManager() {
    Shutdown();
}

bool InputManager::Initialize(HWND hwnd) {
    if (m_initialized) {
        LOG_WARNING("InputManager already initialized");
        return true;
    }

    m_hwnd = hwnd;

    // Initialize keyboard state
    memset(m_currentKeyStates, 0, sizeof(m_currentKeyStates));
    memset(m_previousKeyStates, 0, sizeof(m_previousKeyStates));

    // Initialize mouse state
    m_currentMouseState = MouseState();
    m_previousMouseState = MouseState();
    m_cursorLocked = false;
    m_cursorVisible = true;

    // Initialize gamepad states
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        m_currentGamepadStates[i] = GamepadState();
        m_previousGamepadStates[i] = GamepadState();
    }

    m_initialized = true;

    LOG_INFO("InputManager initialized successfully");
    return true;
}

void InputManager::Shutdown() {
    if (!m_initialized) return;

    // Show cursor if it was hidden
    if (!m_cursorVisible) {
        ShowCursor(true);
    }

    // Unlock cursor if it was locked
    if (m_cursorLocked) {
        LockCursor(false);
    }

    // Stop all gamepad vibrations
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        SetGamepadVibration(i, 0.0f, 0.0f);
    }

    m_initialized = false;

    LOG_INFO("InputManager shut down");
}

void InputManager::Update() {
    if (!m_initialized) return;

    // Save previous states
    memcpy(m_previousKeyStates, m_currentKeyStates, sizeof(m_currentKeyStates));
    m_previousMouseState = m_currentMouseState;
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        m_previousGamepadStates[i] = m_currentGamepadStates[i];
    }

    // Update current states
    UpdateKeyboardState();
    UpdateMouseState();
    UpdateGamepadState();
}

// Keyboard input methods
bool InputManager::IsKeyDown(KeyCode key) const {
    return m_currentKeyStates[static_cast<int>(key)];
}

bool InputManager::IsKeyUp(KeyCode key) const {
    return !m_currentKeyStates[static_cast<int>(key)];
}

bool InputManager::IsKeyPressed(KeyCode key) const {
    int keyIndex = static_cast<int>(key);
    return m_currentKeyStates[keyIndex] && !m_previousKeyStates[keyIndex];
}

bool InputManager::IsKeyReleased(KeyCode key) const {
    int keyIndex = static_cast<int>(key);
    return !m_currentKeyStates[keyIndex] && m_previousKeyStates[keyIndex];
}

// Mouse input methods
bool InputManager::IsMouseButtonDown(MouseButton button) const {
    return m_currentMouseState.buttons[static_cast<int>(button)];
}

bool InputManager::IsMouseButtonUp(MouseButton button) const {
    return !m_currentMouseState.buttons[static_cast<int>(button)];
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    int buttonIndex = static_cast<int>(button);
    return m_currentMouseState.buttons[buttonIndex] && !m_previousMouseState.buttons[buttonIndex];
}

bool InputManager::IsMouseButtonReleased(MouseButton button) const {
    int buttonIndex = static_cast<int>(button);
    return !m_currentMouseState.buttons[buttonIndex] && m_previousMouseState.buttons[buttonIndex];
}

void InputManager::GetMousePosition(int& x, int& y) const {
    x = m_currentMouseState.x;
    y = m_currentMouseState.y;
}

void InputManager::GetMouseDelta(int& deltaX, int& deltaY) const {
    deltaX = m_currentMouseState.deltaX;
    deltaY = m_currentMouseState.deltaY;
}

int InputManager::GetMouseWheelDelta() const {
    return m_currentMouseState.wheelDelta;
}

// Mouse control methods
void InputManager::ShowCursor(bool show) {
    m_cursorVisible = show;
    ::ShowCursor(show ? TRUE : FALSE);
}

void InputManager::SetCursorPosition(int x, int y) {
    POINT point = { x, y };
    ClientToScreen(m_hwnd, &point);
    SetCursorPos(point.x, point.y);
}

void InputManager::LockCursor(bool lock) {
    m_cursorLocked = lock;

    if (lock) {
        RECT rect;
        GetClientRect(m_hwnd, &rect);
        ClientToScreen(m_hwnd, reinterpret_cast<POINT*>(&rect.left));
        ClientToScreen(m_hwnd, reinterpret_cast<POINT*>(&rect.right));
        ClipCursor(&rect);
    } else {
        ClipCursor(nullptr);
    }
}

// Gamepad input methods
bool InputManager::IsGamepadConnected(int gamepadIndex) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return false;
    return m_currentGamepadStates[gamepadIndex].connected;
}

bool InputManager::IsGamepadButtonDown(int gamepadIndex, GamepadButton button) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return false;
    if (!m_currentGamepadStates[gamepadIndex].connected) return false;

    // Map GamepadButton to array index
    int buttonIndex = 0;
    switch (button) {
        case GamepadButton::DPadUp: buttonIndex = 0; break;
        case GamepadButton::DPadDown: buttonIndex = 1; break;
        case GamepadButton::DPadLeft: buttonIndex = 2; break;
        case GamepadButton::DPadRight: buttonIndex = 3; break;
        case GamepadButton::Start: buttonIndex = 4; break;
        case GamepadButton::Back: buttonIndex = 5; break;
        case GamepadButton::LeftThumb: buttonIndex = 6; break;
        case GamepadButton::RightThumb: buttonIndex = 7; break;
        case GamepadButton::LeftShoulder: buttonIndex = 8; break;
        case GamepadButton::RightShoulder: buttonIndex = 9; break;
        case GamepadButton::A: buttonIndex = 10; break;
        case GamepadButton::B: buttonIndex = 11; break;
        case GamepadButton::X: buttonIndex = 12; break;
        case GamepadButton::Y: buttonIndex = 13; break;
        default: return false;
    }

    return m_currentGamepadStates[gamepadIndex].buttons[buttonIndex];
}

bool InputManager::IsGamepadButtonPressed(int gamepadIndex, GamepadButton button) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return false;

    return GetGamepadButtonState(gamepadIndex, button) == InputState::Pressed;
}

bool InputManager::IsGamepadButtonReleased(int gamepadIndex, GamepadButton button) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return false;

    return GetGamepadButtonState(gamepadIndex, button) == InputState::Released;
}

float InputManager::GetGamepadTrigger(int gamepadIndex, bool leftTrigger) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return 0.0f;
    if (!m_currentGamepadStates[gamepadIndex].connected) return 0.0f;

    return leftTrigger ? m_currentGamepadStates[gamepadIndex].leftTrigger
                      : m_currentGamepadStates[gamepadIndex].rightTrigger;
}

void InputManager::GetGamepadLeftStick(int gamepadIndex, float& x, float& y) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) {
        x = y = 0.0f;
        return;
    }

    if (!m_currentGamepadStates[gamepadIndex].connected) {
        x = y = 0.0f;
        return;
    }

    x = m_currentGamepadStates[gamepadIndex].leftStickX;
    y = m_currentGamepadStates[gamepadIndex].leftStickY;
}

void InputManager::GetGamepadRightStick(int gamepadIndex, float& x, float& y) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) {
        x = y = 0.0f;
        return;
    }

    if (!m_currentGamepadStates[gamepadIndex].connected) {
        x = y = 0.0f;
        return;
    }

    x = m_currentGamepadStates[gamepadIndex].rightStickX;
    y = m_currentGamepadStates[gamepadIndex].rightStickY;
}

void InputManager::SetGamepadVibration(int gamepadIndex, float leftMotor, float rightMotor) {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return;
    if (!m_currentGamepadStates[gamepadIndex].connected) return;

    // Clamp values to [0, 1] range
    leftMotor = std::max(0.0f, std::min(1.0f, leftMotor));
    rightMotor = std::max(0.0f, std::min(1.0f, rightMotor));

    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = static_cast<WORD>(leftMotor * 65535);
    vibration.wRightMotorSpeed = static_cast<WORD>(rightMotor * 65535);

    XInputSetState(gamepadIndex, &vibration);
}

// Window message handling
void InputManager::HandleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (m_keyCallback) {
                KeyCode key = static_cast<KeyCode>(wParam);
                InputState state = IsKeyPressed(key) ? InputState::Pressed : InputState::Held;
                TriggerKeyCallback(key, state);
            }
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (m_keyCallback) {
                KeyCode key = static_cast<KeyCode>(wParam);
                TriggerKeyCallback(key, InputState::Released);
            }
            break;

        case WM_LBUTTONDOWN:
            if (m_mouseButtonCallback) {
                TriggerMouseButtonCallback(MouseButton::Left, InputState::Pressed);
            }
            break;

        case WM_LBUTTONUP:
            if (m_mouseButtonCallback) {
                TriggerMouseButtonCallback(MouseButton::Left, InputState::Released);
            }
            break;

        case WM_RBUTTONDOWN:
            if (m_mouseButtonCallback) {
                TriggerMouseButtonCallback(MouseButton::Right, InputState::Pressed);
            }
            break;

        case WM_RBUTTONUP:
            if (m_mouseButtonCallback) {
                TriggerMouseButtonCallback(MouseButton::Right, InputState::Released);
            }
            break;

        case WM_MBUTTONDOWN:
            if (m_mouseButtonCallback) {
                TriggerMouseButtonCallback(MouseButton::Middle, InputState::Pressed);
            }
            break;

        case WM_MBUTTONUP:
            if (m_mouseButtonCallback) {
                TriggerMouseButtonCallback(MouseButton::Middle, InputState::Released);
            }
            break;

        case WM_MOUSEMOVE:
            if (m_mouseMoveCallback) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                int deltaX = x - m_previousMouseState.x;
                int deltaY = y - m_previousMouseState.y;
                m_mouseMoveCallback(x, y, deltaX, deltaY);
            }
            break;

        case WM_MOUSEWHEEL:
            if (m_mouseWheelCallback) {
                int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                m_mouseWheelCallback(delta);
            }
            break;
    }
}

// Input binding methods
void InputManager::BindKey(const std::string& action, KeyCode key) {
    InputBinding binding;
    binding.type = InputBinding::Key;
    binding.key = key;
    m_inputBindings[action].push_back(binding);
}

void InputManager::BindMouseButton(const std::string& action, MouseButton button) {
    InputBinding binding;
    binding.type = InputBinding::MouseBtn;
    binding.mouseButton = button;
    m_inputBindings[action].push_back(binding);
}

void InputManager::BindGamepadButton(const std::string& action, int gamepadIndex, GamepadButton button) {
    InputBinding binding;
    binding.type = InputBinding::GamepadBtn;
    binding.gamepad.gamepadIndex = gamepadIndex;
    binding.gamepad.button = button;
    m_inputBindings[action].push_back(binding);
}

bool InputManager::IsActionPressed(const std::string& action) const {
    auto it = m_inputBindings.find(action);
    if (it == m_inputBindings.end()) return false;

    for (const auto& binding : it->second) {
        switch (binding.type) {
            case InputBinding::Key:
                if (IsKeyPressed(binding.key)) return true;
                break;
            case InputBinding::MouseBtn:
                if (IsMouseButtonPressed(binding.mouseButton)) return true;
                break;
            case InputBinding::GamepadBtn:
                if (IsGamepadButtonPressed(binding.gamepad.gamepadIndex, binding.gamepad.button)) return true;
                break;
        }
    }

    return false;
}

bool InputManager::IsActionHeld(const std::string& action) const {
    auto it = m_inputBindings.find(action);
    if (it == m_inputBindings.end()) return false;

    for (const auto& binding : it->second) {
        switch (binding.type) {
            case InputBinding::Key:
                if (IsKeyDown(binding.key)) return true;
                break;
            case InputBinding::MouseBtn:
                if (IsMouseButtonDown(binding.mouseButton)) return true;
                break;
            case InputBinding::GamepadBtn:
                if (IsGamepadButtonDown(binding.gamepad.gamepadIndex, binding.gamepad.button)) return true;
                break;
        }
    }

    return false;
}

bool InputManager::IsActionReleased(const std::string& action) const {
    auto it = m_inputBindings.find(action);
    if (it == m_inputBindings.end()) return false;

    for (const auto& binding : it->second) {
        switch (binding.type) {
            case InputBinding::Key:
                if (IsKeyReleased(binding.key)) return true;
                break;
            case InputBinding::MouseBtn:
                if (IsMouseButtonReleased(binding.mouseButton)) return true;
                break;
            case InputBinding::GamepadBtn:
                if (IsGamepadButtonReleased(binding.gamepad.gamepadIndex, binding.gamepad.button)) return true;
                break;
        }
    }

    return false;
}

// Private methods
void InputManager::UpdateKeyboardState() {
    for (int i = 0; i < 256; i++) {
        m_currentKeyStates[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
    }
}

void InputManager::UpdateMouseState() {
    // Get cursor position
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(m_hwnd, &cursorPos);

    // Calculate delta
    m_currentMouseState.deltaX = cursorPos.x - m_currentMouseState.x;
    m_currentMouseState.deltaY = cursorPos.y - m_currentMouseState.y;

    // Update position
    m_currentMouseState.x = cursorPos.x;
    m_currentMouseState.y = cursorPos.y;

    // Update button states
    m_currentMouseState.buttons[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    m_currentMouseState.buttons[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    m_currentMouseState.buttons[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    m_currentMouseState.buttons[3] = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
    m_currentMouseState.buttons[4] = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;

    // Reset wheel delta (it's handled by window messages)
    m_currentMouseState.wheelDelta = 0;
}

void InputManager::UpdateGamepadState() {
    for (int i = 0; i < MAX_GAMEPADS; i++) {
        XINPUT_STATE xinputState;
        DWORD result = XInputGetState(i, &xinputState);

        GamepadState& gamepadState = m_currentGamepadStates[i];

        if (result == ERROR_SUCCESS) {
            gamepadState.connected = true;

            // Update button states
            WORD buttons = xinputState.Gamepad.wButtons;
            gamepadState.buttons[0] = (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0;
            gamepadState.buttons[1] = (buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
            gamepadState.buttons[2] = (buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
            gamepadState.buttons[3] = (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
            gamepadState.buttons[4] = (buttons & XINPUT_GAMEPAD_START) != 0;
            gamepadState.buttons[5] = (buttons & XINPUT_GAMEPAD_BACK) != 0;
            gamepadState.buttons[6] = (buttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
            gamepadState.buttons[7] = (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
            gamepadState.buttons[8] = (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
            gamepadState.buttons[9] = (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
            gamepadState.buttons[10] = (buttons & XINPUT_GAMEPAD_A) != 0;
            gamepadState.buttons[11] = (buttons & XINPUT_GAMEPAD_B) != 0;
            gamepadState.buttons[12] = (buttons & XINPUT_GAMEPAD_X) != 0;
            gamepadState.buttons[13] = (buttons & XINPUT_GAMEPAD_Y) != 0;

            // Update triggers
            gamepadState.leftTrigger = NormalizeTriggerValue(xinputState.Gamepad.bLeftTrigger);
            gamepadState.rightTrigger = NormalizeTriggerValue(xinputState.Gamepad.bRightTrigger);

            // Update sticks
            gamepadState.leftStickX = NormalizeStickValue(xinputState.Gamepad.sThumbLX);
            gamepadState.leftStickY = NormalizeStickValue(xinputState.Gamepad.sThumbLY);
            gamepadState.rightStickX = NormalizeStickValue(xinputState.Gamepad.sThumbRX);
            gamepadState.rightStickY = NormalizeStickValue(xinputState.Gamepad.sThumbRY);
        } else {
            gamepadState.connected = false;
        }
    }
}

InputState InputManager::GetKeyState(KeyCode key) const {
    int keyIndex = static_cast<int>(key);
    bool current = m_currentKeyStates[keyIndex];
    bool previous = m_previousKeyStates[keyIndex];

    if (current && !previous) return InputState::Pressed;
    if (current && previous) return InputState::Held;
    if (!current && previous) return InputState::Released;
    return InputState::None;
}

InputState InputManager::GetMouseButtonState(MouseButton button) const {
    int buttonIndex = static_cast<int>(button);
    bool current = m_currentMouseState.buttons[buttonIndex];
    bool previous = m_previousMouseState.buttons[buttonIndex];

    if (current && !previous) return InputState::Pressed;
    if (current && previous) return InputState::Held;
    if (!current && previous) return InputState::Released;
    return InputState::None;
}

InputState InputManager::GetGamepadButtonState(int gamepadIndex, GamepadButton button) const {
    if (gamepadIndex < 0 || gamepadIndex >= MAX_GAMEPADS) return InputState::None;

    bool current = IsGamepadButtonDown(gamepadIndex, button);

    // Get previous state (simplified for now)
    bool previous = false; // Would need proper tracking

    if (current && !previous) return InputState::Pressed;
    if (current && previous) return InputState::Held;
    if (!current && previous) return InputState::Released;
    return InputState::None;
}

void InputManager::TriggerKeyCallback(KeyCode key, InputState state) {
    if (m_keyCallback) {
        m_keyCallback(key, state);
    }
}

void InputManager::TriggerMouseButtonCallback(MouseButton button, InputState state) {
    if (m_mouseButtonCallback) {
        m_mouseButtonCallback(button, state, m_currentMouseState.x, m_currentMouseState.y);
    }
}

void InputManager::TriggerGamepadButtonCallback(int gamepadIndex, GamepadButton button, InputState state) {
    if (m_gamepadButtonCallback) {
        m_gamepadButtonCallback(gamepadIndex, button, state);
    }
}

float InputManager::NormalizeStickValue(SHORT value, SHORT deadzone) const {
    if (value > deadzone) {
        return static_cast<float>(value - deadzone) / (32767 - deadzone);
    } else if (value < -deadzone) {
        return static_cast<float>(value + deadzone) / (32767 - deadzone);
    }
    return 0.0f;
}

float InputManager::NormalizeTriggerValue(BYTE value) const {
    return static_cast<float>(value) / 255.0f;
}

} // namespace Input
} // namespace GameEngine

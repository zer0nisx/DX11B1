# 🛠️ DX11B1 Motor - Compilation Error Fixes TODO

## 🎯 Compilation Error Fixes Roadmap

This TODO list tracks all critical tasks required to fix the current compilation errors in the DirectX 11 game engine codebase. Follow the priority order for efficient resolution.

---

## 1️⃣ Header Includes and Namespace Fixes

- [ ] **ShadowMap.h**: Add `#include <functional>` for `std::function` usage.
- [ ] **InputManager.h**: Add `#include <string>` for `std::string` usage.
- [ ] **Windows API Headers**: Add missing includes (e.g., `#include <Windows.h>`, `#include <WinUser.h>`) for `VK_` keycode constants.
- [ ] **DirectXTex.h**: Fix include path issues (ensure correct relative or absolute path in all files).
- [ ] **Mesh.h**: Fix `Renderer` namespace references.
    - [ ] Add proper forward declarations for `D3D11Renderer`.
    - [ ] Resolve namespace conflicts between `GameEngine::Renderer` and `GameEngine::Mesh`.

---

## 2️⃣ Vector3 Reference Issues

- [ ] **Vector3.h**: Fix line 96 - "`&` requires lvalue" error.
    - [ ] Review and correct operator overloads to ensure proper lvalue/rvalue usage.
    - [ ] Audit all `Vector3` operator implementations for similar issues.

---

## 3️⃣ Engine Inheritance and Access Modifiers

- [ ] **Engine Class**:
    - [ ] Fix destructor access modifiers (should be `public` or `virtual` if base class).
- [ ] **TestGame**:
    - [ ] Ensure `TestGame` properly inherits from `Engine` (public inheritance).
    - [ ] Check for correct constructor/destructor chaining.

---

## 4️⃣ FileSystem Windows API Integration

- [ ] **FileSystem.h/.cpp**:
    - [ ] Fix Windows API function calls (`CreateDirectoryA`, `DeleteFileA`, etc.)—ensure correct signatures and error handling.
    - [ ] Add and initialize missing member variables (e.g., `m_currentDirectory`).
    - [ ] Fix filesystem iteration syntax (use correct STL or platform-specific APIs).

---

## 5️⃣ DirectX Math Corrections

- [ ] **AnimationController**:
    - [ ] Fix `XMMatrixAdd` usage (ensure correct types and usage).
- [ ] **Transform.cpp**:
    - [ ] Fix `XMMATRIX` member access (cannot access members directly; use DirectXMath functions).
    - [ ] Fix DirectX function parameter types (e.g., pass by value/reference as required).

---

## 6️⃣ Function Signature and Overloading Corrections

- [ ] **Mesh Class**:
    - [ ] Fix `Mesh::LoadFromFile` static/non-static overload conflict.
    - [ ] Resolve parameter type mismatches in all overloads.

---

## 7️⃣ Template and STL Issues

- [ ] Fix `std::min`/`std::max` parameter count errors (should be two parameters).
- [ ] Resolve iterator and range-based for loop issues (ensure correct types and usage).

---

## 🏁 Priority Order

1. Header includes and namespace fixes
2. Vector3 reference issues
3. Engine inheritance and access modifiers
4. FileSystem Windows API integration
5. DirectX Math corrections
6. Function signature corrections
7. Template/STL issues

---

## 📋 Progress Tracking

- [ ] All compilation errors resolved
- [ ] Engine builds successfully on Windows (MSVC)
- [ ] All unit tests pass

---

**Note:** Tackle each item in the listed order for best results. Update this checklist as you resolve each issue.

---

# 🛠️ DX11B1 Motor - Compilation Error Fixes TODO

## 🎯 Compilation Error Fixes Roadmap

This TODO list tracks all critical tasks required to fix the current compilation errors in the DirectX 11 game engine codebase. Follow the priority order for efficient resolution.

---

## High Priority Issues

### 1. Mesh.h Class Declaration Problems
- [ ] Fix function overload conflicts in Mesh class (static vs non-static LoadFromFile)
- [ ] Fix namespace resolution issues for D3D11Renderer references
- [ ] Fix missing includes for proper forward declarations

### 2. FileSystem Class Issues
- [ ] Fix missing member variables (m_currentDirectory)
- [ ] Fix missing method declarations (CreateDirectoryA, DeleteFileA, CopyFileA, etc.)
- [ ] Add proper Windows API includes

### 3. D3D11Renderer Class Issues
- [ ] Fix missing member variables (m_constantBuffer, m_viewport, m_screenWidth, etc.)
- [ ] Fix missing method declarations (CreateViewport, CreateDefaultStates, CleanupRenderTargets)
- [ ] Add proper DirectX includes

### 4. String/Character Encoding Issues
- [ ] Fix MessageBox Unicode/ANSI conversion errors
- [ ] Fix wide character string handling

### 5. Texture Loading Issues
- [ ] Fix DirectX texture creation function parameter mismatches
- [ ] Update function calls to match current DirectXTK API

### 6. RapidXML Template Issues
- [ ] Fix template function resolution errors in rapidxml_print.hpp

### 7. DirectXMath API Usage Issues
- [ ] Fix XMMatrixAdd usage (deprecated function)
- [ ] Fix XMMATRIX member access patterns
- [ ] Fix XMStoreFloat3 parameter issues

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
    - [ ] Fix Windows API function calls (`CreateDirectoryA`, `DeleteFileA`, `CopyFileA`, etc.)—ensure correct signatures and error handling.
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

1. Mesh.h class declaration problems
2. FileSystem class issues
3. D3D11Renderer class issues
4. Header includes and namespace fixes
5. Vector3 reference issues
6. Engine inheritance and access modifiers
7. FileSystem Windows API integration
8. DirectX Math corrections
9. Function signature corrections
10. Template/STL issues
11. String/character encoding issues
12. Texture loading issues
13. RapidXML template issues

---

## 📋 Progress Tracking

- [ ] All compilation errors resolved
- [ ] Engine builds successfully on Windows (MSVC)
- [ ] All unit tests pass

---

**Note:** Tackle each item in the listed order for best results. Update this checklist as you resolve each issue.

---

## Current Status

- Status: Analysis complete, ready to start fixes
- Priority: Focus on core class declaration issues first

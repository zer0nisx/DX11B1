# Instrucciones de Compilación - DX11 Game Engine

## 🔧 Solución a Errores LNK2019 (SettingsInterface)

Los errores que estás experimentando ocurren porque **Visual Studio está intentando compilar el proyecto sin haber generado primero los archivos de proyecto usando CMake**.

### ❌ Error Común
```
error LNK2019: símbolo externo "SettingsInterface::GetInstance" sin resolver
```

### ✅ Solución

#### Paso 1: Generar Proyecto CMake
**IMPORTANTE**: Antes de abrir Visual Studio, debes generar los archivos de proyecto.

1. **Opción A - Script Automático** (Recomendado):
   ```batch
   # Ejecuta desde el directorio raíz del proyecto:
   generate_project.bat
   ```

2. **Opción B - Manual**:
   ```batch
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```

#### Paso 2: Abrir en Visual Studio
Una vez generado el proyecto:
```
build\DX11GameEngine.sln
```

#### Paso 3: Compilar
- **Desde Visual Studio**: Build > Build Solution (F7)
- **Desde línea de comandos**:
  ```batch
  cmake --build build --config Release
  ```

## 🛠️ Requisitos Previos

### Software Necesario
- ✅ **Visual Studio 2019/2022** con "Desktop development with C++"
- ✅ **CMake 3.16+** ([Descargar aquí](https://cmake.org/download/))
- ✅ **Windows SDK** (incluido con Visual Studio)

### Dependencias (vcpkg)
```batch
# Instalar vcpkg si no lo tienes
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Instalar dependencias del proyecto
.\vcpkg install directxtk:x64-windows
.\vcpkg install assimp:x64-windows

# Integrar con Visual Studio
.\vcpkg integrate install
```

## 📋 Verificación de Archivos

### Archivos Críticos Presentes ✅
- `Source/Core/SettingsInterface.h` - Declaración de la clase
- `Source/Core/SettingsInterface.cpp` - Implementación completa
- `CMakeLists.txt` - Configuración de build actualizada

### Métodos Implementados ✅
```cpp
static SettingsInterface& GetInstance();
void ApplyAllSettings();
void Initialize(Window* window, Renderer::D3D11Renderer* renderer);
void ToggleVSync();
void ToggleFullscreen();
bool IsVSyncEnabled() const;
bool IsFullscreen() const;
int GetMaxFPS() const;
float GetMouseSensitivity() const;
```

## 🔍 Resolución de Problemas

### Error: "cmake: command not found"
- **Instalar CMake**: https://cmake.org/download/
- **Agregar al PATH**: Reiniciar terminal después de instalación

### Error: "could not find Visual Studio"
```batch
# Verificar versiones disponibles:
cmake --help

# Usar versión específica:
cmake .. -G "Visual Studio 16 2019" -A x64  # Para VS 2019
cmake .. -G "Visual Studio 17 2022" -A x64  # Para VS 2022
```

### Error: "vcpkg not found"
```batch
# En CMakeLists.txt, actualizar la ruta:
set(CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")
# Cambiar por tu ruta real de vcpkg
```

## 🚀 Compilación Exitosa

Después de seguir estos pasos, deberías ver:
```
========== Build: 1 succeeded, 0 failed ==========
```

## 📁 Estructura Post-Compilación
```
DX11B1/
├── build/
│   ├── DX11GameEngine.sln     # ← Archivo de solución
│   ├── DX11GameEngine.vcxproj # ← Proyecto de Visual Studio
│   └── Release/               # ← Ejecutable compilado
├── Source/                    # ← Código fuente
└── generate_project.bat       # ← Script de generación
```

## ⚡ Pasos Rápidos (Resumen)

1. ✅ Ejecutar `generate_project.bat`
2. ✅ Abrir `build\DX11GameEngine.sln`
3. ✅ Compilar con F7
4. ✅ Ejecutar desde `build\Release\DX11GameEngine.exe`

---

**💡 Nota**: Los errores LNK2019 se resuelven completamente siguiendo estos pasos. El problema era que SettingsInterface.cpp no se estaba incluyendo en la compilación porque el proyecto CMake no había sido generado.

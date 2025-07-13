# Motor de Juego DirectX 11

Un motor de juego 3D modular desarrollado en C++ con DirectX 11, diseñado para cargar archivos .x con soporte para animaciones usando Assimp.

## Características

### ✅ Implementado
- **Motor Core**: Engine principal con sistema singleton
- **Renderer DirectX 11**: Pipeline gráfico completo con shaders
- **Sistema de Ventanas**: Gestión Win32 con callbacks de eventos
- **Matemáticas 3D**: Vector3, Matrix4 con integración DirectXMath
- **Sistema de Mallas**: Carga y renderizado de geometría
- **Sistema de Materiales**: Gestión de texturas y propiedades
- **Logger**: Sistema de logging con niveles y archivos (✅ Corregido)
- **Timer**: Gestión de tiempo y FPS
- **Input Manager**: Sistema de entrada completo con teclado, mouse y gamepad
- **Config Manager**: Gestión de configuración XML (✅ Corregido)
- **Shaders HLSL**: Vertex y Pixel shaders básicos

### 🚧 En Desarrollo
- **Integración Assimp**: Carga de archivos .x y otros formatos 3D
- **Sistema de Animaciones**: Skeletal animation y skinning
- **Gestión de Texturas**: Carga desde archivos de imagen
- **Scene Manager**: Sistema de escena con entities

### 📋 Por Implementar
- **Audio System**: Integración de sonido
- **Physics**: Sistema de física básico
- **Asset Pipeline**: Gestión avanzada de recursos

## Estructura del Proyecto

```
DX11GameEngine/
├── Source/
│   ├── Core/           # Motor principal y sistemas base
│   ├── Renderer/       # Pipeline DirectX 11
│   ├── Mesh/           # Sistema de mallas y geometría
│   ├── Input/          # Sistema de entrada completo
│   ├── Math/           # Matemáticas 3D
│   ├── Animation/      # Sistema de animaciones (futuro)
│   └── Scene/          # Gestión de escena (futuro)
├── Shaders/            # Shaders HLSL
└── Assets/             # Recursos del juego
```

## Compilación

### Requisitos
- **Windows 10/11**
- **Visual Studio 2019/2022** con C++17
- **Windows SDK** (incluye DirectX 11)
- **CMake 3.16+**

### Pasos
1. Clonar el repositorio
2. Crear directorio build: `mkdir build && cd build`
3. Generar proyecto: `cmake ..`
4. Compilar: `cmake --build . --config Release`

### Dependencias Futuras
- **Assimp**: Para carga de modelos 3D
- **DirectXTex**: Para carga de texturas

## ✅ Correcciones de Compilación Aplicadas

Este proyecto ha sido corregido para resolver errores de compilación en Visual Studio. Las siguientes correcciones han sido aplicadas:

### 1. ConfigManager.cpp - Include Faltante
**Problema**: Error de compilación por referencia no resuelta a `InputManager`
```cpp
// ❌ Error original:
// 'InputManager': no se pudo resolver el identificador

// ✅ Corrección aplicada:
#include "../Input/InputManager.h"
```

### 2. Logger.h/Logger.cpp - Métodos Faltantes
**Problema**: Error de compilación por métodos no declarados en la clase Logger
```cpp
// ❌ Errores originales:
// 'SetEnabled': no es miembro de 'GameEngine::Core::Logger'
// 'IsEnabled': no es miembro de 'GameEngine::Core::Logger'

// ✅ Correcciones aplicadas en Logger.h:
void SetEnabled(bool enabled) { m_enabled = enabled; }
bool IsEnabled() const { return m_enabled; }

private:
    bool m_enabled = true;  // Nuevo miembro agregado
```

### 3. Logger.cpp - Lógica de Control
**Problema**: El método `Log()` no respetaba el flag de habilitado
```cpp
// ❌ Código original:
void Logger::Log(LogLevel level, const std::string& message) {
    if (level < m_minLogLevel) {
        return;
    }
    // ... resto del código
}

// ✅ Código corregido:
void Logger::Log(LogLevel level, const std::string& message) {
    if (!m_enabled || level < m_minLogLevel) {
        return;
    }
    // ... resto del código
}
```

### Estado de Compilación
- ✅ **ConfigManager.cpp**: Include corregido
- ✅ **Logger.h**: Métodos SetEnabled/IsEnabled agregados
- ✅ **Logger.cpp**: Lógica de control implementada
- ✅ **Todos los errores resueltos**: El proyecto ahora compila correctamente en Visual Studio

## Uso Básico

```cpp
#include "Core/Engine.h"

class MiJuego : public GameEngine::Core::Engine {
protected:
    bool OnInitialize() override {
        // Inicializar recursos del juego
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Lógica del juego
    }

    void OnRender() override {
        // Renderizado personalizado
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    MiJuego juego;
    juego.Initialize(hInstance, "Mi Juego", 1024, 768);
    juego.Run();
    return 0;
}
```

## Controles de Prueba

### Movimiento de Cámara
- **W**: Avanzar
- **S**: Retroceder
- **A**: Mover izquierda
- **D**: Mover derecha

### Información y Debug
- **Space**: Mostrar información (posición cámara, FPS)
- **R**: Resetear cámara a posición inicial
- **F1**: Mostrar ayuda completa de controles

### Sistema
- **ESC**: Salir del motor
- **F5**: Recargar configuración
- **F11**: Toggle fullscreen
- **F2**: Toggle VSync

### Input Manager
- **Mouse**: Detección de movimiento y clics
- **Gamepad**: Soporte para hasta 4 gamepads Xbox
- **Configuración**: Teclas personalizables via XML

## Arquitectura

### Patrón Singleton
El motor usa el patrón Singleton para managers principales:
- `Engine`: Coordinador principal
- `Logger`: Sistema de logging con control de habilitado/deshabilitado
- `ConfigManager`: Gestión de configuración XML
- `InputManager`: Sistema de entrada unificado
- Renderers y Managers de recursos

### Gestión de Memoria
- **Smart Pointers**: `shared_ptr` y `unique_ptr` para gestión automática
- **ComPtr**: Para recursos DirectX
- **RAII**: Inicialización y limpieza automática

### Sistema de Eventos
- Callbacks para eventos de ventana
- Sistema extensible para input personalizado

## Shaders

### Vertex Shader Básico
```hlsl
cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};
```

### Skinned Vertex Shader (Futuro)
```hlsl
cbuffer BoneBuffer : register(b1) {
    matrix BoneTransforms[100];
};
```

## Roadmap

### Versión 0.2 (Próxima)
- [ ] Integración completa con Assimp
- [ ] Carga de archivos .x
- [ ] Sistema básico de animaciones
- [ ] Carga de texturas

### Versión 0.3
- [ ] Scene Manager completo
- [ ] Sistema de input avanzado
- [ ] Optimizaciones de renderizado

### Versión 1.0
- [ ] Motor completo y estable
- [ ] Documentación completa
- [ ] Ejemplos y tutoriales

## Contribución

1. Fork del proyecto
2. Crear branch para feature: `git checkout -b feature/nueva-caracteristica`
3. Commit cambios: `git commit -am 'Agregar nueva característica'`
4. Push al branch: `git push origin feature/nueva-caracteristica`
5. Crear Pull Request

## Licencia

Este proyecto está bajo la Licencia MIT. Ver `LICENSE` para más detalles.

## Contacto

- **Desarrollador**: Tu Nombre
- **Email**: tu.email@ejemplo.com
- **GitHub**: https://github.com/tu-usuario/dx11-game-engine

---

*Motor desarrollado como proyecto educativo para aprender DirectX 11 y arquitectura de motores de juego.*

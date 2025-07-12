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
- **Logger**: Sistema de logging con niveles y archivos
- **Timer**: Gestión de tiempo y FPS
- **Shaders HLSL**: Vertex y Pixel shaders básicos

### 🚧 En Desarrollo
- **Integración Assimp**: Carga de archivos .x y otros formatos 3D
- **Sistema de Animaciones**: Skeletal animation y skinning
- **Gestión de Texturas**: Carga desde archivos de imagen
- **Scene Manager**: Sistema de escena con entities

### 📋 Por Implementar
- **Input Manager**: Sistema de entrada avanzado
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
│   ├── Animation/      # Sistema de animaciones (futuro)
│   ├── Scene/          # Gestión de escena (futuro)
│   ├── Input/          # Sistema de entrada (futuro)
│   └── Math/           # Matemáticas 3D
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

- **WASD**: Movimiento de cámara
- **ESC**: Salir
- **Space**: Log de información
- **R**: Reset rotación
- **C**: Crear nuevo cubo
- **F11**: Toggle fullscreen (próximamente)

## Arquitectura

### Patrón Singleton
El motor usa el patrón Singleton para managers principales:
- `Engine`: Coordinador principal
- `Logger`: Sistema de logging
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

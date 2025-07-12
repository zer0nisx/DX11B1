# Assets/Textures

Este directorio contiene las texturas utilizadas por el motor DX11B1.

## Formatos Soportados

### Implementación Completa ✅
- **PNG** - Imágenes con transparencia
- **JPG/JPEG** - Imágenes comprimidas
- **BMP** - Imágenes bitmap
- **TGA** - Imágenes Targa
- **DDS** - Texturas DirectDraw Surface (optimizadas para DirectX)

## Texturas Procedurales Disponibles

El motor puede generar texturas procedurales en tiempo real:

### Checkerboard
```cpp
texture->CreateCheckerboard(512, 512, device, 0xFFFFFFFF, 0xFF000000, 32);
```

### Color Sólido
```cpp
texture->CreateSolidColor(256, 256, device, 0xFF0080FF);
```

## Uso en el Motor

```cpp
// Cargar texture mediante TextureManager
auto& textureManager = TextureManager::GetInstance();
auto texture = textureManager.LoadTexture(L"Assets/Textures/brick.png", device);

// Usar en material
material->SetDiffuseTexture(texture);
```

## TextureManager Features

✅ **Cache automático** - Las texturas se cargan una vez y se reutilizan
✅ **Gestión de memoria** - Liberación automática con weak_ptr
✅ **Sampler states** - Configuración flexible de filtrado
✅ **Render targets** - Soporte para texturas de renderizado
✅ **Depth buffers** - Texturas de profundidad con shader access

## Ejemplos de Uso

### Texturas de Prueba
Para testing rápido, el motor incluye texturas procedurales:
- **Checkerboard** - Para UV mapping testing
- **Color sólido** - Para materials básicos
- **Gradientes** - Para efectos visuales

### Integración con Assimp
Las texturas se cargan automáticamente desde archivos de modelo:
```cpp
// El AssimpLoader busca texturas en:
// - Ruta relativa al archivo .x
// - Assets/Textures/
// - Directorio actual
```

---
**Status**: ✅ Sistema de texturas completo y funcional

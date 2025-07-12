# Assets/Models

Este directorio contiene los modelos 3D utilizados por el motor DX11B1.

## Formatos Soportados (via Assimp)

### Implementación Completa ✅
- **.x** - DirectX mesh files (Microsoft)
- **.fbx** - Autodesk FBX
- **.obj** - Wavefront OBJ
- **.dae** - Collada
- **.3ds** - 3D Studio Max
- **.blend** - Blender (limitado)
- **+50 formatos más** via Assimp

## Características del AssimpLoader

### ✅ Funcionalidades Implementadas
- **Geometría estática** - Vertices, normales, UV coordinates
- **Múltiples submeshes** - Modelos complejos con varias partes
- **Materiales automáticos** - Extracción de propiedades y texturas
- **Skeletal animation** - Huesos y weights (hasta 4 por vértice)
- **Optimizaciones** - Post-processing para rendimiento

### 🎯 Especialización .x Files
El motor está optimizado específicamente para archivos DirectX .x:
```cpp
// Configuración optimizada para .x
aiProcess_Triangulate |
aiProcess_FlipUVs |
aiProcess_CalcTangentSpace |
aiProcess_LimitBoneWeights |
aiProcess_OptimizeMeshes
```

## Uso en el Motor

### Carga de Modelos
```cpp
AssimpLoader loader;
std::shared_ptr<Mesh> mesh;
bool success = loader.LoadMesh("Assets/Models/character.x", mesh, renderer);
```

### Modelos Animados
```cpp
if (loader.HasAnimations()) {
    auto bones = loader.GetBones();
    auto animations = loader.GetAnimations();
    // Setup animation system
}
```

## Estructura de Datos

### Bone Information
```cpp
struct BoneInfo {
    std::string name;
    DirectX::XMMATRIX offsetMatrix;
    int parentIndex;
};
```

### Animation Data
```cpp
struct AnimationData {
    std::string name;
    float duration;
    float ticksPerSecond;
};
```

## Testing Assets

### Modelos Recomendados para Pruebas

1. **basic_cube.x** - Cubo simple con UV mapping
2. **animated_character.x** - Personaje con animación básica
3. **multi_mesh_scene.x** - Escena con múltiples objetos
4. **rigged_model.x** - Modelo con skeleton completo

### Texturas Asociadas
Los modelos pueden referenciar texturas en:
- Mismo directorio que el modelo
- `../Textures/` relativo al modelo
- `Assets/Textures/` absoluto

## Integration Pipeline

### 1. File Loading
```
.x file → Assimp → Scene Graph → Processing
```

### 2. Data Extraction
```
Vertices → Normals → UVs → Bones → Materials → Textures
```

### 3. DirectX Integration
```
Vertex Buffers → Index Buffers → Constant Buffers → Shaders
```

### 4. Animation Setup
```
Bone Hierarchy → Keyframes → Interpolation → Skinning
```

## Performance Considerations

### ✅ Optimizaciones Implementadas
- **Vertex cache optimization** - Mejor uso de cache GPU
- **Mesh optimization** - Reducción de draw calls
- **Bone limit** - Máximo 4 weights por vértice
- **Triangle optimization** - Solo triángulos (no quads/ngons)

### 📊 Métricas Target
- **Vertices**: < 50,000 por mesh para 60 FPS
- **Triangles**: < 100,000 por escena
- **Bones**: < 100 por skeleton
- **Textures**: < 2048x2048 para mobile compatibility

---
**Status**: ✅ AssimpLoader completo - Soporte .x files y animaciones

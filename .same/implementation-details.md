# Detalles de Implementación Técnica

## 1. Core Engine Architecture

### Engine Class (Singleton)
```cpp
class Engine {
public:
    static Engine& GetInstance();
    bool Initialize(HINSTANCE hInstance, int width, int height);
    void Run();
    void Shutdown();

private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<D3D11Renderer> m_renderer;
    std::unique_ptr<MeshManager> m_meshManager;
    std::unique_ptr<AnimationManager> m_animManager;
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<Timer> m_timer;

    bool m_isRunning;
};
```

### Timer System
```cpp
class Timer {
public:
    void Start();
    void Update();
    float GetDeltaTime() const { return m_deltaTime; }
    float GetTotalTime() const { return m_totalTime; }
    int GetFPS() const { return m_fps; }

private:
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_startTime;
    LARGE_INTEGER m_currentTime;
    LARGE_INTEGER m_lastTime;
    float m_deltaTime;
    float m_totalTime;
    int m_fps;
    int m_frameCount;
    float m_fpsTimer;
};
```

## 2. DirectX 11 Renderer

### D3D11Renderer Class
```cpp
class D3D11Renderer {
public:
    bool Initialize(HWND hwnd, int width, int height);
    void Clear(float r, float g, float b, float a);
    void Present();
    void Shutdown();

    // Resource creation
    ID3D11Buffer* CreateVertexBuffer(const void* data, UINT size);
    ID3D11Buffer* CreateIndexBuffer(const UINT* indices, UINT count);
    ID3D11Buffer* CreateConstantBuffer(UINT size);

    // Shader management
    bool LoadVertexShader(const std::wstring& filename, ID3D11VertexShader** shader, ID3D11InputLayout** layout);
    bool LoadPixelShader(const std::wstring& filename, ID3D11PixelShader** shader);

    // Getters
    ID3D11Device* GetDevice() const { return m_device; }
    ID3D11DeviceContext* GetContext() const { return m_context; }

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;
    ID3D11DepthStencilView* m_depthStencilView;
    ID3D11Texture2D* m_depthStencilBuffer;

    D3D11_VIEWPORT m_viewport;
    int m_screenWidth;
    int m_screenHeight;
};
```

## 3. Mesh System con Assimp

### Vertex Structures
```cpp
struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
};

struct SkinnedVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
    DirectX::XMFLOAT4 boneWeights;
    DirectX::XMUINT4 boneIndices;
};
```

### Mesh Class
```cpp
class Mesh {
public:
    bool LoadFromFile(const std::string& filename);
    void Render(ID3D11DeviceContext* context);

    // Animation support
    bool IsAnimated() const { return m_isAnimated; }
    const std::vector<Bone>& GetBones() const { return m_bones; }

private:
    std::vector<Vertex> m_vertices;
    std::vector<UINT> m_indices;
    std::vector<Material> m_materials;
    std::vector<Bone> m_bones;

    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;

    bool m_isAnimated;
    UINT m_vertexCount;
    UINT m_indexCount;
};
```

### AssimpLoader Class
```cpp
class AssimpLoader {
public:
    static bool LoadMesh(const std::string& filename, Mesh& outMesh);

private:
    static void ProcessNode(aiNode* node, const aiScene* scene, Mesh& mesh);
    static void ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<UINT>& indices);
    static void ProcessAnimations(const aiScene* scene, Mesh& mesh);
    static void ProcessBones(aiMesh* mesh, std::vector<Bone>& bones);

    static Material LoadMaterial(aiMaterial* mat);
    static DirectX::XMMATRIX ConvertMatrix(const aiMatrix4x4& matrix);
};
```

## 4. Animation System

### Bone Structure
```cpp
struct Bone {
    std::string name;
    int parentIndex;
    DirectX::XMMATRIX offsetMatrix;
    DirectX::XMMATRIX finalTransform;

    std::vector<int> children;
};

struct Keyframe {
    float time;
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR rotation;
    DirectX::XMVECTOR scale;
};

struct AnimationChannel {
    std::string boneName;
    std::vector<Keyframe> keyframes;
};
```

### AnimationClip Class
```cpp
class AnimationClip {
public:
    bool LoadFromAssimp(const aiAnimation* animation);
    void Sample(float time, std::vector<DirectX::XMMATRIX>& boneTransforms, const std::vector<Bone>& skeleton);

    float GetDuration() const { return m_duration; }
    float GetTicksPerSecond() const { return m_ticksPerSecond; }

private:
    std::string m_name;
    float m_duration;
    float m_ticksPerSecond;
    std::vector<AnimationChannel> m_channels;

    DirectX::XMMATRIX InterpolateTransform(const AnimationChannel& channel, float time);
    int FindKeyframe(const std::vector<Keyframe>& keyframes, float time);
};
```

### AnimationManager Class
```cpp
class AnimationManager {
public:
    void Update(float deltaTime);
    void PlayAnimation(const std::string& meshName, const std::string& animationName, bool loop = true);
    void StopAnimation(const std::string& meshName);

    void RegisterAnimatedMesh(const std::string& name, std::shared_ptr<Mesh> mesh);

private:
    struct AnimationState {
        std::shared_ptr<AnimationClip> currentClip;
        float currentTime;
        bool isPlaying;
        bool isLooping;
        std::vector<DirectX::XMMATRIX> boneTransforms;
    };

    std::unordered_map<std::string, AnimationState> m_animationStates;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_animatedMeshes;
};
```

## 5. Shaders HLSL

### Vertex Shader (VertexShader.hlsl)
```hlsl
cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};

struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 worldPos : WORLD_POSITION;
};

VertexOutput main(VertexInput input) {
    VertexOutput output;

    float4 worldPos = mul(float4(input.position, 1.0f), World);
    float4 viewPos = mul(worldPos, View);
    output.position = mul(viewPos, Projection);

    output.normal = normalize(mul(input.normal, (float3x3)World));
    output.texCoord = input.texCoord;
    output.worldPos = worldPos.xyz;

    return output;
}
```

### Skinned Vertex Shader (SkinnedVertexShader.hlsl)
```hlsl
cbuffer ConstantBuffer : register(b0) {
    matrix World;
    matrix View;
    matrix Projection;
};

cbuffer BoneBuffer : register(b1) {
    matrix BoneTransforms[100]; // Max 100 bones
};

struct SkinnedVertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 boneWeights : BLENDWEIGHT;
    uint4 boneIndices : BLENDINDICES;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 worldPos : WORLD_POSITION;
};

VertexOutput main(SkinnedVertexInput input) {
    VertexOutput output;

    // Skinning calculation
    float4 skinnedPos = float4(0, 0, 0, 0);
    float3 skinnedNormal = float3(0, 0, 0);

    for(int i = 0; i < 4; i++) {
        if(input.boneWeights[i] > 0.0f) {
            matrix boneTransform = BoneTransforms[input.boneIndices[i]];
            skinnedPos += input.boneWeights[i] * mul(float4(input.position, 1.0f), boneTransform);
            skinnedNormal += input.boneWeights[i] * mul(input.normal, (float3x3)boneTransform);
        }
    }

    float4 worldPos = mul(skinnedPos, World);
    float4 viewPos = mul(worldPos, View);
    output.position = mul(viewPos, Projection);

    output.normal = normalize(mul(skinnedNormal, (float3x3)World));
    output.texCoord = input.texCoord;
    output.worldPos = worldPos.xyz;

    return output;
}
```

## 6. Resource Management

### ResourceManager Class
```cpp
template<typename T>
class ResourceManager {
public:
    std::shared_ptr<T> Load(const std::string& filename);
    void Unload(const std::string& filename);
    void UnloadAll();

    std::shared_ptr<T> Get(const std::string& filename);
    bool IsLoaded(const std::string& filename) const;

private:
    std::unordered_map<std::string, std::weak_ptr<T>> m_resources;

    virtual std::shared_ptr<T> LoadResource(const std::string& filename) = 0;
};

// Especializaciones
using TextureManager = ResourceManager<Texture>;
using MeshManager = ResourceManager<Mesh>;
using ShaderManager = ResourceManager<Shader>;
```

## 7. Configuración del Proyecto

### CMakeLists.txt Básico
```cmake
cmake_minimum_required(VERSION 3.16)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)

# Buscar DirectX
find_package(directx REQUIRED)

# Buscar Assimp
find_package(assimp REQUIRED)

# Archivos fuente
file(GLOB_RECURSE SOURCES "Source/*.cpp" "Source/*.h")
file(GLOB_RECURSE SHADERS "Shaders/*.hlsl")

# Crear ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Librerías
target_link_libraries(${PROJECT_NAME}
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    assimp::assimp
)

# Incluir directorios
target_include_directories(${PROJECT_NAME} PRIVATE "Source")

# Copiar shaders
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders)
```

## 8. Milestones de Testing

### Milestone 1: Renderizado Básico
- Renderizar un triángulo coloreado
- Aplicar transformaciones básicas
- Cámara funcional

### Milestone 2: Mesh Loading
- Cargar un cubo desde archivo .x
- Aplicar textura básica
- Múltiples meshes en escena

### Milestone 3: Animación Básica
- Cargar modelo animado simple
- Reproducir una animación
- Transiciones entre frames

### Milestone 4: Sistema Completo
- Escena con múltiples objetos animados
- Diferentes tipos de animaciones
- Controles de usuario funcionales

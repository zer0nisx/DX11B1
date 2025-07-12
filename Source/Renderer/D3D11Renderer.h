#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>
#include <memory>
#include "../Math/Matrix4.h"
#include "../Math/Vector3.h"
#include "Light.h"
#include "ShadowMap.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace GameEngine {
namespace Renderer {

using Microsoft::WRL::ComPtr;

struct ConstantBuffer {
    DirectX::XMMATRIX World;
    DirectX::XMMATRIX View;
    DirectX::XMMATRIX Projection;
};

struct BoneBuffer {
    DirectX::XMMATRIX BoneTransforms[100]; // Max 100 bones
};

// Light buffer for shaders
struct LightBuffer {
    DirectX::XMFLOAT4 lightDirection;    // w = light type
    DirectX::XMFLOAT4 lightColor;        // w = intensity
    DirectX::XMFLOAT4 lightPosition;     // w = range
    DirectX::XMFLOAT4 lightParams;       // x = inner cone, y = outer cone, z = shadow bias, w = shadow strength
    DirectX::XMFLOAT4X4 lightSpaceMatrix;
    DirectX::XMFLOAT4 ambientLight;      // w = ambient intensity
};

class D3D11Renderer {
public:
    D3D11Renderer();
    ~D3D11Renderer();

    bool Initialize(HWND hwnd, int width, int height, bool fullscreen = false);
    void Shutdown();

    void BeginFrame(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
    void EndFrame();
    void Resize(int width, int height);

    // Resource creation
    ComPtr<ID3D11Buffer> CreateVertexBuffer(const void* data, UINT size, bool dynamic = false);
    ComPtr<ID3D11Buffer> CreateIndexBuffer(const UINT* indices, UINT count);
    ComPtr<ID3D11Buffer> CreateConstantBuffer(UINT size);

    // Shader management
    bool LoadVertexShader(const std::wstring& filename, ComPtr<ID3D11VertexShader>& shader,
                         ComPtr<ID3D11InputLayout>& layout, const D3D11_INPUT_ELEMENT_DESC* elements, UINT elementCount);
    bool LoadPixelShader(const std::wstring& filename, ComPtr<ID3D11PixelShader>& shader);

    // Texture management
    ComPtr<ID3D11ShaderResourceView> LoadTexture(const std::wstring& filename);
    ComPtr<ID3D11SamplerState> CreateSamplerState();

    // State management
    void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
    void SetVertexBuffer(ID3D11Buffer* buffer, UINT stride, UINT offset = 0);
    void SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format = DXGI_FORMAT_R32_UINT);
    void SetVertexShader(ID3D11VertexShader* shader, ID3D11InputLayout* layout);
    void SetPixelShader(ID3D11PixelShader* shader);
    void SetConstantBuffer(ID3D11Buffer* buffer, UINT slot, bool vertex = true, bool pixel = true);
    void SetTexture(ID3D11ShaderResourceView* texture, UINT slot);
    void SetSampler(ID3D11SamplerState* sampler, UINT slot);

    // Drawing
    void DrawIndexed(UINT indexCount, UINT startIndex = 0, INT baseVertex = 0);
    void Draw(UINT vertexCount, UINT startVertex = 0);

    // Matrix operations
    void UpdateConstantBuffer(const Math::Matrix4& world, const Math::Matrix4& view, const Math::Matrix4& projection);
    void UpdateBoneBuffer(const DirectX::XMMATRIX* boneTransforms, UINT boneCount);

    // ✨ NEW: Light and Shadow system integration
    void UpdateLightBuffer(const LightManager& lightManager, const Math::Vector3& cameraPosition);
    void SetLightBuffer(ID3D11Buffer* lightBuffer, UINT slot = 1);

    // Shadow rendering
    void BeginShadowPass();
    void EndShadowPass();
    void SetShadowMap(ID3D11ShaderResourceView* shadowMap, UINT slot = 1);

    // Light management
    LightManager& GetLightManager() { return *m_lightManager; }
    ShadowMapManager& GetShadowMapManager() { return *m_shadowMapManager; }

    // Getters
    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }
    int GetWidth() const { return m_screenWidth; }
    int GetHeight() const { return m_screenHeight; }
    bool IsInitialized() const { return m_initialized; }

private:
    // Core DirectX objects
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11Texture2D> m_depthStencilBuffer;

    // States
    ComPtr<ID3D11RasterizerState> m_rasterizerState;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    ComPtr<ID3D11BlendState> m_blendState;
    ComPtr<ID3D11SamplerState> m_defaultSampler;

    // Constant buffers
    ComPtr<ID3D11Buffer> m_matrixBuffer;
    ComPtr<ID3D11Buffer> m_boneBuffer;
    ComPtr<ID3D11Buffer> m_lightBuffer;

    // ✨ NEW: Light and Shadow managers
    std::unique_ptr<LightManager> m_lightManager;
    std::unique_ptr<ShadowMapManager> m_shadowMapManager;

    // Viewport and screen info
    D3D11_VIEWPORT m_viewport;
    int m_screenWidth;
    int m_screenHeight;
    bool m_initialized;
    bool m_vsyncEnabled;

    // Helper methods
    bool CreateDeviceAndSwapChain(HWND hwnd, bool fullscreen);
    bool CreateRenderTargetView();
    bool CreateDepthStencilBuffer();
    bool CreateStates();
    void SetDefaultRenderStates();
};

} // namespace Renderer
} // namespace GameEngine

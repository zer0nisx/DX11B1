#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

using Microsoft::WRL::ComPtr;

namespace GameEngine {
namespace Renderer {

// Forward declarations
class Light;
class DirectionalLight;
class PointLight;
class SpotLight;

// Shadow map types
enum class ShadowMapType {
    Simple2D,           // Basic 2D shadow map
    Cascade,            // Cascaded shadow maps for directional lights
    Cube,               // Cube shadow map for point lights
    Array               // Array of shadow maps
};

// Shadow map filtering
enum class ShadowFilter {
    None,               // No filtering (hard shadows)
    PCF,                // Percentage Closer Filtering
    PCSS,               // Percentage Closer Soft Shadows
    VSM                 // Variance Shadow Maps
};

// Base shadow map class
class ShadowMap {
public:
    ShadowMap(ID3D11Device* device, ShadowMapType type, int width, int height);
    virtual ~ShadowMap();

    // Properties
    ShadowMapType GetType() const { return m_type; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

    ShadowFilter GetFilter() const { return m_filter; }
    void SetFilter(ShadowFilter filter) { m_filter = filter; }

    // Render targets
    ID3D11DepthStencilView* GetDepthStencilView() const { return m_depthStencilView.Get(); }
    ID3D11ShaderResourceView* GetShaderResourceView() const { return m_shaderResourceView.Get(); }

    // Viewport
    const D3D11_VIEWPORT& GetViewport() const { return m_viewport; }

    // Clear shadow map
    void Clear(ID3D11DeviceContext* context, float clearValue = 1.0f);

    // Bind for rendering
    void BindForRendering(ID3D11DeviceContext* context);
    void BindForSampling(ID3D11DeviceContext* context, UINT slot);

protected:
    bool CreateShadowMap(ID3D11Device* device);

    ShadowMapType m_type;
    ShadowFilter m_filter;
    int m_width;
    int m_height;

    ComPtr<ID3D11Texture2D> m_shadowTexture;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    D3D11_VIEWPORT m_viewport;
};

// Simple 2D shadow map (for directional and spot lights)
class ShadowMap2D : public ShadowMap {
public:
    ShadowMap2D(ID3D11Device* device, int width = 1024, int height = 1024);
    virtual ~ShadowMap2D() = default;
};

// Cascade shadow map (for directional lights)
class CascadeShadowMap : public ShadowMap {
public:
    CascadeShadowMap(ID3D11Device* device, int cascadeCount = 3, int width = 1024, int height = 1024);
    virtual ~CascadeShadowMap() = default;

    int GetCascadeCount() const { return m_cascadeCount; }

    // Get depth stencil view for specific cascade
    ID3D11DepthStencilView* GetCascadeDepthStencilView(int cascade) const;

    // Get shader resource view for all cascades
    ID3D11ShaderResourceView* GetCascadeArraySRV() const { return m_cascadeArraySRV.Get(); }

    // Bind specific cascade for rendering
    void BindCascadeForRendering(ID3D11DeviceContext* context, int cascade);

private:
    bool CreateCascadeShadowMap(ID3D11Device* device);

    int m_cascadeCount;
    ComPtr<ID3D11Texture2D> m_cascadeTexture;
    std::vector<ComPtr<ID3D11DepthStencilView>> m_cascadeDepthStencilViews;
    ComPtr<ID3D11ShaderResourceView> m_cascadeArraySRV;
};

// Cube shadow map (for point lights)
class CubeShadowMap : public ShadowMap {
public:
    CubeShadowMap(ID3D11Device* device, int size = 512);
    virtual ~CubeShadowMap() = default;

    // Get depth stencil view for specific face
    ID3D11DepthStencilView* GetFaceDepthStencilView(int face) const;

    // Get shader resource view for cube map
    ID3D11ShaderResourceView* GetCubeSRV() const { return m_cubeSRV.Get(); }

    // Bind specific face for rendering
    void BindFaceForRendering(ID3D11DeviceContext* context, int face);

private:
    bool CreateCubeShadowMap(ID3D11Device* device);

    ComPtr<ID3D11Texture2D> m_cubeTexture;
    std::vector<ComPtr<ID3D11DepthStencilView>> m_faceDepthStencilViews;
    ComPtr<ID3D11ShaderResourceView> m_cubeSRV;
};

// Shadow map manager
class ShadowMapManager {
public:
    ShadowMapManager(ID3D11Device* device);
    ~ShadowMapManager();

    // Shadow map creation
    std::shared_ptr<ShadowMap2D> CreateShadowMap2D(int width = 1024, int height = 1024);
    std::shared_ptr<CascadeShadowMap> CreateCascadeShadowMap(int cascadeCount = 3, int width = 1024, int height = 1024);
    std::shared_ptr<CubeShadowMap> CreateCubeShadowMap(int size = 512);

    // Shadow rendering
    void RenderShadowMap(ID3D11DeviceContext* context, Light* light, ShadowMap* shadowMap,
                        const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback);

    void RenderDirectionalShadow(ID3D11DeviceContext* context, DirectionalLight* light,
                                CascadeShadowMap* shadowMap, const DirectX::XMMATRIX& cameraView,
                                const DirectX::XMMATRIX& cameraProjection,
                                const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback);

    void RenderPointShadow(ID3D11DeviceContext* context, PointLight* light, CubeShadowMap* shadowMap,
                          const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback);

    void RenderSpotShadow(ID3D11DeviceContext* context, SpotLight* light, ShadowMap2D* shadowMap,
                         const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback);

    // Global shadow settings
    void SetShadowBias(float bias) { m_shadowBias = bias; }
    float GetShadowBias() const { return m_shadowBias; }

    void SetShadowNormalBias(float bias) { m_shadowNormalBias = bias; }
    float GetShadowNormalBias() const { return m_shadowNormalBias; }

    // Statistics
    size_t GetShadowMapCount() const { return m_shadowMaps.size(); }

private:
    ID3D11Device* m_device;
    std::vector<std::shared_ptr<ShadowMap>> m_shadowMaps;

    // Shadow settings
    float m_shadowBias;
    float m_shadowNormalBias;

    // Helper methods
    void SetShadowRenderState(ID3D11DeviceContext* context);
    void RestoreRenderState(ID3D11DeviceContext* context);

    // Saved render state
    ComPtr<ID3D11RenderTargetView> m_savedRenderTarget;
    ComPtr<ID3D11DepthStencilView> m_savedDepthStencil;
    ComPtr<ID3D11RasterizerState> m_savedRasterizerState;
    D3D11_VIEWPORT m_savedViewport;
};

// Shadow map constants for shaders
struct ShadowConstants {
    DirectX::XMFLOAT4X4 lightSpaceMatrix;
    DirectX::XMFLOAT4 shadowParams; // x = bias, y = normal bias, z = filter type, w = cascade count
    DirectX::XMFLOAT4 cascadeDistances; // For cascade shadow maps
    DirectX::XMFLOAT4 lightPosition; // For point light shadows
};

} // namespace Renderer
} // namespace GameEngine

#include "ShadowMap.h"
#include "Light.h"
#include "../Core/Logger.h"

namespace GameEngine {
namespace Renderer {

// Base ShadowMap implementation
ShadowMap::ShadowMap(ID3D11Device* device, ShadowMapType type, int width, int height)
    : m_type(type)
    , m_filter(ShadowFilter::PCF)
    , m_width(width)
    , m_height(height)
{
    CreateShadowMap(device);

    // Setup viewport
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
}

ShadowMap::~ShadowMap() {
}

bool ShadowMap::CreateShadowMap(ID3D11Device* device) {
    HRESULT hr;

    // Create depth texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = m_width;
    textureDesc.Height = m_height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_shadowTexture);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create shadow map texture");
        return false;
    }

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    hr = device->CreateDepthStencilView(m_shadowTexture.Get(), &dsvDesc, &m_depthStencilView);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create shadow map depth stencil view");
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(m_shadowTexture.Get(), &srvDesc, &m_shaderResourceView);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create shadow map shader resource view");
        return false;
    }

    LOG_DEBUG("Shadow map created successfully (" << m_width << "x" << m_height << ")");
    return true;
}

void ShadowMap::Clear(ID3D11DeviceContext* context, float clearValue) {
    context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, clearValue, 0);
}

void ShadowMap::BindForRendering(ID3D11DeviceContext* context) {
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, m_depthStencilView.Get());
    context->RSSetViewports(1, &m_viewport);
}

void ShadowMap::BindForSampling(ID3D11DeviceContext* context, UINT slot) {
    context->PSSetShaderResources(slot, 1, m_shaderResourceView.GetAddressOf());
}

// ShadowMap2D implementation
ShadowMap2D::ShadowMap2D(ID3D11Device* device, int width, int height)
    : ShadowMap(device, ShadowMapType::Simple2D, width, height)
{
}

// CascadeShadowMap implementation
CascadeShadowMap::CascadeShadowMap(ID3D11Device* device, int cascadeCount, int width, int height)
    : ShadowMap(device, ShadowMapType::Cascade, width, height)
    , m_cascadeCount(cascadeCount)
{
    CreateCascadeShadowMap(device);
}

bool CascadeShadowMap::CreateCascadeShadowMap(ID3D11Device* device) {
    HRESULT hr;

    // Create texture array for cascades
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = m_width;
    textureDesc.Height = m_height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = m_cascadeCount;
    textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_cascadeTexture);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create cascade shadow map texture");
        return false;
    }

    // Create depth stencil views for each cascade
    m_cascadeDepthStencilViews.resize(m_cascadeCount);
    for (int i = 0; i < m_cascadeCount; i++) {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.ArraySize = 1;

        hr = device->CreateDepthStencilView(m_cascadeTexture.Get(), &dsvDesc, &m_cascadeDepthStencilViews[i]);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create cascade shadow map depth stencil view " << i);
            return false;
        }
    }

    // Create shader resource view for entire array
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = m_cascadeCount;

    hr = device->CreateShaderResourceView(m_cascadeTexture.Get(), &srvDesc, &m_cascadeArraySRV);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create cascade shadow map shader resource view");
        return false;
    }

    LOG_DEBUG("Cascade shadow map created successfully (" << m_cascadeCount << " cascades, "
              << m_width << "x" << m_height << ")");
    return true;
}

ID3D11DepthStencilView* CascadeShadowMap::GetCascadeDepthStencilView(int cascade) const {
    if (cascade >= 0 && cascade < m_cascadeCount) {
        return m_cascadeDepthStencilViews[cascade].Get();
    }
    return nullptr;
}

void CascadeShadowMap::BindCascadeForRendering(ID3D11DeviceContext* context, int cascade) {
    if (cascade >= 0 && cascade < m_cascadeCount) {
        ID3D11RenderTargetView* nullRTV = nullptr;
        context->OMSetRenderTargets(1, &nullRTV, m_cascadeDepthStencilViews[cascade].Get());
        context->RSSetViewports(1, &m_viewport);
    }
}

// CubeShadowMap implementation
CubeShadowMap::CubeShadowMap(ID3D11Device* device, int size)
    : ShadowMap(device, ShadowMapType::Cube, size, size)
{
    CreateCubeShadowMap(device);
}

bool CubeShadowMap::CreateCubeShadowMap(ID3D11Device* device) {
    HRESULT hr;

    // Create cube texture
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = m_width;
    textureDesc.Height = m_height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 6; // 6 faces for cube map
    textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    hr = device->CreateTexture2D(&textureDesc, nullptr, &m_cubeTexture);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create cube shadow map texture");
        return false;
    }

    // Create depth stencil views for each face
    m_faceDepthStencilViews.resize(6);
    for (int i = 0; i < 6; i++) {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.MipSlice = 0;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.ArraySize = 1;

        hr = device->CreateDepthStencilView(m_cubeTexture.Get(), &dsvDesc, &m_faceDepthStencilViews[i]);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create cube shadow map depth stencil view " << i);
            return false;
        }
    }

    // Create shader resource view for cube map
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;
    srvDesc.TextureCube.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(m_cubeTexture.Get(), &srvDesc, &m_cubeSRV);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create cube shadow map shader resource view");
        return false;
    }

    LOG_DEBUG("Cube shadow map created successfully (" << m_width << "x" << m_height << ")");
    return true;
}

ID3D11DepthStencilView* CubeShadowMap::GetFaceDepthStencilView(int face) const {
    if (face >= 0 && face < 6) {
        return m_faceDepthStencilViews[face].Get();
    }
    return nullptr;
}

void CubeShadowMap::BindFaceForRendering(ID3D11DeviceContext* context, int face) {
    if (face >= 0 && face < 6) {
        ID3D11RenderTargetView* nullRTV = nullptr;
        context->OMSetRenderTargets(1, &nullRTV, m_faceDepthStencilViews[face].Get());
        context->RSSetViewports(1, &m_viewport);
    }
}

// ShadowMapManager implementation
ShadowMapManager::ShadowMapManager(ID3D11Device* device)
    : m_device(device)
    , m_shadowBias(0.001f)
    , m_shadowNormalBias(0.1f)
{
    LOG_INFO("ShadowMapManager initialized");
}

ShadowMapManager::~ShadowMapManager() {
    m_shadowMaps.clear();
    LOG_INFO("ShadowMapManager destroyed");
}

std::shared_ptr<ShadowMap2D> ShadowMapManager::CreateShadowMap2D(int width, int height) {
    auto shadowMap = std::make_shared<ShadowMap2D>(m_device, width, height);
    m_shadowMaps.push_back(shadowMap);
    return shadowMap;
}

std::shared_ptr<CascadeShadowMap> ShadowMapManager::CreateCascadeShadowMap(int cascadeCount, int width, int height) {
    auto shadowMap = std::make_shared<CascadeShadowMap>(m_device, cascadeCount, width, height);
    m_shadowMaps.push_back(shadowMap);
    return shadowMap;
}

std::shared_ptr<CubeShadowMap> ShadowMapManager::CreateCubeShadowMap(int size) {
    auto shadowMap = std::make_shared<CubeShadowMap>(m_device, size);
    m_shadowMaps.push_back(shadowMap);
    return shadowMap;
}

void ShadowMapManager::RenderShadowMap(ID3D11DeviceContext* context, Light* light, ShadowMap* shadowMap,
                                      const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback) {
    if (!light || !shadowMap || !renderCallback) return;

    // Save current render state
    SetShadowRenderState(context);

    // Clear shadow map
    shadowMap->Clear(context);

    // Bind shadow map for rendering
    shadowMap->BindForRendering(context);

    // Get light matrices
    DirectX::XMMATRIX viewMatrix = light->GetViewMatrix();
    DirectX::XMMATRIX projectionMatrix = light->GetProjectionMatrix();

    // Render shadow casters
    renderCallback(viewMatrix, projectionMatrix);

    // Restore render state
    RestoreRenderState(context);
}

void ShadowMapManager::RenderDirectionalShadow(ID3D11DeviceContext* context, DirectionalLight* light,
                                              CascadeShadowMap* shadowMap, const DirectX::XMMATRIX& cameraView,
                                              const DirectX::XMMATRIX& cameraProjection,
                                              const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback) {
    if (!light || !shadowMap || !renderCallback) return;

    // Save current render state
    SetShadowRenderState(context);

    // Get cascade view-projection matrices
    auto cascadeMatrices = light->GetCascadeViewProjectionMatrices(cameraView, cameraProjection);

    // Render each cascade
    for (int i = 0; i < shadowMap->GetCascadeCount() && i < static_cast<int>(cascadeMatrices.size()); i++) {
        // Clear cascade
        context->ClearDepthStencilView(shadowMap->GetCascadeDepthStencilView(i), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Bind cascade for rendering
        shadowMap->BindCascadeForRendering(context, i);

        // Extract view and projection from combined matrix (simplified)
        DirectX::XMMATRIX viewMatrix = light->GetViewMatrix();
        DirectX::XMMATRIX projMatrix = light->GetProjectionMatrix();

        // Render shadow casters for this cascade
        renderCallback(viewMatrix, projMatrix);
    }

    // Restore render state
    RestoreRenderState(context);
}

void ShadowMapManager::RenderPointShadow(ID3D11DeviceContext* context, PointLight* light, CubeShadowMap* shadowMap,
                                        const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback) {
    if (!light || !shadowMap || !renderCallback) return;

    // Save current render state
    SetShadowRenderState(context);

    // Get projection matrix (same for all faces)
    DirectX::XMMATRIX projectionMatrix = light->GetProjectionMatrix();

    // Get view matrices for all 6 faces
    auto viewMatrices = light->GetShadowViewMatrices();

    // Render each face
    for (int face = 0; face < 6; face++) {
        // Clear face
        context->ClearDepthStencilView(shadowMap->GetFaceDepthStencilView(face), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Bind face for rendering
        shadowMap->BindFaceForRendering(context, face);

        // Render shadow casters for this face
        renderCallback(viewMatrices[face], projectionMatrix);
    }

    // Restore render state
    RestoreRenderState(context);
}

void ShadowMapManager::RenderSpotShadow(ID3D11DeviceContext* context, SpotLight* light, ShadowMap2D* shadowMap,
                                       const std::function<void(const DirectX::XMMATRIX&, const DirectX::XMMATRIX&)>& renderCallback) {
    if (!light || !shadowMap || !renderCallback) return;

    // Save current render state
    SetShadowRenderState(context);

    // Clear shadow map
    shadowMap->Clear(context);

    // Bind shadow map for rendering
    shadowMap->BindForRendering(context);

    // Get light matrices
    DirectX::XMMATRIX viewMatrix = light->GetViewMatrix();
    DirectX::XMMATRIX projectionMatrix = light->GetProjectionMatrix();

    // Render shadow casters
    renderCallback(viewMatrix, projectionMatrix);

    // Restore render state
    RestoreRenderState(context);
}

void ShadowMapManager::SetShadowRenderState(ID3D11DeviceContext* context) {
    // Save current render targets
    context->OMGetRenderTargets(1, &m_savedRenderTarget, &m_savedDepthStencil);

    // Save current viewport
    UINT numViewports = 1;
    context->RSGetViewports(&numViewports, &m_savedViewport);

    // Save current rasterizer state
    context->RSGetState(&m_savedRasterizerState);

    // Set shadow-specific rasterizer state if needed
    // (depth bias, front face culling, etc.)
}

void ShadowMapManager::RestoreRenderState(ID3D11DeviceContext* context) {
    // Restore render targets
    context->OMSetRenderTargets(1, m_savedRenderTarget.GetAddressOf(), m_savedDepthStencil.Get());

    // Restore viewport
    context->RSSetViewports(1, &m_savedViewport);

    // Restore rasterizer state
    context->RSSetState(m_savedRasterizerState.Get());

    // Clear saved references
    m_savedRenderTarget.Reset();
    m_savedDepthStencil.Reset();
    m_savedRasterizerState.Reset();
}

} // namespace Renderer
} // namespace GameEngine

#include "D3D11Renderer.h"
#include "../Core/Logger.h"
#include <d3d11.h>
#include <DirectXTex.h>

namespace GameEngine {
namespace Renderer {

D3D11Renderer::D3D11Renderer()
    : m_screenWidth(0)
    , m_screenHeight(0)
    , m_initialized(false)
    , m_vsyncEnabled(true)
{
}

D3D11Renderer::~D3D11Renderer() {
    Shutdown();
}

bool D3D11Renderer::Initialize(HWND hwnd, int width, int height, bool fullscreen) {
    if (m_initialized) {
        LOG_WARNING("Renderer already initialized");
        return true;
    }

    m_screenWidth = width;
    m_screenHeight = height;

    LOG_INFO("Initializing D3D11 Renderer - " << width << "x" << height);

    // Create device and swap chain
    if (!CreateDeviceAndSwapChain(hwnd, fullscreen)) {
        LOG_ERROR("Failed to create D3D11 device and swap chain");
        return false;
    }

    // Create render target view
    if (!CreateRenderTargetView()) {
        LOG_ERROR("Failed to create render target view");
        return false;
    }

    // Create depth stencil buffer
    if (!CreateDepthStencilBuffer()) {
        LOG_ERROR("Failed to create depth stencil buffer");
        return false;
    }

    // Create viewport
    if (!CreateViewport()) {
        LOG_ERROR("Failed to create viewport");
        return false;
    }

    // Create default states
    if (!CreateDefaultStates()) {
        LOG_ERROR("Failed to create default states");
        return false;
    }

    // Create constant buffers
    m_constantBuffer = CreateConstantBuffer(sizeof(ConstantBuffer));
    if (!m_constantBuffer) {
        LOG_ERROR("Failed to create constant buffer");
        return false;
    }

    m_boneBuffer = CreateConstantBuffer(sizeof(BoneBuffer));
    if (!m_boneBuffer) {
        LOG_ERROR("Failed to create bone buffer");
        return false;
    }

    // Set render targets
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set default states
    m_context->RSSetState(m_rasterizerState.Get());
    m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_context->OMSetBlendState(m_blendState.Get(), blendFactor, 0xffffffff);

    m_initialized = true;
    LOG_INFO("D3D11 Renderer initialized successfully");

    return true;
}

void D3D11Renderer::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Clear state
    if (m_context) {
        m_context->ClearState();
    }

    m_initialized = false;
    LOG_INFO("D3D11 Renderer shutdown complete");
}

void D3D11Renderer::BeginFrame(float r, float g, float b, float a) {
    if (!m_initialized) {
        return;
    }

    // Clear render target
    float clearColor[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);

    // Clear depth stencil
    m_context->ClearDepthStencilView(m_depthStencilView.Get(),
                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                     1.0f, 0);
}

void D3D11Renderer::EndFrame() {
    if (!m_initialized) {
        return;
    }

    // Present the frame
    HRESULT hr = m_swapChain->Present(m_vsyncEnabled ? 1 : 0, 0);

    if (FAILED(hr)) {
        LOG_ERROR("Failed to present frame: " << std::hex << hr);
    }
}

void D3D11Renderer::Resize(int width, int height) {
    if (!m_initialized || (width == m_screenWidth && height == m_screenHeight)) {
        return;
    }

    LOG_INFO("Resizing renderer to " << width << "x" << height);

    // Clear render targets
    CleanupRenderTargets();

    // Resize swap chain
    HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to resize swap chain buffers");
        return;
    }

    m_screenWidth = width;
    m_screenHeight = height;

    // Recreate render targets
    CreateRenderTargetView();
    CreateDepthStencilBuffer();
    CreateViewport();

    // Reset render targets
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

ComPtr<ID3D11Buffer> D3D11Renderer::CreateVertexBuffer(const void* data, UINT size, bool dynamic) {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = size;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

    ComPtr<ID3D11Buffer> buffer;

    if (data) {
        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;

        HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &buffer);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create vertex buffer with data");
            return nullptr;
        }
    }
    else {
        HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &buffer);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create vertex buffer");
            return nullptr;
        }
    }

    return buffer;
}

ComPtr<ID3D11Buffer> D3D11Renderer::CreateIndexBuffer(const UINT* indices, UINT count) {
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(UINT) * count;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;

    ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &buffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create index buffer");
        return nullptr;
    }

    return buffer;
}

ComPtr<ID3D11Buffer> D3D11Renderer::CreateConstantBuffer(UINT size) {
    // Constant buffer size must be multiple of 16 bytes
    UINT alignedSize = (size + 15) & ~15;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = alignedSize;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Buffer> buffer;
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, nullptr, &buffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create constant buffer");
        return nullptr;
    }

    return buffer;
}

bool D3D11Renderer::LoadVertexShader(const std::wstring& filename, ComPtr<ID3D11VertexShader>& shader,
                                     ComPtr<ID3D11InputLayout>& layout, const D3D11_INPUT_ELEMENT_DESC* elements, UINT elementCount) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", "vs_5_0",
                                   D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0,
                                   &shaderBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            LOG_ERROR("Vertex shader compilation failed: " << (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    // Create vertex shader
    hr = m_device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                                     nullptr, &shader);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create vertex shader");
        return false;
    }

    // Create input layout
    hr = m_device->CreateInputLayout(elements, elementCount, shaderBlob->GetBufferPointer(),
                                    shaderBlob->GetBufferSize(), &layout);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create input layout");
        return false;
    }

    return true;
}

bool D3D11Renderer::LoadPixelShader(const std::wstring& filename, ComPtr<ID3D11PixelShader>& shader) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, "main", "ps_5_0",
                                   D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0,
                                   &shaderBlob, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            LOG_ERROR("Pixel shader compilation failed: " << (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    // Create pixel shader
    hr = m_device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
                                    nullptr, &shader);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create pixel shader");
        return false;
    }

    return true;
}

void D3D11Renderer::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) {
    m_context->IASetPrimitiveTopology(topology);
}

void D3D11Renderer::SetVertexBuffer(ID3D11Buffer* buffer, UINT stride, UINT offset) {
    m_context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
}

void D3D11Renderer::SetIndexBuffer(ID3D11Buffer* buffer, DXGI_FORMAT format) {
    m_context->IASetIndexBuffer(buffer, format, 0);
}

void D3D11Renderer::SetVertexShader(ID3D11VertexShader* shader, ID3D11InputLayout* layout) {
    m_context->VSSetShader(shader, nullptr, 0);
    m_context->IASetInputLayout(layout);
}

void D3D11Renderer::SetPixelShader(ID3D11PixelShader* shader) {
    m_context->PSSetShader(shader, nullptr, 0);
}

void D3D11Renderer::SetConstantBuffer(ID3D11Buffer* buffer, UINT slot, bool vertex, bool pixel) {
    if (vertex) {
        m_context->VSSetConstantBuffers(slot, 1, &buffer);
    }
    if (pixel) {
        m_context->PSSetConstantBuffers(slot, 1, &buffer);
    }
}

void D3D11Renderer::DrawIndexed(UINT indexCount, UINT startIndex, INT baseVertex) {
    m_context->DrawIndexed(indexCount, startIndex, baseVertex);
}

void D3D11Renderer::Draw(UINT vertexCount, UINT startVertex) {
    m_context->Draw(vertexCount, startVertex);
}

void D3D11Renderer::UpdateConstantBuffer(const Math::Matrix4& world, const Math::Matrix4& view, const Math::Matrix4& projection) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    if (SUCCEEDED(hr)) {
        ConstantBuffer* cb = (ConstantBuffer*)mappedResource.pData;
        cb->World = DirectX::XMMatrixTranspose(world.ToXMMATRIX());
        cb->View = DirectX::XMMatrixTranspose(view.ToXMMATRIX());
        cb->Projection = DirectX::XMMatrixTranspose(projection.ToXMMATRIX());

        m_context->Unmap(m_constantBuffer.Get(), 0);
    }
}

bool D3D11Renderer::CreateDeviceAndSwapChain(HWND hwnd, bool fullscreen) {
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = m_screenWidth;
    swapChainDesc.BufferDesc.Height = m_screenHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = !fullscreen;

    D3D_FEATURE_LEVEL featureLevel;
    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context
    );

    if (FAILED(hr)) {
        LOG_ERROR("Failed to create D3D11 device and swap chain");
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0) {
        LOG_WARNING("D3D11 feature level not supported, using: " << featureLevel);
    }

    return true;
}

bool D3D11Renderer::CreateRenderTargetView() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    return SUCCEEDED(hr);
}

bool D3D11Renderer::CreateDepthStencilBuffer() {
    D3D11_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = m_screenWidth;
    depthStencilDesc.Height = m_screenHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    HRESULT hr = m_device->CreateTexture2D(&depthStencilDesc, nullptr, &m_depthStencilBuffer);
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
    return SUCCEEDED(hr);
}

bool D3D11Renderer::CreateViewport() {
    m_viewport.Width = static_cast<float>(m_screenWidth);
    m_viewport.Height = static_cast<float>(m_screenHeight);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

    m_context->RSSetViewports(1, &m_viewport);
    return true;
}

bool D3D11Renderer::CreateDefaultStates() {
    // Rasterizer state
    D3D11_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;

    HRESULT hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);
    if (FAILED(hr)) {
        return false;
    }

    // Depth stencil state
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
    if (FAILED(hr)) {
        return false;
    }

    // Blend state
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_device->CreateBlendState(&blendDesc, &m_blendState);
    return SUCCEEDED(hr);
}

void D3D11Renderer::CleanupRenderTargets() {
    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_depthStencilBuffer.Reset();
}

} // namespace Renderer
} // namespace GameEngine

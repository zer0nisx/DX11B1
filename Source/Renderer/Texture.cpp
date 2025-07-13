#include "Texture.h"
#include "../Core/Logger.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <algorithm>
#include <codecvt>
#include <locale>

using namespace GameEngine::Renderer;
using namespace GameEngine::Core;

Texture::Texture()
    : m_width(0)
    , m_height(0)
    , m_format(DXGI_FORMAT_UNKNOWN)
    , m_isRenderTarget(false)
{
}

bool Texture::LoadFromFile(const std::wstring& filename, ID3D11Device* device) {
    if (!device) {
        Logger::GetInstance().LogError("Texture::LoadFromFile - Device is null");
        return false;
    }

    Release();

    std::wstring extension = filename.substr(filename.find_last_of(L".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    bool success = false;

    if (extension == L"dds") {
        success = LoadDDS(filename, device);
    } else {
        success = LoadWIC(filename, device);
    }

    if (success) {
        m_filename = filename;
        // Convert wstring to string properly
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string filenameStr = converter.to_bytes(filename);
        Logger::GetInstance().LogInfo("Texture loaded successfully: " + filenameStr);
    } else {
        // Convert wstring to string properly
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string filenameStr = converter.to_bytes(filename);
        Logger::GetInstance().LogError("Failed to load texture: " + filenameStr);
    }

    return success;
}

bool Texture::LoadFromMemory(const void* data, size_t dataSize, ID3D11Device* device) {
    if (!device || !data || dataSize == 0) {
        Logger::GetInstance().LogError("Texture::LoadFromMemory - Invalid parameters");
        return false;
    }

    Release();

    ComPtr<ID3D11Resource> resource;
    HRESULT hr = DirectX::CreateWICTextureFromMemory(
        device,
        static_cast<const uint8_t*>(data),
        dataSize,
        resource.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        hr = resource.As(&m_texture);
    }

    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);
        m_width = desc.Width;
        m_height = desc.Height;
        m_format = desc.Format;
        return true;
    }

    Logger::GetInstance().LogError("Failed to load texture from memory");
    return false;
}

bool Texture::CreateFromTexture2D(ID3D11Texture2D* texture, ID3D11Device* device) {
    if (!texture || !device) {
        Logger::GetInstance().LogError("Texture::CreateFromTexture2D - Invalid parameters");
        return false;
    }

    Release();

    m_texture = texture;

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    m_width = desc.Width;
    m_height = desc.Height;
    m_format = desc.Format;

    return CreateShaderResourceView(device);
}

bool Texture::CreateCheckerboard(int width, int height, ID3D11Device* device, UINT32 color1, UINT32 color2, int checkSize) {
    if (!device || width <= 0 || height <= 0 || checkSize <= 0) {
        Logger::GetInstance().LogError("Texture::CreateCheckerboard - Invalid parameters");
        return false;
    }

    Release();

    std::vector<UINT32> pixels(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            bool checker = ((x / checkSize) + (y / checkSize)) % 2 == 0;
            pixels[y * width + x] = checker ? color1 : color2;
        }
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = width * sizeof(UINT32);

    HRESULT hr = device->CreateTexture2D(&desc, &initData, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create checkerboard texture");
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = desc.Format;

    return CreateShaderResourceView(device);
}

bool Texture::CreateSolidColor(int width, int height, ID3D11Device* device, UINT32 color) {
    if (!device || width <= 0 || height <= 0) {
        Logger::GetInstance().LogError("Texture::CreateSolidColor - Invalid parameters");
        return false;
    }

    Release();

    std::vector<UINT32> pixels(width * height, color);

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = width * sizeof(UINT32);

    HRESULT hr = device->CreateTexture2D(&desc, &initData, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create solid color texture");
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = desc.Format;

    return CreateShaderResourceView(device);
}

bool Texture::CreateRenderTarget(int width, int height, DXGI_FORMAT format, ID3D11Device* device) {
    if (!device || width <= 0 || height <= 0) {
        Logger::GetInstance().LogError("Texture::CreateRenderTarget - Invalid parameters");
        return false;
    }

    Release();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create render target texture");
        return false;
    }

    // Create render target view
    hr = device->CreateRenderTargetView(m_texture.Get(), nullptr, m_renderTargetView.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create render target view");
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = format;
    m_isRenderTarget = true;

    return CreateShaderResourceView(device);
}

bool Texture::CreateDepthStencil(int width, int height, ID3D11Device* device) {
    if (!device || width <= 0 || height <= 0) {
        Logger::GetInstance().LogError("Texture::CreateDepthStencil - Invalid parameters");
        return false;
    }

    Release();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create depth stencil texture");
        return false;
    }

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    hr = device->CreateDepthStencilView(m_texture.Get(), &dsvDesc, m_depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create depth stencil view");
        return false;
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_shaderResourceView.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create depth stencil shader resource view");
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = desc.Format;

    return true;
}

void Texture::Release() {
    m_texture.Reset();
    m_shaderResourceView.Reset();
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();

    m_filename.clear();
    m_width = 0;
    m_height = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_isRenderTarget = false;
}

bool Texture::CreateShaderResourceView(ID3D11Device* device) {
    if (!device || !m_texture) {
        return false;
    }

    HRESULT hr = device->CreateShaderResourceView(m_texture.Get(), nullptr, m_shaderResourceView.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create shader resource view");
        return false;
    }

    return true;
}

bool Texture::LoadDDS(const std::wstring& filename, ID3D11Device* device) {
    ComPtr<ID3D11Resource> resource;
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        device,
        filename.c_str(),
        resource.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        hr = resource.As(&m_texture);
    }

    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);
        m_width = desc.Width;
        m_height = desc.Height;
        m_format = desc.Format;
        return true;
    }

    return false;
}

bool Texture::LoadWIC(const std::wstring& filename, ID3D11Device* device) {
    ComPtr<ID3D11Resource> resource;
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        device,
        filename.c_str(),
        resource.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        hr = resource.As(&m_texture);
    }

    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);
        m_width = desc.Width;
        m_height = desc.Height;
        m_format = desc.Format;
        return true;
    }

    return false;
}

// TextureManager implementation
TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::wstring& filename, ID3D11Device* device) {
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        } else {
            m_textures.erase(it);
        }
    }

    auto texture = std::make_shared<Texture>();
    if (texture->LoadFromFile(filename, device)) {
        m_textures[filename] = texture;
        return texture;
    }

    return nullptr;
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::wstring& filename) {
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        } else {
            m_textures.erase(it);
        }
    }
    return nullptr;
}

void TextureManager::UnloadTexture(const std::wstring& filename) {
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        m_textures.erase(it);
    }
}

void TextureManager::UnloadAll() {
    m_textures.clear();
    m_defaultSampler.Reset();
}

ComPtr<ID3D11SamplerState> TextureManager::CreateSamplerState(ID3D11Device* device,
    D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) {

    ComPtr<ID3D11SamplerState> sampler;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = filter;
    samplerDesc.AddressU = addressMode;
    samplerDesc.AddressV = addressMode;
    samplerDesc.AddressW = addressMode;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;

    HRESULT hr = device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());
    if (FAILED(hr)) {
        Logger::GetInstance().LogError("Failed to create sampler state");
        return nullptr;
    }

    return sampler;
}

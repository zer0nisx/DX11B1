#include "Texture.h"
#include "../Core/Logger.h"
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <algorithm>

#ifdef DIRECTXTK_ENABLED
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#endif

namespace GameEngine {
namespace Renderer {

// Texture Implementation
Texture::Texture()
    : m_width(0)
    , m_height(0)
    , m_format(DXGI_FORMAT_UNKNOWN)
    , m_isRenderTarget(false) {
}

bool Texture::LoadFromFile(const std::wstring& filename, ID3D11Device* device) {
    if (!device) {
        LOG_ERROR("Device is null in Texture::LoadFromFile");
        return false;
    }

    if (!std::filesystem::exists(filename)) {
        LOG_ERROR("Texture file not found: " << std::string(filename.begin(), filename.end()));
        return false;
    }

    Release();
    m_filename = filename;

    // Check file extension to determine loading method
    std::wstring extension = std::filesystem::path(filename).extension();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);

    bool success = false;
    if (extension == L".dds") {
        success = LoadDDS(filename, device);
    } else {
        success = LoadWIC(filename, device);
    }

    if (success) {
        LOG_INFO("Successfully loaded texture: " << std::string(filename.begin(), filename.end()));
    } else {
        LOG_ERROR("Failed to load texture: " << std::string(filename.begin(), filename.end()));
    }

    return success;
}

bool Texture::LoadFromMemory(const void* data, size_t dataSize, ID3D11Device* device) {
    if (!device || !data || dataSize == 0) {
        LOG_ERROR("Invalid parameters in Texture::LoadFromMemory");
        return false;
    }

    Release();

#ifdef DIRECTXTK_ENABLED
    HRESULT hr = DirectX::CreateWICTextureFromMemory(
        device,
        static_cast<const uint8_t*>(data),
        dataSize,
        m_texture.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        // Get texture description
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);
        m_width = desc.Width;
        m_height = desc.Height;
        m_format = desc.Format;

        LOG_INFO("Successfully loaded texture from memory");
        return true;
    }

    LOG_ERROR("Failed to load texture from memory. HRESULT: " << std::hex << hr);
    return false;
#else
    LOG_WARNING("DirectXTK not available - texture loading from memory not supported");
    return false;
#endif
}

bool Texture::CreateFromTexture2D(ID3D11Texture2D* texture, ID3D11Device* device) {
    if (!texture || !device) {
        LOG_ERROR("Invalid parameters in Texture::CreateFromTexture2D");
        return false;
    }

    Release();
    m_texture = texture;

    // Get texture description
    D3D11_TEXTURE2D_DESC desc;
    m_texture->GetDesc(&desc);
    m_width = desc.Width;
    m_height = desc.Height;
    m_format = desc.Format;

    return CreateShaderResourceView(device);
}

bool Texture::CreateCheckerboard(int width, int height, ID3D11Device* device,
                                UINT32 color1, UINT32 color2, int checkSize) {
    if (!device || width <= 0 || height <= 0 || checkSize <= 0) {
        LOG_ERROR("Invalid parameters in Texture::CreateCheckerboard");
        return false;
    }

    Release();

    // Create texture data
    std::vector<UINT32> textureData(width * height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int checkX = x / checkSize;
            int checkY = y / checkSize;
            bool isEven = (checkX + checkY) % 2 == 0;
            textureData[y * width + x] = isEven ? color1 : color2;
        }
    }

    // Create texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    // Create subresource data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = textureData.data();
    initData.SysMemPitch = width * sizeof(UINT32);
    initData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateTexture2D(&desc, &initData, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create checkerboard texture. HRESULT: " << std::hex << hr);
        return false;
    }

    if (!CreateShaderResourceView(device)) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

    LOG_INFO("Successfully created checkerboard texture: " << width << "x" << height);
    return true;
}

bool Texture::CreateSolidColor(int width, int height, ID3D11Device* device, UINT32 color) {
    if (!device || width <= 0 || height <= 0) {
        LOG_ERROR("Invalid parameters in Texture::CreateSolidColor");
        return false;
    }

    Release();

    // Create texture data
    std::vector<UINT32> textureData(width * height, color);

    // Create texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    // Create subresource data
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = textureData.data();
    initData.SysMemPitch = width * sizeof(UINT32);
    initData.SysMemSlicePitch = 0;

    HRESULT hr = device->CreateTexture2D(&desc, &initData, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create solid color texture. HRESULT: " << std::hex << hr);
        return false;
    }

    if (!CreateShaderResourceView(device)) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = DXGI_FORMAT_R8G8B8A8_UNORM;

    LOG_INFO("Successfully created solid color texture: " << width << "x" << height);
    return true;
}

bool Texture::CreateRenderTarget(int width, int height, DXGI_FORMAT format, ID3D11Device* device) {
    if (!device || width <= 0 || height <= 0) {
        LOG_ERROR("Invalid parameters in Texture::CreateRenderTarget");
        return false;
    }

    Release();

    // Create texture description
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create render target texture. HRESULT: " << std::hex << hr);
        return false;
    }

    // Create render target view
    hr = device->CreateRenderTargetView(m_texture.Get(), nullptr, m_renderTargetView.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create render target view. HRESULT: " << std::hex << hr);
        return false;
    }

    // Create shader resource view
    if (!CreateShaderResourceView(device)) {
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = format;
    m_isRenderTarget = true;

    LOG_INFO("Successfully created render target: " << width << "x" << height);
    return true;
}

bool Texture::CreateDepthStencil(int width, int height, ID3D11Device* device) {
    if (!device || width <= 0 || height <= 0) {
        LOG_ERROR("Invalid parameters in Texture::CreateDepthStencil");
        return false;
    }

    Release();

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil texture. HRESULT: " << std::hex << hr);
        return false;
    }

    // Create depth stencil view
    hr = device->CreateDepthStencilView(m_texture.Get(), nullptr, m_depthStencilView.GetAddressOf());
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil view. HRESULT: " << std::hex << hr);
        return false;
    }

    m_width = width;
    m_height = height;
    m_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    LOG_INFO("Successfully created depth stencil: " << width << "x" << height);
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
        LOG_ERROR("Failed to create shader resource view. HRESULT: " << std::hex << hr);
        return false;
    }

    return true;
}

bool Texture::LoadDDS(const std::wstring& filename, ID3D11Device* device) {
#ifdef DIRECTXTK_ENABLED
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        device,
        filename.c_str(),
        m_texture.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        // Get texture description
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);
        m_width = desc.Width;
        m_height = desc.Height;
        m_format = desc.Format;
        return true;
    }

    LOG_ERROR("Failed to load DDS texture. HRESULT: " << std::hex << hr);
    return false;
#else
    LOG_WARNING("DirectXTK not available - DDS texture loading not supported");
    return false;
#endif
}

bool Texture::LoadWIC(const std::wstring& filename, ID3D11Device* device) {
#ifdef DIRECTXTK_ENABLED
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        device,
        filename.c_str(),
        m_texture.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (SUCCEEDED(hr)) {
        // Get texture description
        D3D11_TEXTURE2D_DESC desc;
        m_texture->GetDesc(&desc);
        m_width = desc.Width;
        m_height = desc.Height;
        m_format = desc.Format;
        return true;
    }

    LOG_ERROR("Failed to load WIC texture. HRESULT: " << std::hex << hr);
    return false;
#else
    LOG_WARNING("DirectXTK not available - WIC texture loading not supported");
    return false;
#endif
}

// TextureManager Implementation
TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::LoadTexture(const std::wstring& filename, ID3D11Device* device) {
    // Check if texture is already loaded
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto existing = it->second.lock()) {
            LOG_INFO("Texture already loaded (from cache): " << std::string(filename.begin(), filename.end()));
            return existing;
        } else {
            // Remove expired weak_ptr
            m_textures.erase(it);
        }
    }

    // Create new texture
    auto texture = std::make_shared<Texture>();
    if (texture->LoadFromFile(filename, device)) {
        m_textures[filename] = texture;
        LOG_INFO("Texture loaded and cached: " << std::string(filename.begin(), filename.end()));
        return texture;
    }

    LOG_ERROR("Failed to load texture: " << std::string(filename.begin(), filename.end()));
    return nullptr;
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::wstring& filename) {
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        } else {
            // Remove expired weak_ptr
            m_textures.erase(it);
        }
    }
    return nullptr;
}

void TextureManager::UnloadTexture(const std::wstring& filename) {
    auto it = m_textures.find(filename);
    if (it != m_textures.end()) {
        m_textures.erase(it);
        LOG_INFO("Texture unloaded: " << std::string(filename.begin(), filename.end()));
    }
}

void TextureManager::UnloadAll() {
    size_t count = m_textures.size();
    m_textures.clear();
    m_defaultSampler.Reset();
    LOG_INFO("All textures unloaded. Count: " << count);
}

ComPtr<ID3D11SamplerState> TextureManager::CreateSamplerState(ID3D11Device* device,
    D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) {

    if (!device) {
        LOG_ERROR("Device is null in TextureManager::CreateSamplerState");
        return nullptr;
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = filter;
    samplerDesc.AddressU = addressMode;
    samplerDesc.AddressV = addressMode;
    samplerDesc.AddressW = addressMode;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ComPtr<ID3D11SamplerState> samplerState;
    HRESULT hr = device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

    if (FAILED(hr)) {
        LOG_ERROR("Failed to create sampler state. HRESULT: " << std::hex << hr);
        return nullptr;
    }

    return samplerState;
}

} // namespace Renderer
} // namespace GameEngine

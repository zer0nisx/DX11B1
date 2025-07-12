#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace GameEngine {
namespace Renderer {

using Microsoft::WRL::ComPtr;

class Texture {
public:
    Texture();
    ~Texture() = default;

    // Loading methods
    bool LoadFromFile(const std::wstring& filename, ID3D11Device* device);
    bool LoadFromMemory(const void* data, size_t dataSize, ID3D11Device* device);

    // Create from existing resource
    bool CreateFromTexture2D(ID3D11Texture2D* texture, ID3D11Device* device);

    // Create procedural textures
    bool CreateCheckerboard(int width, int height, ID3D11Device* device,
                           UINT32 color1 = 0xFFFFFFFF, UINT32 color2 = 0xFF000000, int checkSize = 32);
    bool CreateSolidColor(int width, int height, ID3D11Device* device, UINT32 color = 0xFFFFFFFF);

    // Render target creation
    bool CreateRenderTarget(int width, int height, DXGI_FORMAT format, ID3D11Device* device);
    bool CreateDepthStencil(int width, int height, ID3D11Device* device);

    // Getters
    ID3D11ShaderResourceView* GetSRV() const { return m_shaderResourceView.Get(); }
    ID3D11RenderTargetView* GetRTV() const { return m_renderTargetView.Get(); }
    ID3D11DepthStencilView* GetDSV() const { return m_depthStencilView.Get(); }
    ID3D11Texture2D* GetTexture() const { return m_texture.Get(); }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    DXGI_FORMAT GetFormat() const { return m_format; }
    bool IsLoaded() const { return m_texture != nullptr; }

    const std::wstring& GetFilename() const { return m_filename; }

    // Utility
    void Release();

private:
    bool CreateShaderResourceView(ID3D11Device* device);
    bool LoadDDS(const std::wstring& filename, ID3D11Device* device);
    bool LoadWIC(const std::wstring& filename, ID3D11Device* device); // For PNG, JPG, etc.

private:
    ComPtr<ID3D11Texture2D> m_texture;
    ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;

    std::wstring m_filename;
    int m_width;
    int m_height;
    DXGI_FORMAT m_format;
    bool m_isRenderTarget;
};

// Texture Manager for resource management
class TextureManager {
public:
    static TextureManager& GetInstance();

    std::shared_ptr<Texture> LoadTexture(const std::wstring& filename, ID3D11Device* device);
    std::shared_ptr<Texture> GetTexture(const std::wstring& filename);

    void UnloadTexture(const std::wstring& filename);
    void UnloadAll();

    // Create sampler states
    ComPtr<ID3D11SamplerState> CreateSamplerState(ID3D11Device* device,
        D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
        D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_WRAP);

private:
    TextureManager() = default;
    ~TextureManager() = default;

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

private:
    std::unordered_map<std::wstring, std::weak_ptr<Texture>> m_textures;
    ComPtr<ID3D11SamplerState> m_defaultSampler;
};

} // namespace Renderer
} // namespace GameEngine

// Convenience macro
#define TEXTURE_MANAGER GameEngine::Renderer::TextureManager::GetInstance()

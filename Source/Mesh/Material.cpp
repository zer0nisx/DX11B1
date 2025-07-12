#include "Material.h"
#include "../Core/Logger.h"
// TODO: Add texture loading library like DirectXTex or WIC

namespace GameEngine {
namespace Mesh {

ComPtr<ID3D11ShaderResourceView> Material::s_defaultTexture = nullptr;

Material::Material(const std::string& name)
    : m_name(name)
{
}

bool Material::LoadDiffuseTexture(ID3D11Device* device, const std::string& filename) {
    // TODO: Implement texture loading using DirectXTex or WIC
    // For now, we'll use the default texture
    if (!s_defaultTexture) {
        CreateDefaultTexture(device);
    }

    m_diffuseTexture = s_defaultTexture;
    LOG_INFO("Loaded diffuse texture: " << filename << " (using default texture for now)");
    return true;
}

bool Material::LoadNormalTexture(ID3D11Device* device, const std::string& filename) {
    // TODO: Implement texture loading
    LOG_INFO("Normal texture loading not implemented yet: " << filename);
    return false;
}

bool Material::LoadSpecularTexture(ID3D11Device* device, const std::string& filename) {
    // TODO: Implement texture loading
    LOG_INFO("Specular texture loading not implemented yet: " << filename);
    return false;
}

void Material::Apply(ID3D11DeviceContext* context, UINT textureSlot, UINT samplerSlot) {
    // Apply diffuse texture
    if (m_diffuseTexture) {
        ID3D11ShaderResourceView* textures[] = { m_diffuseTexture.Get() };
        context->PSSetShaderResources(textureSlot, 1, textures);
    }
    else if (s_defaultTexture) {
        ID3D11ShaderResourceView* textures[] = { s_defaultTexture.Get() };
        context->PSSetShaderResources(textureSlot, 1, textures);
    }

    // TODO: Apply other textures (normal, specular) to additional slots
    // TODO: Create and apply material constant buffer with properties
}

void Material::CreateDefaultTexture(ID3D11Device* device) {
    if (s_defaultTexture) {
        return;
    }

    // Create a simple 2x2 white texture
    const UINT textureWidth = 2;
    const UINT textureHeight = 2;
    const UINT pixelSize = 4; // RGBA

    UINT32 pixels[textureWidth * textureHeight];
    for (UINT i = 0; i < textureWidth * textureHeight; i++) {
        pixels[i] = 0xFFFFFFFF; // White pixel (RGBA)
    }

    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = textureWidth;
    textureDesc.Height = textureHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels;
    initData.SysMemPitch = textureWidth * pixelSize;
    initData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, &texture);

    if (SUCCEEDED(hr)) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;

        hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &s_defaultTexture);

        if (SUCCEEDED(hr)) {
            LOG_INFO("Created default white texture");
        }
        else {
            LOG_ERROR("Failed to create default texture shader resource view");
        }
    }
    else {
        LOG_ERROR("Failed to create default texture");
    }
}

} // namespace Mesh
} // namespace GameEngine

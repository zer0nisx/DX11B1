#pragma once
#include <string>
#include <memory>
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace GameEngine {
namespace Mesh {

using Microsoft::WRL::ComPtr;

struct MaterialProperties {
    DirectX::XMFLOAT3 ambient = {0.2f, 0.2f, 0.2f};
    DirectX::XMFLOAT3 diffuse = {0.8f, 0.8f, 0.8f};
    DirectX::XMFLOAT3 specular = {1.0f, 1.0f, 1.0f};
    float specularPower = 32.0f;
    float opacity = 1.0f;
};

class Material {
public:
    Material(const std::string& name = "DefaultMaterial");
    ~Material() = default;

    // Property setters
    void SetName(const std::string& name) { m_name = name; }
    void SetAmbient(const DirectX::XMFLOAT3& ambient) { m_properties.ambient = ambient; }
    void SetDiffuse(const DirectX::XMFLOAT3& diffuse) { m_properties.diffuse = diffuse; }
    void SetSpecular(const DirectX::XMFLOAT3& specular) { m_properties.specular = specular; }
    void SetSpecularPower(float power) { m_properties.specularPower = power; }
    void SetOpacity(float opacity) { m_properties.opacity = opacity; }

    // Texture management
    void SetDiffuseTexture(ComPtr<ID3D11ShaderResourceView> texture) { m_diffuseTexture = texture; }
    void SetNormalTexture(ComPtr<ID3D11ShaderResourceView> texture) { m_normalTexture = texture; }
    void SetSpecularTexture(ComPtr<ID3D11ShaderResourceView> texture) { m_specularTexture = texture; }

    bool LoadDiffuseTexture(ID3D11Device* device, const std::string& filename);
    bool LoadNormalTexture(ID3D11Device* device, const std::string& filename);
    bool LoadSpecularTexture(ID3D11Device* device, const std::string& filename);

    // Getters
    const std::string& GetName() const { return m_name; }
    const MaterialProperties& GetProperties() const { return m_properties; }

    ID3D11ShaderResourceView* GetDiffuseTexture() const { return m_diffuseTexture.Get(); }
    ID3D11ShaderResourceView* GetNormalTexture() const { return m_normalTexture.Get(); }
    ID3D11ShaderResourceView* GetSpecularTexture() const { return m_specularTexture.Get(); }

    bool HasDiffuseTexture() const { return m_diffuseTexture != nullptr; }
    bool HasNormalTexture() const { return m_normalTexture != nullptr; }
    bool HasSpecularTexture() const { return m_specularTexture != nullptr; }

    // Rendering
    void Apply(ID3D11DeviceContext* context, UINT textureSlot = 0, UINT samplerSlot = 0);

private:
    std::string m_name;
    MaterialProperties m_properties;

    ComPtr<ID3D11ShaderResourceView> m_diffuseTexture;
    ComPtr<ID3D11ShaderResourceView> m_normalTexture;
    ComPtr<ID3D11ShaderResourceView> m_specularTexture;

    // Default white texture for materials without textures
    static ComPtr<ID3D11ShaderResourceView> s_defaultTexture;
    static void CreateDefaultTexture(ID3D11Device* device);
};

} // namespace Mesh
} // namespace GameEngine

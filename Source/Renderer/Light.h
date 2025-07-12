#pragma once

#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace GameEngine {
namespace Renderer {

// Light types
enum class LightType {
    Directional = 0,
    Point = 1,
    Spot = 2,
    Area = 3
};

// Base light class
class Light {
public:
    Light(LightType type);
    virtual ~Light() = default;

    // Light properties
    LightType GetType() const { return m_type; }

    const DirectX::XMFLOAT3& GetColor() const { return m_color; }
    void SetColor(const DirectX::XMFLOAT3& color) { m_color = color; }
    void SetColor(float r, float g, float b) { m_color = DirectX::XMFLOAT3(r, g, b); }

    float GetIntensity() const { return m_intensity; }
    void SetIntensity(float intensity) { m_intensity = intensity; }

    bool IsEnabled() const { return m_enabled; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    bool IsCastingShadows() const { return m_castShadows; }
    void SetCastShadows(bool castShadows) { m_castShadows = castShadows; }

    // Shadow properties
    int GetShadowMapSize() const { return m_shadowMapSize; }
    void SetShadowMapSize(int size) { m_shadowMapSize = size; }

    float GetShadowBias() const { return m_shadowBias; }
    void SetShadowBias(float bias) { m_shadowBias = bias; }

    float GetShadowStrength() const { return m_shadowStrength; }
    void SetShadowStrength(float strength) { m_shadowStrength = strength; }

    // Virtual methods for specific light types
    virtual DirectX::XMMATRIX GetViewMatrix() const = 0;
    virtual DirectX::XMMATRIX GetProjectionMatrix() const = 0;
    virtual float GetRange() const { return 0.0f; }

protected:
    LightType m_type;
    DirectX::XMFLOAT3 m_color;
    float m_intensity;
    bool m_enabled;
    bool m_castShadows;

    // Shadow properties
    int m_shadowMapSize;
    float m_shadowBias;
    float m_shadowStrength;
};

// Directional light (sun/moon)
class DirectionalLight : public Light {
public:
    DirectionalLight();
    virtual ~DirectionalLight() = default;

    const DirectX::XMFLOAT3& GetDirection() const { return m_direction; }
    void SetDirection(const DirectX::XMFLOAT3& direction);
    void SetDirection(float x, float y, float z);

    // Cascade shadow map properties
    int GetCascadeCount() const { return m_cascadeCount; }
    void SetCascadeCount(int count) { m_cascadeCount = count; }

    const std::vector<float>& GetCascadeDistances() const { return m_cascadeDistances; }
    void SetCascadeDistances(const std::vector<float>& distances) { m_cascadeDistances = distances; }

    float GetShadowDistance() const { return m_shadowDistance; }
    void SetShadowDistance(float distance) { m_shadowDistance = distance; }

    // Light space matrices for cascades
    std::vector<DirectX::XMMATRIX> GetCascadeViewProjectionMatrices(
        const DirectX::XMMATRIX& cameraView,
        const DirectX::XMMATRIX& cameraProjection) const;

    // Override virtual methods
    virtual DirectX::XMMATRIX GetViewMatrix() const override;
    virtual DirectX::XMMATRIX GetProjectionMatrix() const override;

private:
    DirectX::XMFLOAT3 m_direction;

    // Cascade shadow mapping
    int m_cascadeCount;
    std::vector<float> m_cascadeDistances;
    float m_shadowDistance;

    // Helper methods
    DirectX::XMMATRIX CalculateLightSpaceMatrix(
        const std::vector<DirectX::XMFLOAT3>& frustumCorners) const;
    std::vector<DirectX::XMFLOAT3> GetFrustumCorners(
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection,
        float nearPlane,
        float farPlane) const;
};

// Point light (bulb, fire, etc.)
class PointLight : public Light {
public:
    PointLight();
    virtual ~PointLight() = default;

    const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
    void SetPosition(const DirectX::XMFLOAT3& position) { m_position = position; }
    void SetPosition(float x, float y, float z) { m_position = DirectX::XMFLOAT3(x, y, z); }

    virtual float GetRange() const override { return m_range; }
    void SetRange(float range) { m_range = range; }

    // Attenuation
    const DirectX::XMFLOAT3& GetAttenuation() const { return m_attenuation; }
    void SetAttenuation(float constant, float linear, float quadratic) {
        m_attenuation = DirectX::XMFLOAT3(constant, linear, quadratic);
    }

    // For cube shadow maps (6 faces)
    std::vector<DirectX::XMMATRIX> GetShadowViewMatrices() const;

    // Override virtual methods
    virtual DirectX::XMMATRIX GetViewMatrix() const override;
    virtual DirectX::XMMATRIX GetProjectionMatrix() const override;

private:
    DirectX::XMFLOAT3 m_position;
    float m_range;
    DirectX::XMFLOAT3 m_attenuation; // constant, linear, quadratic
};

// Spot light (flashlight, lamp)
class SpotLight : public Light {
public:
    SpotLight();
    virtual ~SpotLight() = default;

    const DirectX::XMFLOAT3& GetPosition() const { return m_position; }
    void SetPosition(const DirectX::XMFLOAT3& position) { m_position = position; }
    void SetPosition(float x, float y, float z) { m_position = DirectX::XMFLOAT3(x, y, z); }

    const DirectX::XMFLOAT3& GetDirection() const { return m_direction; }
    void SetDirection(const DirectX::XMFLOAT3& direction);
    void SetDirection(float x, float y, float z);

    virtual float GetRange() const override { return m_range; }
    void SetRange(float range) { m_range = range; }

    float GetInnerConeAngle() const { return m_innerConeAngle; }
    void SetInnerConeAngle(float angle) { m_innerConeAngle = angle; }

    float GetOuterConeAngle() const { return m_outerConeAngle; }
    void SetOuterConeAngle(float angle) { m_outerConeAngle = angle; }

    // Attenuation
    const DirectX::XMFLOAT3& GetAttenuation() const { return m_attenuation; }
    void SetAttenuation(float constant, float linear, float quadratic) {
        m_attenuation = DirectX::XMFLOAT3(constant, linear, quadratic);
    }

    // Override virtual methods
    virtual DirectX::XMMATRIX GetViewMatrix() const override;
    virtual DirectX::XMMATRIX GetProjectionMatrix() const override;

private:
    DirectX::XMFLOAT3 m_position;
    DirectX::XMFLOAT3 m_direction;
    float m_range;
    float m_innerConeAngle; // In radians
    float m_outerConeAngle; // In radians
    DirectX::XMFLOAT3 m_attenuation; // constant, linear, quadratic
};

// Light data structure for shaders (std140 layout)
struct LightData {
    DirectX::XMFLOAT4 position;      // w = light type
    DirectX::XMFLOAT4 direction;     // w = range
    DirectX::XMFLOAT4 color;         // w = intensity
    DirectX::XMFLOAT4 attenuation;   // xyz = attenuation, w = inner cone angle
    DirectX::XMFLOAT4 shadowParams;  // x = outer cone angle, y = shadow bias, z = shadow strength, w = enabled
    DirectX::XMFLOAT4X4 lightSpaceMatrix; // For shadow mapping
};

// Light manager for culling and batching
class LightManager {
public:
    LightManager();
    ~LightManager();

    // Light management
    void AddLight(std::shared_ptr<Light> light);
    void RemoveLight(std::shared_ptr<Light> light);
    void RemoveAllLights();

    // Light queries
    const std::vector<std::shared_ptr<Light>>& GetAllLights() const { return m_lights; }
    std::vector<std::shared_ptr<Light>> GetLightsInRange(const DirectX::XMFLOAT3& position, float radius) const;
    std::vector<std::shared_ptr<Light>> GetShadowCastingLights() const;

    // Culling
    void CullLights(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix);
    const std::vector<std::shared_ptr<Light>>& GetVisibleLights() const { return m_visibleLights; }

    // Shader data preparation
    std::vector<LightData> PrepareShaderData(size_t maxLights = 64) const;

    // Statistics
    size_t GetLightCount() const { return m_lights.size(); }
    size_t GetVisibleLightCount() const { return m_visibleLights.size(); }

private:
    std::vector<std::shared_ptr<Light>> m_lights;
    std::vector<std::shared_ptr<Light>> m_visibleLights;

    // Helper methods
    bool IsLightVisible(const Light* light, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix) const;
    LightData ConvertToShaderData(const Light* light) const;
};

} // namespace Renderer
} // namespace GameEngine

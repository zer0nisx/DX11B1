#pragma once

#include "Component.h"
#include "../Renderer/Light.h"
#include <memory>

namespace GameEngine {
namespace Scene {

// Base light component
class LightComponent : public Component {
public:
    COMPONENT_TYPE(LightComponent)

    LightComponent();
    virtual ~LightComponent() = default;

    // Light access
    std::shared_ptr<Renderer::Light> GetLight() const { return m_light; }

    // Common light properties
    const DirectX::XMFLOAT3& GetColor() const;
    void SetColor(const DirectX::XMFLOAT3& color);
    void SetColor(float r, float g, float b);

    float GetIntensity() const;
    void SetIntensity(float intensity);

    bool IsCastingShadows() const;
    void SetCastShadows(bool castShadows);

    int GetShadowMapSize() const;
    void SetShadowMapSize(int size);

    // Component lifecycle
    virtual void OnStart() override;
    virtual void OnUpdate(float deltaTime) override;
    virtual void OnDestroy() override;

protected:
    std::shared_ptr<Renderer::Light> m_light;

    // Update light transform from entity transform
    virtual void UpdateLightTransform() {}
};

// Directional light component (sun/moon)
class DirectionalLightComponent : public LightComponent {
public:
    COMPONENT_TYPE(DirectionalLightComponent)

    DirectionalLightComponent();
    virtual ~DirectionalLightComponent() = default;

    // Directional light specific properties
    const DirectX::XMFLOAT3& GetDirection() const;
    void SetDirection(const DirectX::XMFLOAT3& direction);
    void SetDirection(float x, float y, float z);

    // Cascade shadow mapping
    int GetCascadeCount() const;
    void SetCascadeCount(int count);

    float GetShadowDistance() const;
    void SetShadowDistance(float distance);

    // Get the underlying directional light
    std::shared_ptr<Renderer::DirectionalLight> GetDirectionalLight() const {
        return std::static_pointer_cast<Renderer::DirectionalLight>(m_light);
    }

protected:
    virtual void UpdateLightTransform() override;
};

// Point light component (bulb, fire, etc.)
class PointLightComponent : public LightComponent {
public:
    COMPONENT_TYPE(PointLightComponent)

    PointLightComponent();
    virtual ~PointLightComponent() = default;

    // Point light specific properties
    float GetRange() const;
    void SetRange(float range);

    const DirectX::XMFLOAT3& GetAttenuation() const;
    void SetAttenuation(float constant, float linear, float quadratic);
    void SetAttenuation(const DirectX::XMFLOAT3& attenuation);

    // Get the underlying point light
    std::shared_ptr<Renderer::PointLight> GetPointLight() const {
        return std::static_pointer_cast<Renderer::PointLight>(m_light);
    }

protected:
    virtual void UpdateLightTransform() override;
};

// Spot light component (flashlight, lamp)
class SpotLightComponent : public LightComponent {
public:
    COMPONENT_TYPE(SpotLightComponent)

    SpotLightComponent();
    virtual ~SpotLightComponent() = default;

    // Spot light specific properties
    float GetRange() const;
    void SetRange(float range);

    float GetInnerConeAngle() const;
    void SetInnerConeAngle(float angle);

    float GetOuterConeAngle() const;
    void SetOuterConeAngle(float angle);

    void SetConeAngles(float innerAngle, float outerAngle);

    const DirectX::XMFLOAT3& GetAttenuation() const;
    void SetAttenuation(float constant, float linear, float quadratic);
    void SetAttenuation(const DirectX::XMFLOAT3& attenuation);

    // Get the underlying spot light
    std::shared_ptr<Renderer::SpotLight> GetSpotLight() const {
        return std::static_pointer_cast<Renderer::SpotLight>(m_light);
    }

protected:
    virtual void UpdateLightTransform() override;
};

} // namespace Scene
} // namespace GameEngine

#include "Light.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <cmath>

namespace GameEngine {
namespace Renderer {

// Base Light implementation
Light::Light(LightType type)
    : m_type(type)
    , m_color(1.0f, 1.0f, 1.0f)
    , m_intensity(1.0f)
    , m_enabled(true)
    , m_castShadows(false)
    , m_shadowMapSize(1024)
    , m_shadowBias(0.001f)
    , m_shadowStrength(1.0f)
{
}

// DirectionalLight implementation
DirectionalLight::DirectionalLight()
    : Light(LightType::Directional)
    , m_direction(0.0f, -1.0f, 0.0f)
    , m_cascadeCount(3)
    , m_shadowDistance(100.0f)
{
    // Default cascade distances
    m_cascadeDistances = { 10.0f, 30.0f, 100.0f };
}

void DirectionalLight::SetDirection(const DirectX::XMFLOAT3& direction) {
    // Normalize direction
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
    dir = DirectX::XMVector3Normalize(dir);
    DirectX::XMStoreFloat3(&m_direction, dir);
}

void DirectionalLight::SetDirection(float x, float y, float z) {
    SetDirection(DirectX::XMFLOAT3(x, y, z));
}

DirectX::XMMATRIX DirectionalLight::GetViewMatrix() const {
    DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&m_direction);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // If direction is parallel to up vector, use different up vector
    DirectX::XMVECTOR dot = DirectX::XMVector3Dot(direction, up);
    if (abs(DirectX::XMVectorGetX(dot)) > 0.99f) {
        up = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    }

    DirectX::XMVECTOR position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR target = DirectX::XMVectorAdd(position, direction);

    return DirectX::XMMatrixLookAtLH(position, target, up);
}

DirectX::XMMATRIX DirectionalLight::GetProjectionMatrix() const {
    // Use orthographic projection for directional lights
    float size = 50.0f; // This should be calculated based on scene bounds
    return DirectX::XMMatrixOrthographicLH(size, size, 0.1f, m_shadowDistance);
}

std::vector<DirectX::XMMATRIX> DirectionalLight::GetCascadeViewProjectionMatrices(
    const DirectX::XMMATRIX& cameraView,
    const DirectX::XMMATRIX& cameraProjection) const {

    std::vector<DirectX::XMMATRIX> cascadeMatrices;
    cascadeMatrices.reserve(m_cascadeCount);

    float nearPlane = 0.1f;
    for (int i = 0; i < m_cascadeCount; i++) {
        float farPlane = m_cascadeDistances[i];

        // Get frustum corners for this cascade
        auto frustumCorners = GetFrustumCorners(cameraView, cameraProjection, nearPlane, farPlane);

        // Calculate light space matrix for this cascade
        DirectX::XMMATRIX lightSpaceMatrix = CalculateLightSpaceMatrix(frustumCorners);
        cascadeMatrices.push_back(lightSpaceMatrix);

        nearPlane = farPlane;
    }

    return cascadeMatrices;
}

DirectX::XMMATRIX DirectionalLight::CalculateLightSpaceMatrix(
    const std::vector<DirectX::XMFLOAT3>& frustumCorners) const {

    // Calculate center of frustum
    DirectX::XMVECTOR center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    for (const auto& corner : frustumCorners) {
        DirectX::XMVECTOR cornerVec = DirectX::XMLoadFloat3(&corner);
        center = DirectX::XMVectorAdd(center, cornerVec);
    }
    center = DirectX::XMVectorScale(center, 1.0f / frustumCorners.size());

    // Create light view matrix
    DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat3(&m_direction);
    DirectX::XMVECTOR lightPos = DirectX::XMVectorSubtract(center, DirectX::XMVectorScale(lightDir, 100.0f));
    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(
        lightPos,
        DirectX::XMVectorAdd(lightPos, lightDir),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );

    // Transform frustum corners to light space
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;
    float minZ = FLT_MAX, maxZ = -FLT_MAX;

    for (const auto& corner : frustumCorners) {
        DirectX::XMVECTOR cornerVec = DirectX::XMLoadFloat3(&corner);
        DirectX::XMVECTOR lightSpaceCorner = DirectX::XMVector3Transform(cornerVec, lightView);

        float x = DirectX::XMVectorGetX(lightSpaceCorner);
        float y = DirectX::XMVectorGetY(lightSpaceCorner);
        float z = DirectX::XMVectorGetZ(lightSpaceCorner);

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
        minZ = std::min(minZ, z);
        maxZ = std::max(maxZ, z);
    }

    // Create orthographic projection
    DirectX::XMMATRIX lightProjection = DirectX::XMMatrixOrthographicOffCenterLH(
        minX, maxX, minY, maxY, minZ, maxZ);

    return DirectX::XMMatrixMultiply(lightView, lightProjection);
}

std::vector<DirectX::XMFLOAT3> DirectionalLight::GetFrustumCorners(
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& projection,
    float nearPlane,
    float farPlane) const {

    DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr,
        DirectX::XMMatrixMultiply(view, projection));

    std::vector<DirectX::XMFLOAT3> corners;
    corners.reserve(8);

    // Define frustum corners in NDC space
    DirectX::XMFLOAT3 ndcCorners[8] = {
        {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, // Near
        {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}  // Far
    };

    // Transform to world space
    for (int i = 0; i < 8; i++) {
        DirectX::XMVECTOR corner = DirectX::XMLoadFloat3(&ndcCorners[i]);
        corner = DirectX::XMVector3Transform(corner, invViewProj);
        corner = DirectX::XMVectorScale(corner, 1.0f / DirectX::XMVectorGetW(corner));

        DirectX::XMFLOAT3 worldCorner;
        DirectX::XMStoreFloat3(&worldCorner, corner);
        corners.push_back(worldCorner);
    }

    return corners;
}

// PointLight implementation
PointLight::PointLight()
    : Light(LightType::Point)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_range(10.0f)
    , m_attenuation(1.0f, 0.09f, 0.032f)
{
}

std::vector<DirectX::XMMATRIX> PointLight::GetShadowViewMatrices() const {
    std::vector<DirectX::XMMATRIX> viewMatrices;
    viewMatrices.reserve(6);

    DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_position);

    // Define six directions for cube map faces
    DirectX::XMVECTOR targets[6] = {
        DirectX::XMVectorAdd(position, DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)),  // +X
        DirectX::XMVectorAdd(position, DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f)), // -X
        DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),  // +Y
        DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f)), // -Y
        DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)),  // +Z
        DirectX::XMVectorAdd(position, DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f))  // -Z
    };

    DirectX::XMVECTOR ups[6] = {
        DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), // +X
        DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), // -X
        DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),  // +Y
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), // -Y
        DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), // +Z
        DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f)  // -Z
    };

    for (int i = 0; i < 6; i++) {
        viewMatrices.push_back(DirectX::XMMatrixLookAtLH(position, targets[i], ups[i]));
    }

    return viewMatrices;
}

DirectX::XMMATRIX PointLight::GetViewMatrix() const {
    // Return first face view matrix (can be used for debugging)
    auto viewMatrices = GetShadowViewMatrices();
    return viewMatrices[0];
}

DirectX::XMMATRIX PointLight::GetProjectionMatrix() const {
    // 90-degree FOV perspective projection for cube map
    return DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.1f, m_range);
}

// SpotLight implementation
SpotLight::SpotLight()
    : Light(LightType::Spot)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_direction(0.0f, -1.0f, 0.0f)
    , m_range(10.0f)
    , m_innerConeAngle(DirectX::XM_PIDIV4)      // 45 degrees
    , m_outerConeAngle(DirectX::XM_PIDIV4 * 1.2f) // 54 degrees
    , m_attenuation(1.0f, 0.09f, 0.032f)
{
}

void SpotLight::SetDirection(const DirectX::XMFLOAT3& direction) {
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
    dir = DirectX::XMVector3Normalize(dir);
    DirectX::XMStoreFloat3(&m_direction, dir);
}

void SpotLight::SetDirection(float x, float y, float z) {
    SetDirection(DirectX::XMFLOAT3(x, y, z));
}

DirectX::XMMATRIX SpotLight::GetViewMatrix() const {
    DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_position);
    DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&m_direction);
    DirectX::XMVECTOR target = DirectX::XMVectorAdd(position, direction);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    return DirectX::XMMatrixLookAtLH(position, target, up);
}

DirectX::XMMATRIX SpotLight::GetProjectionMatrix() const {
    // Use outer cone angle for projection FOV
    float fov = m_outerConeAngle * 2.0f;
    return DirectX::XMMatrixPerspectiveFovLH(fov, 1.0f, 0.1f, m_range);
}

// LightManager implementation
LightManager::LightManager() {
    LOG_INFO("LightManager initialized");
}

LightManager::~LightManager() {
    RemoveAllLights();
    LOG_INFO("LightManager destroyed");
}

void LightManager::AddLight(std::shared_ptr<Light> light) {
    if (!light) {
        LOG_WARNING("Attempted to add null light");
        return;
    }

    m_lights.push_back(light);
    LOG_DEBUG("Light added to manager (type: " << static_cast<int>(light->GetType()) << ")");
}

void LightManager::RemoveLight(std::shared_ptr<Light> light) {
    auto it = std::find(m_lights.begin(), m_lights.end(), light);
    if (it != m_lights.end()) {
        m_lights.erase(it);
        LOG_DEBUG("Light removed from manager");
    }
}

void LightManager::RemoveAllLights() {
    m_lights.clear();
    m_visibleLights.clear();
    LOG_DEBUG("All lights removed from manager");
}

std::vector<std::shared_ptr<Light>> LightManager::GetLightsInRange(
    const DirectX::XMFLOAT3& position, float radius) const {

    std::vector<std::shared_ptr<Light>> lightsInRange;
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);

    for (const auto& light : m_lights) {
        if (!light->IsEnabled()) continue;

        bool inRange = false;

        if (light->GetType() == LightType::Directional) {
            // Directional lights affect everything
            inRange = true;
        } else if (light->GetType() == LightType::Point) {
            auto pointLight = std::static_pointer_cast<PointLight>(light);
            DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&pointLight->GetPosition());
            float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(pos, lightPos)));
            inRange = distance <= (pointLight->GetRange() + radius);
        } else if (light->GetType() == LightType::Spot) {
            auto spotLight = std::static_pointer_cast<SpotLight>(light);
            DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&spotLight->GetPosition());
            float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(pos, lightPos)));
            inRange = distance <= (spotLight->GetRange() + radius);
        }

        if (inRange) {
            lightsInRange.push_back(light);
        }
    }

    return lightsInRange;
}

std::vector<std::shared_ptr<Light>> LightManager::GetShadowCastingLights() const {
    std::vector<std::shared_ptr<Light>> shadowLights;

    for (const auto& light : m_lights) {
        if (light->IsEnabled() && light->IsCastingShadows()) {
            shadowLights.push_back(light);
        }
    }

    return shadowLights;
}

void LightManager::CullLights(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix) {
    m_visibleLights.clear();

    for (const auto& light : m_lights) {
        if (light->IsEnabled() && IsLightVisible(light.get(), viewMatrix, projectionMatrix)) {
            m_visibleLights.push_back(light);
        }
    }
}

std::vector<LightData> LightManager::PrepareShaderData(size_t maxLights) const {
    std::vector<LightData> shaderData;
    shaderData.reserve(std::min(m_visibleLights.size(), maxLights));

    size_t count = 0;
    for (const auto& light : m_visibleLights) {
        if (count >= maxLights) break;

        shaderData.push_back(ConvertToShaderData(light.get()));
        count++;
    }

    return shaderData;
}

bool LightManager::IsLightVisible(const Light* light,
    const DirectX::XMMATRIX& viewMatrix,
    const DirectX::XMMATRIX& projectionMatrix) const {

    // Simplified visibility test - in a real engine you'd do proper frustum culling
    if (light->GetType() == LightType::Directional) {
        return true; // Directional lights are always visible
    }

    // For point and spot lights, check if they're within range
    // This is a simplified implementation
    return true;
}

LightData LightManager::ConvertToShaderData(const Light* light) const {
    LightData data;

    // Set common properties
    data.color = DirectX::XMFLOAT4(light->GetColor().x, light->GetColor().y, light->GetColor().z, light->GetIntensity());
    data.shadowParams = DirectX::XMFLOAT4(0.0f, light->GetShadowBias(), light->GetShadowStrength(), light->IsEnabled() ? 1.0f : 0.0f);

    switch (light->GetType()) {
        case LightType::Directional: {
            auto dirLight = static_cast<const DirectionalLight*>(light);
            data.position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, static_cast<float>(LightType::Directional));
            data.direction = DirectX::XMFLOAT4(dirLight->GetDirection().x, dirLight->GetDirection().y, dirLight->GetDirection().z, 0.0f);
            data.attenuation = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
            break;
        }
        case LightType::Point: {
            auto pointLight = static_cast<const PointLight*>(light);
            data.position = DirectX::XMFLOAT4(pointLight->GetPosition().x, pointLight->GetPosition().y, pointLight->GetPosition().z, static_cast<float>(LightType::Point));
            data.direction = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, pointLight->GetRange());
            data.attenuation = DirectX::XMFLOAT4(pointLight->GetAttenuation().x, pointLight->GetAttenuation().y, pointLight->GetAttenuation().z, 0.0f);
            break;
        }
        case LightType::Spot: {
            auto spotLight = static_cast<const SpotLight*>(light);
            data.position = DirectX::XMFLOAT4(spotLight->GetPosition().x, spotLight->GetPosition().y, spotLight->GetPosition().z, static_cast<float>(LightType::Spot));
            data.direction = DirectX::XMFLOAT4(spotLight->GetDirection().x, spotLight->GetDirection().y, spotLight->GetDirection().z, spotLight->GetRange());
            data.attenuation = DirectX::XMFLOAT4(spotLight->GetAttenuation().x, spotLight->GetAttenuation().y, spotLight->GetAttenuation().z, spotLight->GetInnerConeAngle());
            data.shadowParams.x = spotLight->GetOuterConeAngle();
            break;
        }
    }

    // Set light space matrix for shadow mapping
    DirectX::XMMATRIX lightSpace = DirectX::XMMatrixMultiply(light->GetViewMatrix(), light->GetProjectionMatrix());
    DirectX::XMStoreFloat4x4(&data.lightSpaceMatrix, lightSpace);

    return data;
}

} // namespace Renderer
} // namespace GameEngine

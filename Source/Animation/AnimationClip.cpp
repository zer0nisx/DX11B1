#include "AnimationClip.h"
#include "../Core/Logger.h"
#include <algorithm>

namespace GameEngine {
namespace Animation {

// AnimationChannel implementations
DirectX::XMFLOAT3 AnimationChannel::SamplePosition(float time) const {
    if (positionKeys.empty()) {
        return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    if (positionKeys.size() == 1) {
        return positionKeys[0].position;
    }

    size_t keyIndex = FindPositionKeyframe(time);

    if (keyIndex >= positionKeys.size() - 1) {
        return positionKeys.back().position;
    }

    const PositionKeyframe& key1 = positionKeys[keyIndex];
    const PositionKeyframe& key2 = positionKeys[keyIndex + 1];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.position;
    }

    float t = (time - key1.time) / deltaTime;
    return InterpolatePosition(key1, key2, t);
}

DirectX::XMFLOAT4 AnimationChannel::SampleRotation(float time) const {
    if (rotationKeys.empty()) {
        return DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // Identity quaternion
    }

    if (rotationKeys.size() == 1) {
        return rotationKeys[0].rotation;
    }

    size_t keyIndex = FindRotationKeyframe(time);

    if (keyIndex >= rotationKeys.size() - 1) {
        return rotationKeys.back().rotation;
    }

    const RotationKeyframe& key1 = rotationKeys[keyIndex];
    const RotationKeyframe& key2 = rotationKeys[keyIndex + 1];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.rotation;
    }

    float t = (time - key1.time) / deltaTime;
    return InterpolateRotation(key1, key2, t);
}

DirectX::XMFLOAT3 AnimationChannel::SampleScale(float time) const {
    if (scaleKeys.empty()) {
        return DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    }

    if (scaleKeys.size() == 1) {
        return scaleKeys[0].scale;
    }

    size_t keyIndex = FindScaleKeyframe(time);

    if (keyIndex >= scaleKeys.size() - 1) {
        return scaleKeys.back().scale;
    }

    const ScaleKeyframe& key1 = scaleKeys[keyIndex];
    const ScaleKeyframe& key2 = scaleKeys[keyIndex + 1];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.scale;
    }

    float t = (time - key1.time) / deltaTime;
    return InterpolateScale(key1, key2, t);
}

DirectX::XMMATRIX AnimationChannel::SampleTransform(float time) const {
    DirectX::XMFLOAT3 position = SamplePosition(time);
    DirectX::XMFLOAT4 rotation = SampleRotation(time);
    DirectX::XMFLOAT3 scale = SampleScale(time);

    // Create transformation matrix: Scale * Rotation * Translation
    DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
    DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

    return DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(scaleMatrix, rotationMatrix), translationMatrix);
}

DirectX::XMFLOAT3 AnimationChannel::InterpolatePosition(const PositionKeyframe& key1, const PositionKeyframe& key2, float t) const {
    // Linear interpolation
    DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&key1.position);
    DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&key2.position);
    DirectX::XMVECTOR result = DirectX::XMVectorLerp(pos1, pos2, t);

    DirectX::XMFLOAT3 interpolated;
    DirectX::XMStoreFloat3(&interpolated, result);
    return interpolated;
}

DirectX::XMFLOAT4 AnimationChannel::InterpolateRotation(const RotationKeyframe& key1, const RotationKeyframe& key2, float t) const {
    // Spherical linear interpolation (SLERP)
    DirectX::XMVECTOR quat1 = DirectX::XMLoadFloat4(&key1.rotation);
    DirectX::XMVECTOR quat2 = DirectX::XMLoadFloat4(&key2.rotation);
    DirectX::XMVECTOR result = DirectX::XMQuaternionSlerp(quat1, quat2, t);

    DirectX::XMFLOAT4 interpolated;
    DirectX::XMStoreFloat4(&interpolated, result);
    return interpolated;
}

DirectX::XMFLOAT3 AnimationChannel::InterpolateScale(const ScaleKeyframe& key1, const ScaleKeyframe& key2, float t) const {
    // Linear interpolation
    DirectX::XMVECTOR scale1 = DirectX::XMLoadFloat3(&key1.scale);
    DirectX::XMVECTOR scale2 = DirectX::XMLoadFloat3(&key2.scale);
    DirectX::XMVECTOR result = DirectX::XMVectorLerp(scale1, scale2, t);

    DirectX::XMFLOAT3 interpolated;
    DirectX::XMStoreFloat3(&interpolated, result);
    return interpolated;
}

size_t AnimationChannel::FindPositionKeyframe(float time) const {
    auto it = std::lower_bound(positionKeys.begin(), positionKeys.end(), time,
        [](const PositionKeyframe& key, float time) {
            return key.time < time;
        });

    if (it == positionKeys.begin()) return 0;
    return std::distance(positionKeys.begin(), it) - 1;
}

size_t AnimationChannel::FindRotationKeyframe(float time) const {
    auto it = std::lower_bound(rotationKeys.begin(), rotationKeys.end(), time,
        [](const RotationKeyframe& key, float time) {
            return key.time < time;
        });

    if (it == rotationKeys.begin()) return 0;
    return std::distance(rotationKeys.begin(), it) - 1;
}

size_t AnimationChannel::FindScaleKeyframe(float time) const {
    auto it = std::lower_bound(scaleKeys.begin(), scaleKeys.end(), time,
        [](const ScaleKeyframe& key, float time) {
            return key.time < time;
        });

    if (it == scaleKeys.begin()) return 0;
    return std::distance(scaleKeys.begin(), it) - 1;
}

// AnimationClip implementations
AnimationClip::AnimationClip(const std::string& name)
    : m_name(name)
    , m_duration(0.0f)
    , m_ticksPerSecond(25.0f)
{
}

void AnimationClip::AddChannel(const AnimationChannel& channel) {
    m_channels.push_back(channel);

    // Update duration if necessary
    float channelDuration = 0.0f;

    if (!channel.positionKeys.empty()) {
        channelDuration = std::max(channelDuration, channel.positionKeys.back().time);
    }

    if (!channel.rotationKeys.empty()) {
        channelDuration = std::max(channelDuration, channel.rotationKeys.back().time);
    }

    if (!channel.scaleKeys.empty()) {
        channelDuration = std::max(channelDuration, channel.scaleKeys.back().time);
    }

    m_duration = std::max(m_duration, channelDuration);
}

AnimationChannel* AnimationClip::FindChannel(const std::string& boneName) {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
        [&boneName](const AnimationChannel& channel) {
            return channel.boneName == boneName;
        });

    return (it != m_channels.end()) ? &(*it) : nullptr;
}

AnimationChannel* AnimationClip::FindChannel(int boneIndex) {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
        [boneIndex](const AnimationChannel& channel) {
            return channel.boneIndex == boneIndex;
        });

    return (it != m_channels.end()) ? &(*it) : nullptr;
}

void AnimationClip::SampleAnimation(float time, std::vector<DirectX::XMMATRIX>& boneTransforms) const {
    // Ensure bone transforms array is large enough
    size_t maxBoneIndex = 0;
    for (const auto& channel : m_channels) {
        if (channel.boneIndex >= 0) {
            maxBoneIndex = std::max(maxBoneIndex, static_cast<size_t>(channel.boneIndex));
        }
    }

    if (boneTransforms.size() <= maxBoneIndex) {
        boneTransforms.resize(maxBoneIndex + 1, DirectX::XMMatrixIdentity());
    }

    // Sample each channel
    for (const auto& channel : m_channels) {
        if (channel.boneIndex >= 0 && channel.boneIndex < static_cast<int>(boneTransforms.size())) {
            boneTransforms[channel.boneIndex] = channel.SampleTransform(time);
        }
    }
}

float AnimationClip::NormalizeTime(float time) const {
    return std::max(0.0f, std::min(time, m_duration));
}

float AnimationClip::LoopTime(float time) const {
    if (m_duration <= 0.0f) return 0.0f;

    float normalizedTime = fmod(time, m_duration);
    return (normalizedTime < 0.0f) ? normalizedTime + m_duration : normalizedTime;
}

bool AnimationClip::IsValid() const {
    if (m_name.empty() || m_duration <= 0.0f || m_channels.empty()) {
        return false;
    }

    // Check that all channels have valid bone indices
    for (const auto& channel : m_channels) {
        if (channel.boneIndex < 0) {
            LOG_WARNING("Animation channel '" << channel.boneName << "' has invalid bone index");
        }

        if (channel.positionKeys.empty() && channel.rotationKeys.empty() && channel.scaleKeys.empty()) {
            LOG_WARNING("Animation channel '" << channel.boneName << "' has no keyframes");
        }
    }

    return true;
}

} // namespace Animation
} // namespace GameEngine

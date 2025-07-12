#include "AnimationClip.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <DirectXMath.h>

using namespace GameEngine::Animation;
using namespace GameEngine::Core;
using namespace DirectX;

// AnimationChannel implementation
DirectX::XMFLOAT3 AnimationChannel::SamplePosition(float time) const {
    if (positionKeys.empty()) {
        return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    if (positionKeys.size() == 1) {
        return positionKeys[0].position;
    }

    size_t keyIndex = FindPositionKeyframe(time);

    if (keyIndex == positionKeys.size() - 1) {
        return positionKeys[keyIndex].position;
    }

    const PositionKeyframe& key1 = positionKeys[keyIndex];
    const PositionKeyframe& key2 = positionKeys[keyIndex + 1];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.position;
    }

    float t = (time - key1.time) / deltaTime;
    t = std::clamp(t, 0.0f, 1.0f);

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

    if (keyIndex == rotationKeys.size() - 1) {
        return rotationKeys[keyIndex].rotation;
    }

    const RotationKeyframe& key1 = rotationKeys[keyIndex];
    const RotationKeyframe& key2 = rotationKeys[keyIndex + 1];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.rotation;
    }

    float t = (time - key1.time) / deltaTime;
    t = std::clamp(t, 0.0f, 1.0f);

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

    if (keyIndex == scaleKeys.size() - 1) {
        return scaleKeys[keyIndex].scale;
    }

    const ScaleKeyframe& key1 = scaleKeys[keyIndex];
    const ScaleKeyframe& key2 = scaleKeys[keyIndex + 1];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.scale;
    }

    float t = (time - key1.time) / deltaTime;
    t = std::clamp(t, 0.0f, 1.0f);

    return InterpolateScale(key1, key2, t);
}

DirectX::XMMATRIX AnimationChannel::SampleTransform(float time) const {
    XMFLOAT3 position = SamplePosition(time);
    XMFLOAT4 rotation = SampleRotation(time);
    XMFLOAT3 scale = SampleScale(time);

    // Convert to DirectX vectors
    XMVECTOR positionVec = XMLoadFloat3(&position);
    XMVECTOR rotationQuat = XMLoadFloat4(&rotation);
    XMVECTOR scaleVec = XMLoadFloat3(&scale);

    // Create transformation matrix
    XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scaleVec);
    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);
    XMMATRIX translationMatrix = XMMatrixTranslationFromVector(positionVec);

    return scaleMatrix * rotationMatrix * translationMatrix;
}

DirectX::XMFLOAT3 AnimationChannel::InterpolatePosition(const PositionKeyframe& key1, const PositionKeyframe& key2, float t) const {
    XMVECTOR pos1 = XMLoadFloat3(&key1.position);
    XMVECTOR pos2 = XMLoadFloat3(&key2.position);

    XMVECTOR result = XMVectorLerp(pos1, pos2, t);

    XMFLOAT3 interpolated;
    XMStoreFloat3(&interpolated, result);
    return interpolated;
}

DirectX::XMFLOAT4 AnimationChannel::InterpolateRotation(const RotationKeyframe& key1, const RotationKeyframe& key2, float t) const {
    XMVECTOR quat1 = XMLoadFloat4(&key1.rotation);
    XMVECTOR quat2 = XMLoadFloat4(&key2.rotation);

    // Spherical linear interpolation for smooth rotation
    XMVECTOR result = XMQuaternionSlerp(quat1, quat2, t);

    // Normalize the result quaternion
    result = XMQuaternionNormalize(result);

    XMFLOAT4 interpolated;
    XMStoreFloat4(&interpolated, result);
    return interpolated;
}

DirectX::XMFLOAT3 AnimationChannel::InterpolateScale(const ScaleKeyframe& key1, const ScaleKeyframe& key2, float t) const {
    XMVECTOR scale1 = XMLoadFloat3(&key1.scale);
    XMVECTOR scale2 = XMLoadFloat3(&key2.scale);

    XMVECTOR result = XMVectorLerp(scale1, scale2, t);

    XMFLOAT3 interpolated;
    XMStoreFloat3(&interpolated, result);
    return interpolated;
}

size_t AnimationChannel::FindPositionKeyframe(float time) const {
    if (positionKeys.empty()) {
        return 0;
    }

    // Binary search for efficiency
    auto it = std::lower_bound(positionKeys.begin(), positionKeys.end(), time,
        [](const PositionKeyframe& keyframe, float t) {
            return keyframe.time < t;
        });

    if (it == positionKeys.end()) {
        return positionKeys.size() - 1;
    }

    if (it == positionKeys.begin()) {
        return 0;
    }

    // Return the index of the keyframe before the found one
    return static_cast<size_t>(std::distance(positionKeys.begin(), it) - 1);
}

size_t AnimationChannel::FindRotationKeyframe(float time) const {
    if (rotationKeys.empty()) {
        return 0;
    }

    auto it = std::lower_bound(rotationKeys.begin(), rotationKeys.end(), time,
        [](const RotationKeyframe& keyframe, float t) {
            return keyframe.time < t;
        });

    if (it == rotationKeys.end()) {
        return rotationKeys.size() - 1;
    }

    if (it == rotationKeys.begin()) {
        return 0;
    }

    return static_cast<size_t>(std::distance(rotationKeys.begin(), it) - 1);
}

size_t AnimationChannel::FindScaleKeyframe(float time) const {
    if (scaleKeys.empty()) {
        return 0;
    }

    auto it = std::lower_bound(scaleKeys.begin(), scaleKeys.end(), time,
        [](const ScaleKeyframe& keyframe, float t) {
            return keyframe.time < t;
        });

    if (it == scaleKeys.end()) {
        return scaleKeys.size() - 1;
    }

    if (it == scaleKeys.begin()) {
        return 0;
    }

    return static_cast<size_t>(std::distance(scaleKeys.begin(), it) - 1);
}

// AnimationClip implementation
AnimationClip::AnimationClip(const std::string& name)
    : m_name(name)
    , m_duration(0.0f)
    , m_ticksPerSecond(25.0f)
{
}

void AnimationClip::AddChannel(const AnimationChannel& channel) {
    m_channels.push_back(channel);

    // Update duration based on channel keyframes
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
    if (boneTransforms.empty()) {
        Logger::GetInstance().LogWarning("AnimationClip::SampleAnimation - Empty bone transforms array");
        return;
    }

    // Normalize time to animation duration
    float normalizedTime = NormalizeTime(time);

    // Initialize all bone transforms to identity
    for (auto& transform : boneTransforms) {
        transform = XMMatrixIdentity();
    }

    // Sample each channel and apply to corresponding bone
    for (const auto& channel : m_channels) {
        if (channel.boneIndex >= 0 && channel.boneIndex < static_cast<int>(boneTransforms.size())) {
            boneTransforms[channel.boneIndex] = channel.SampleTransform(normalizedTime);
        }
    }
}

float AnimationClip::NormalizeTime(float time) const {
    if (m_duration <= 0.0f) {
        return 0.0f;
    }

    return std::clamp(time, 0.0f, m_duration);
}

float AnimationClip::LoopTime(float time) const {
    if (m_duration <= 0.0f) {
        return 0.0f;
    }

    // Handle negative time
    if (time < 0.0f) {
        time = m_duration + fmod(time, m_duration);
    }

    return fmod(time, m_duration);
}

bool AnimationClip::IsValid() const {
    if (m_name.empty()) {
        return false;
    }

    if (m_duration <= 0.0f) {
        return false;
    }

    if (m_channels.empty()) {
        return false;
    }

    // Check if all channels have valid data
    for (const auto& channel : m_channels) {
        if (channel.boneName.empty() && channel.boneIndex < 0) {
            return false;
        }

        // At least one type of keyframe should exist
        if (channel.positionKeys.empty() &&
            channel.rotationKeys.empty() &&
            channel.scaleKeys.empty()) {
            return false;
        }
    }

    return true;
}

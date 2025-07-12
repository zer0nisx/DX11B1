#pragma once

#include <vector>
#include <string>
#include <DirectXMath.h>

namespace GameEngine {
namespace Animation {

// Animation keyframe types
struct PositionKeyframe {
    float time;
    DirectX::XMFLOAT3 position;

    PositionKeyframe(float t, const DirectX::XMFLOAT3& pos) : time(t), position(pos) {}
};

struct RotationKeyframe {
    float time;
    DirectX::XMFLOAT4 rotation; // Quaternion

    RotationKeyframe(float t, const DirectX::XMFLOAT4& rot) : time(t), rotation(rot) {}
};

struct ScaleKeyframe {
    float time;
    DirectX::XMFLOAT3 scale;

    ScaleKeyframe(float t, const DirectX::XMFLOAT3& s) : time(t), scale(s) {}
};

// Animation channel (per bone)
struct AnimationChannel {
    std::string boneName;
    int boneIndex;

    std::vector<PositionKeyframe> positionKeys;
    std::vector<RotationKeyframe> rotationKeys;
    std::vector<ScaleKeyframe> scaleKeys;

    AnimationChannel() : boneIndex(-1) {}

    // Sampling methods
    DirectX::XMFLOAT3 SamplePosition(float time) const;
    DirectX::XMFLOAT4 SampleRotation(float time) const;
    DirectX::XMFLOAT3 SampleScale(float time) const;
    DirectX::XMMATRIX SampleTransform(float time) const;

private:
    // Interpolation helpers
    DirectX::XMFLOAT3 InterpolatePosition(const PositionKeyframe& key1, const PositionKeyframe& key2, float t) const;
    DirectX::XMFLOAT4 InterpolateRotation(const RotationKeyframe& key1, const RotationKeyframe& key2, float t) const;
    DirectX::XMFLOAT3 InterpolateScale(const ScaleKeyframe& key1, const ScaleKeyframe& key2, float t) const;

    // Keyframe finding
    size_t FindPositionKeyframe(float time) const;
    size_t FindRotationKeyframe(float time) const;
    size_t FindScaleKeyframe(float time) const;
};

// Animation clip
class AnimationClip {
public:
    AnimationClip(const std::string& name);
    ~AnimationClip() = default;

    // Properties
    const std::string& GetName() const { return m_name; }
    float GetDuration() const { return m_duration; }
    float GetTicksPerSecond() const { return m_ticksPerSecond; }

    void SetDuration(float duration) { m_duration = duration; }
    void SetTicksPerSecond(float tps) { m_ticksPerSecond = tps; }

    // Channels
    void AddChannel(const AnimationChannel& channel);
    const std::vector<AnimationChannel>& GetChannels() const { return m_channels; }
    AnimationChannel* FindChannel(const std::string& boneName);
    AnimationChannel* FindChannel(int boneIndex);

    // Animation sampling
    void SampleAnimation(float time, std::vector<DirectX::XMMATRIX>& boneTransforms) const;

    // Time utilities
    float NormalizeTime(float time) const; // Clamp to [0, duration]
    float LoopTime(float time) const;      // Loop time within duration

    // Validation
    bool IsValid() const;

private:
    std::string m_name;
    float m_duration;
    float m_ticksPerSecond;
    std::vector<AnimationChannel> m_channels;
};

} // namespace Animation
} // namespace GameEngine

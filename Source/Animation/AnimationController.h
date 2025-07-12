#pragma once

#include "AnimationClip.h"
#include "../Scene/Component.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace GameEngine {
namespace Animation {

// Animation state for state machine
struct AnimationState {
    std::string name;
    std::shared_ptr<AnimationClip> clip;
    bool loop;
    float speed;
    float blendInTime;
    float blendOutTime;

    AnimationState() : loop(true), speed(1.0f), blendInTime(0.3f), blendOutTime(0.3f) {}
};

// Animation transition
struct AnimationTransition {
    std::string fromState;
    std::string toState;
    float duration;
    bool hasExitTime;
    float exitTime; // Normalized time (0-1) in source animation

    // Transition conditions (simplified)
    std::string triggerName;

    AnimationTransition() : duration(0.3f), hasExitTime(false), exitTime(0.9f) {}
};

// Currently playing animation info
struct PlayingAnimation {
    std::shared_ptr<AnimationClip> clip;
    float currentTime;
    float speed;
    float weight;
    bool loop;
    bool isBlending;
    float blendTime;
    float blendDuration;

    PlayingAnimation() : currentTime(0.0f), speed(1.0f), weight(1.0f),
                        loop(true), isBlending(false), blendTime(0.0f), blendDuration(0.0f) {}
};

class AnimationController : public Scene::Component {
public:
    COMPONENT_TYPE(AnimationController)

    AnimationController();
    virtual ~AnimationController() = default;

    // Animation clip management
    void AddAnimationClip(const std::string& name, std::shared_ptr<AnimationClip> clip);
    std::shared_ptr<AnimationClip> GetAnimationClip(const std::string& name) const;
    void RemoveAnimationClip(const std::string& name);

    // Simple animation playback
    void Play(const std::string& animationName, bool loop = true, float speed = 1.0f);
    void Stop();
    void Pause();
    void Resume();

    // Animation blending
    void CrossFade(const std::string& animationName, float fadeDuration, bool loop = true, float speed = 1.0f);
    void AddLayer(const std::string& animationName, float weight, bool loop = true, float speed = 1.0f);
    void RemoveLayer(int layerIndex);
    void SetLayerWeight(int layerIndex, float weight);

    // State machine
    void AddState(const std::string& stateName, const std::string& clipName, bool loop = true, float speed = 1.0f);
    void AddTransition(const std::string& fromState, const std::string& toState, float duration = 0.3f);
    void SetTrigger(const std::string& triggerName);
    void TransitionToState(const std::string& stateName);

    const std::string& GetCurrentState() const { return m_currentState; }

    // Animation parameters
    void SetFloat(const std::string& name, float value);
    void SetInt(const std::string& name, int value);
    void SetBool(const std::string& name, bool value);

    float GetFloat(const std::string& name) const;
    int GetInt(const std::string& name) const;
    bool GetBool(const std::string& name) const;

    // Animation info
    bool IsPlaying() const { return m_isPlaying; }
    bool IsPaused() const { return m_isPaused; }
    float GetCurrentTime() const;
    float GetNormalizedTime() const; // 0-1
    std::string GetCurrentAnimationName() const;

    // Bone transforms output
    const std::vector<DirectX::XMMATRIX>& GetBoneTransforms() const { return m_boneTransforms; }
    void SetBoneCount(size_t boneCount) { m_boneTransforms.resize(boneCount, DirectX::XMMatrixIdentity()); }

    // Component lifecycle
    virtual void OnStart() override;
    virtual void OnUpdate(float deltaTime) override;

private:
    // Animation storage
    std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_animationClips;

    // State machine
    std::unordered_map<std::string, AnimationState> m_states;
    std::vector<AnimationTransition> m_transitions;
    std::string m_currentState;
    std::string m_targetState;

    // Currently playing animations (for layering/blending)
    std::vector<PlayingAnimation> m_playingAnimations;

    // Control state
    bool m_isPlaying;
    bool m_isPaused;
    bool m_useStateMachine;

    // Parameters
    std::unordered_map<std::string, float> m_floatParams;
    std::unordered_map<std::string, int> m_intParams;
    std::unordered_map<std::string, bool> m_boolParams;
    std::unordered_map<std::string, bool> m_triggers;

    // Output
    std::vector<DirectX::XMMATRIX> m_boneTransforms;

    // Internal methods
    void UpdateAnimations(float deltaTime);
    void UpdateStateMachine(float deltaTime);
    void BlendAnimations();
    void CheckTransitions();
    void StartTransition(const AnimationTransition& transition);

    // Helper methods
    void CleanupFinishedAnimations();
    void NormalizeWeights();
    AnimationState* FindState(const std::string& name);
    std::vector<AnimationTransition*> FindTransitionsFromState(const std::string& stateName);
};

} // namespace Animation
} // namespace GameEngine

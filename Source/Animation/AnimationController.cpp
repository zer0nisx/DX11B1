#include "AnimationController.h"
#include "../Core/Logger.h"
#include <algorithm>

namespace GameEngine {
namespace Animation {

AnimationController::AnimationController()
    : m_isPlaying(false)
    , m_isPaused(false)
    , m_useStateMachine(false)
{
}

void AnimationController::OnStart() {
    LOG_DEBUG("AnimationController started for entity: " << (GetEntity() ? GetEntity()->GetName() : "Unknown"));
}

void AnimationController::OnUpdate(float deltaTime) {
    if (!IsEnabled() || m_isPaused) return;

    if (m_useStateMachine) {
        UpdateStateMachine(deltaTime);
    } else {
        UpdateAnimations(deltaTime);
    }

    BlendAnimations();
    CleanupFinishedAnimations();
}

// Animation clip management
void AnimationController::AddAnimationClip(const std::string& name, std::shared_ptr<AnimationClip> clip) {
    if (!clip) {
        LOG_ERROR("Cannot add null animation clip: " << name);
        return;
    }

    m_animationClips[name] = clip;
    LOG_DEBUG("Animation clip added: " << name);
}

std::shared_ptr<AnimationClip> AnimationController::GetAnimationClip(const std::string& name) const {
    auto it = m_animationClips.find(name);
    return (it != m_animationClips.end()) ? it->second : nullptr;
}

void AnimationController::RemoveAnimationClip(const std::string& name) {
    auto it = m_animationClips.find(name);
    if (it != m_animationClips.end()) {
        m_animationClips.erase(it);
        LOG_DEBUG("Animation clip removed: " << name);
    }
}

// Simple animation playback
void AnimationController::Play(const std::string& animationName, bool loop, float speed) {
    auto clip = GetAnimationClip(animationName);
    if (!clip) {
        LOG_ERROR("Animation clip not found: " << animationName);
        return;
    }

    // Stop current animations
    m_playingAnimations.clear();

    // Start new animation
    PlayingAnimation playingAnim;
    playingAnim.clip = clip;
    playingAnim.currentTime = 0.0f;
    playingAnim.speed = speed;
    playingAnim.weight = 1.0f;
    playingAnim.loop = loop;

    m_playingAnimations.push_back(playingAnim);
    m_isPlaying = true;
    m_useStateMachine = false;

    LOG_DEBUG("Playing animation: " << animationName);
}

void AnimationController::Stop() {
    m_playingAnimations.clear();
    m_isPlaying = false;

    // Reset bone transforms to identity
    std::fill(m_boneTransforms.begin(), m_boneTransforms.end(), DirectX::XMMatrixIdentity());

    LOG_DEBUG("Animation stopped");
}

void AnimationController::Pause() {
    m_isPaused = true;
    LOG_DEBUG("Animation paused");
}

void AnimationController::Resume() {
    m_isPaused = false;
    LOG_DEBUG("Animation resumed");
}

// Animation blending
void AnimationController::CrossFade(const std::string& animationName, float fadeDuration, bool loop, float speed) {
    auto clip = GetAnimationClip(animationName);
    if (!clip) {
        LOG_ERROR("Animation clip not found: " << animationName);
        return;
    }

    // Start fade out for current animations
    for (auto& playingAnim : m_playingAnimations) {
        if (!playingAnim.isBlending) {
            playingAnim.isBlending = true;
            playingAnim.blendTime = 0.0f;
            playingAnim.blendDuration = fadeDuration;
        }
    }

    // Add new animation with fade in
    PlayingAnimation newAnim;
    newAnim.clip = clip;
    newAnim.currentTime = 0.0f;
    newAnim.speed = speed;
    newAnim.weight = 0.0f; // Start with 0 weight, will fade in
    newAnim.loop = loop;
    newAnim.isBlending = true;
    newAnim.blendTime = 0.0f;
    newAnim.blendDuration = fadeDuration;

    m_playingAnimations.push_back(newAnim);
    m_isPlaying = true;
    m_useStateMachine = false;

    LOG_DEBUG("Cross-fading to animation: " << animationName);
}

void AnimationController::AddLayer(const std::string& animationName, float weight, bool loop, float speed) {
    auto clip = GetAnimationClip(animationName);
    if (!clip) {
        LOG_ERROR("Animation clip not found: " << animationName);
        return;
    }

    PlayingAnimation layerAnim;
    layerAnim.clip = clip;
    layerAnim.currentTime = 0.0f;
    layerAnim.speed = speed;
    layerAnim.weight = weight;
    layerAnim.loop = loop;

    m_playingAnimations.push_back(layerAnim);

    LOG_DEBUG("Added animation layer: " << animationName << " (weight: " << weight << ")");
}

void AnimationController::RemoveLayer(int layerIndex) {
    if (layerIndex >= 0 && layerIndex < static_cast<int>(m_playingAnimations.size())) {
        m_playingAnimations.erase(m_playingAnimations.begin() + layerIndex);
        LOG_DEBUG("Removed animation layer: " << layerIndex);
    }
}

void AnimationController::SetLayerWeight(int layerIndex, float weight) {
    if (layerIndex >= 0 && layerIndex < static_cast<int>(m_playingAnimations.size())) {
        m_playingAnimations[layerIndex].weight = weight;
    }
}

// State machine
void AnimationController::AddState(const std::string& stateName, const std::string& clipName, bool loop, float speed) {
    auto clip = GetAnimationClip(clipName);
    if (!clip) {
        LOG_ERROR("Cannot create state '" << stateName << "' - animation clip '" << clipName << "' not found");
        return;
    }

    AnimationState state;
    state.name = stateName;
    state.clip = clip;
    state.loop = loop;
    state.speed = speed;

    m_states[stateName] = state;

    // Set as current state if it's the first one
    if (m_currentState.empty()) {
        m_currentState = stateName;
    }

    LOG_DEBUG("Animation state added: " << stateName);
}

void AnimationController::AddTransition(const std::string& fromState, const std::string& toState, float duration) {
    AnimationTransition transition;
    transition.fromState = fromState;
    transition.toState = toState;
    transition.duration = duration;

    m_transitions.push_back(transition);

    LOG_DEBUG("Animation transition added: " << fromState << " -> " << toState);
}

void AnimationController::SetTrigger(const std::string& triggerName) {
    m_triggers[triggerName] = true;
    LOG_DEBUG("Animation trigger set: " << triggerName);
}

void AnimationController::TransitionToState(const std::string& stateName) {
    if (m_states.find(stateName) == m_states.end()) {
        LOG_ERROR("Animation state not found: " << stateName);
        return;
    }

    if (m_currentState == stateName) {
        return; // Already in target state
    }

    m_targetState = stateName;
    m_useStateMachine = true;

    // Find and execute transition
    auto transitions = FindTransitionsFromState(m_currentState);
    for (auto* transition : transitions) {
        if (transition->toState == stateName) {
            StartTransition(*transition);
            break;
        }
    }

    LOG_DEBUG("Transitioning to state: " << stateName);
}

// Animation parameters
void AnimationController::SetFloat(const std::string& name, float value) {
    m_floatParams[name] = value;
}

void AnimationController::SetInt(const std::string& name, int value) {
    m_intParams[name] = value;
}

void AnimationController::SetBool(const std::string& name, bool value) {
    m_boolParams[name] = value;
}

float AnimationController::GetFloat(const std::string& name) const {
    auto it = m_floatParams.find(name);
    return (it != m_floatParams.end()) ? it->second : 0.0f;
}

int AnimationController::GetInt(const std::string& name) const {
    auto it = m_intParams.find(name);
    return (it != m_intParams.end()) ? it->second : 0;
}

bool AnimationController::GetBool(const std::string& name) const {
    auto it = m_boolParams.find(name);
    return (it != m_boolParams.end()) ? it->second : false;
}

// Animation info
float AnimationController::GetCurrentTime() const {
    if (!m_playingAnimations.empty()) {
        return m_playingAnimations[0].currentTime;
    }
    return 0.0f;
}

float AnimationController::GetNormalizedTime() const {
    if (!m_playingAnimations.empty() && m_playingAnimations[0].clip) {
        float duration = m_playingAnimations[0].clip->GetDuration();
        return (duration > 0.0f) ? (m_playingAnimations[0].currentTime / duration) : 0.0f;
    }
    return 0.0f;
}

std::string AnimationController::GetCurrentAnimationName() const {
    if (!m_playingAnimations.empty() && m_playingAnimations[0].clip) {
        return m_playingAnimations[0].clip->GetName();
    }
    return "";
}

// Private methods
void AnimationController::UpdateAnimations(float deltaTime) {
    for (auto& playingAnim : m_playingAnimations) {
        if (!playingAnim.clip) continue;

        // Update animation time
        playingAnim.currentTime += deltaTime * playingAnim.speed;

        // Handle looping
        if (playingAnim.loop) {
            playingAnim.currentTime = playingAnim.clip->LoopTime(playingAnim.currentTime);
        } else {
            playingAnim.currentTime = playingAnim.clip->NormalizeTime(playingAnim.currentTime);
        }

        // Update blending
        if (playingAnim.isBlending) {
            playingAnim.blendTime += deltaTime;

            if (playingAnim.blendTime >= playingAnim.blendDuration) {
                // Blending finished
                playingAnim.isBlending = false;
                playingAnim.weight = (playingAnim.weight > 0.5f) ? 1.0f : 0.0f;
            } else {
                // Update blend weight
                float t = playingAnim.blendTime / playingAnim.blendDuration;
                if (playingAnim.weight < 0.5f) {
                    // Fading out
                    playingAnim.weight = 1.0f - t;
                } else {
                    // Fading in
                    playingAnim.weight = t;
                }
            }
        }
    }
}

void AnimationController::UpdateStateMachine(float deltaTime) {
    if (m_currentState.empty()) return;

    // Update current state animation
    AnimationState* currentState = FindState(m_currentState);
    if (currentState && currentState->clip) {
        // Update or create playing animation for current state
        if (m_playingAnimations.empty()) {
            PlayingAnimation playingAnim;
            playingAnim.clip = currentState->clip;
            playingAnim.currentTime = 0.0f;
            playingAnim.speed = currentState->speed;
            playingAnim.weight = 1.0f;
            playingAnim.loop = currentState->loop;
            m_playingAnimations.push_back(playingAnim);
        }

        UpdateAnimations(deltaTime);
    }

    // Check for transitions
    CheckTransitions();
}

void AnimationController::BlendAnimations() {
    if (m_playingAnimations.empty() || m_boneTransforms.empty()) return;

    // Reset bone transforms
    std::fill(m_boneTransforms.begin(), m_boneTransforms.end(), DirectX::XMMatrixIdentity());

    // Temporary storage for blended transforms
    std::vector<DirectX::XMMATRIX> tempTransforms(m_boneTransforms.size(), DirectX::XMMatrixIdentity());

    // Normalize weights
    NormalizeWeights();

    // Blend all playing animations
    for (const auto& playingAnim : m_playingAnimations) {
        if (!playingAnim.clip || playingAnim.weight <= 0.0f) continue;

        // Sample animation
        std::vector<DirectX::XMMATRIX> animTransforms(m_boneTransforms.size(), DirectX::XMMatrixIdentity());
        playingAnim.clip->SampleAnimation(playingAnim.currentTime, animTransforms);

        // Blend with existing transforms
        for (size_t i = 0; i < m_boneTransforms.size(); i++) {
            if (playingAnim.weight >= 1.0f) {
                m_boneTransforms[i] = animTransforms[i];
            } else {
                // Linear blend (simplified - should use proper matrix interpolation)
                DirectX::XMMATRIX blended = DirectX::XMMatrixMultiply(
                    DirectX::XMMatrixScaling(1.0f - playingAnim.weight, 1.0f - playingAnim.weight, 1.0f - playingAnim.weight),
                    m_boneTransforms[i]
                );
                blended = DirectX::XMMatrixAdd(blended,
                    DirectX::XMMatrixMultiply(
                        DirectX::XMMatrixScaling(playingAnim.weight, playingAnim.weight, playingAnim.weight),
                        animTransforms[i]
                    )
                );
                m_boneTransforms[i] = blended;
            }
        }
    }
}

void AnimationController::CheckTransitions() {
    auto transitions = FindTransitionsFromState(m_currentState);

    for (auto* transition : transitions) {
        bool shouldTransition = false;

        // Check trigger condition
        if (!transition->triggerName.empty()) {
            auto it = m_triggers.find(transition->triggerName);
            if (it != m_triggers.end() && it->second) {
                shouldTransition = true;
                m_triggers[transition->triggerName] = false; // Reset trigger
            }
        }

        // Check exit time condition
        if (transition->hasExitTime) {
            float normalizedTime = GetNormalizedTime();
            if (normalizedTime >= transition->exitTime) {
                shouldTransition = true;
            }
        }

        if (shouldTransition) {
            StartTransition(*transition);
            break;
        }
    }
}

void AnimationController::StartTransition(const AnimationTransition& transition) {
    AnimationState* targetState = FindState(transition.toState);
    if (!targetState) return;

    // Start cross-fade to new state
    CrossFade(targetState->clip->GetName(), transition.duration, targetState->loop, targetState->speed);

    m_currentState = transition.toState;
    m_targetState.clear();

    LOG_DEBUG("Started transition to state: " << transition.toState);
}

void AnimationController::CleanupFinishedAnimations() {
    m_playingAnimations.erase(
        std::remove_if(m_playingAnimations.begin(), m_playingAnimations.end(),
            [](const PlayingAnimation& anim) {
                // Remove animations with zero weight that are not blending
                return anim.weight <= 0.0f && !anim.isBlending;
            }),
        m_playingAnimations.end()
    );
}

void AnimationController::NormalizeWeights() {
    float totalWeight = 0.0f;
    for (const auto& anim : m_playingAnimations) {
        totalWeight += anim.weight;
    }

    if (totalWeight > 1.0f) {
        float scale = 1.0f / totalWeight;
        for (auto& anim : m_playingAnimations) {
            anim.weight *= scale;
        }
    }
}

AnimationState* AnimationController::FindState(const std::string& name) {
    auto it = m_states.find(name);
    return (it != m_states.end()) ? &it->second : nullptr;
}

std::vector<AnimationTransition*> AnimationController::FindTransitionsFromState(const std::string& stateName) {
    std::vector<AnimationTransition*> result;

    for (auto& transition : m_transitions) {
        if (transition.fromState == stateName) {
            result.push_back(&transition);
        }
    }

    return result;
}

} // namespace Animation
} // namespace GameEngine

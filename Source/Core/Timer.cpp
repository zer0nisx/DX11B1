#include "Timer.h"
#include <algorithm>

namespace GameEngine {
namespace Core {

Timer::Timer()
    : m_deltaTime(0.0f)
    , m_totalTime(0.0f)
    , m_fps(0.0f)
    , m_frameCount(0)
    , m_fpsFrameCount(0)
    , m_fpsTimeAccumulator(0.0f)
    , m_isPaused(false)
    , m_isStarted(false)
{
}

void Timer::Start() {
    auto now = std::chrono::high_resolution_clock::now();

    m_startTime = now;
    m_currentTime = now;
    m_lastTime = now;

    m_deltaTime = 0.0f;
    m_totalTime = 0.0f;
    m_fps = 0.0f;
    m_frameCount = 0;
    m_fpsFrameCount = 0;
    m_fpsTimeAccumulator = 0.0f;

    m_isPaused = false;
    m_isStarted = true;
}

void Timer::Update() {
    if (!m_isStarted || m_isPaused) {
        m_deltaTime = 0.0f;
        return;
    }

    m_currentTime = std::chrono::high_resolution_clock::now();

    // Calculate delta time
    auto deltaTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        m_currentTime - m_lastTime).count();
    m_deltaTime = static_cast<float>(deltaTimeNs) / 1000000000.0f;

    // Cap delta time to prevent large jumps (e.g., when debugging)
    m_deltaTime = std::min(m_deltaTime, 1.0f / 30.0f); // Max 30 FPS minimum

    // Calculate total time
    auto totalTimeNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        m_currentTime - m_startTime).count();
    m_totalTime = static_cast<float>(totalTimeNs) / 1000000000.0f;

    m_lastTime = m_currentTime;
    m_frameCount++;

    // Update FPS
    UpdateFPS();
}

void Timer::Pause() {
    if (!m_isStarted || m_isPaused) {
        return;
    }

    m_pauseTime = std::chrono::high_resolution_clock::now();
    m_isPaused = true;
}

void Timer::Resume() {
    if (!m_isStarted || !m_isPaused) {
        return;
    }

    auto resumeTime = std::chrono::high_resolution_clock::now();
    auto pauseDuration = resumeTime - m_pauseTime;

    // Adjust start time to account for pause duration
    m_startTime += pauseDuration;
    m_lastTime = resumeTime;

    m_isPaused = false;
}

void Timer::Reset() {
    Start();
}

void Timer::UpdateFPS() {
    m_fpsFrameCount++;
    m_fpsTimeAccumulator += m_deltaTime;

    // Update FPS every second
    if (m_fpsTimeAccumulator >= 1.0f) {
        m_fps = static_cast<float>(m_fpsFrameCount) / m_fpsTimeAccumulator;
        m_fpsFrameCount = 0;
        m_fpsTimeAccumulator = 0.0f;
    }
}

} // namespace Core
} // namespace GameEngine

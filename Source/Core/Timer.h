#pragma once
#include <chrono>

namespace GameEngine {
namespace Core {

class Timer {
public:
    Timer();

    void Start();
    void Update();
    void Pause();
    void Resume();
    void Reset();

    float GetDeltaTime() const { return m_deltaTime; }
    float GetTotalTime() const { return m_totalTime; }
    float GetFPS() const { return m_fps; }
    int GetFrameCount() const { return m_frameCount; }

    bool IsPaused() const { return m_isPaused; }

private:
    void UpdateFPS();

    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_currentTime;
    std::chrono::high_resolution_clock::time_point m_lastTime;
    std::chrono::high_resolution_clock::time_point m_pauseTime;

    float m_deltaTime;
    float m_totalTime;
    float m_fps;

    int m_frameCount;
    int m_fpsFrameCount;
    float m_fpsTimeAccumulator;

    bool m_isPaused;
    bool m_isStarted;
};

} // namespace Core
} // namespace GameEngine

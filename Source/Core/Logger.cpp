#include "Logger.h"
#include <chrono>
#include <iomanip>

namespace GameEngine {
namespace Core {

Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    Shutdown();
}

void Logger::Initialize(const std::string& filename, LogLevel minLevel) {
    std::lock_guard<std::mutex> lock(m_logMutex);

    if (m_initialized) {
        return;
    }

    m_minLogLevel = minLevel;

    // Open log file
    m_logFile.open(filename, std::ios::out | std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        return;
    }

    m_initialized = true;

    // Log initialization message
    Log(LogLevel::Info, "Logger initialized");
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(m_logMutex);

    if (!m_initialized) {
        return;
    }

    Log(LogLevel::Info, "Logger shutting down");

    if (m_logFile.is_open()) {
        m_logFile.close();
    }

    m_initialized = false;
}

void Logger::LogDebug(const std::string& message) {
    Log(LogLevel::Debug, message);
}

void Logger::LogInfo(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::LogWarning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void Logger::LogError(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::Log(LogLevel level, const std::string& message) {
    if (level < m_minLogLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_logMutex);

    if (!m_initialized) {
        return;
    }

    std::string logEntry = GetTimeStamp() + " [" + LogLevelToString(level) + "] " + message;

    // Write to console
    std::cout << logEntry << std::endl;

    // Write to file
    if (m_logFile.is_open()) {
        m_logFile << logEntry << std::endl;
        m_logFile.flush();
    }
}

std::string Logger::GetTimeStamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string Logger::LogLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO ";
        case LogLevel::Warning: return "WARN ";
        case LogLevel::Error:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

} // namespace Core
} // namespace GameEngine

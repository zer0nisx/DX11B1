#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>

namespace GameEngine {
	namespace Core {
		enum class LogLevel {
			Debug = 0,
			Info = 1,
			Warning = 2,
			Error = 3
		};

		class Logger {
		public:
			static Logger& GetInstance();

			void Initialize(const std::string& filename = "engine.log", LogLevel minLevel = LogLevel::Info);
			void Shutdown();

			void LogDebug(const std::string& message);
			void LogInfo(const std::string& message);
			void LogWarning(const std::string& message);
			void LogError(const std::string& message);

			void SetMinLogLevel(LogLevel level) { m_minLogLevel = level; }
			LogLevel GetMinLogLevel() const { return m_minLogLevel; }

			void SetEnabled(bool enabled) { m_enabled = enabled; }
			bool IsEnabled() const { return m_enabled; }

		private:
			Logger() = default;
			~Logger();

			void Log(LogLevel level, const std::string& message);
			std::string GetTimeStamp() const;
			std::string LogLevelToString(LogLevel level) const;

			std::ofstream m_logFile;
			LogLevel m_minLogLevel = LogLevel::Info;
			std::recursive_mutex m_logMutex;
			bool m_initialized = false;
			bool m_enabled = true;
		};

		// Macros for convenient logging
#define LOG_DEBUG(msg) do { \
    std::stringstream ss; \
    ss << msg; \
    GameEngine::Core::Logger::GetInstance().LogDebug(ss.str()); \
} while(0)

#define LOG_INFO(msg) do { \
    std::stringstream ss; \
    ss << msg; \
    GameEngine::Core::Logger::GetInstance().LogInfo(ss.str()); \
} while(0)

#define LOG_WARNING(msg) do { \
    std::stringstream ss; \
    ss << msg; \
    GameEngine::Core::Logger::GetInstance().LogWarning(ss.str()); \
} while(0)

#define LOG_ERROR(msg) do { \
    std::stringstream ss; \
    ss << msg; \
    GameEngine::Core::Logger::GetInstance().LogError(ss.str()); \
} while(0)
	} // namespace Core
} // namespace GameEngine

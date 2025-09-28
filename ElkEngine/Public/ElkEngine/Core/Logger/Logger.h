#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <format>

#include "Core/Logger/LogLevel.h"
#include "Core/Logger/Details/LogBuffer.h"
#include "Core/Logger/Details/ILogSink.h"

namespace elk::logger {
	class LogBuffer;
}

namespace elk {
	class Logger {
	public:
		Logger() = default;
		~Logger() = default;

	public:
		bool Initialize(const std::string_view& log_file_path = "game.log");

		template<typename... Args>
		void LogTrace(const char* file, int line, const char* func,
			std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			LogImple(file, line, func, system, LogLevel::Trace, fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void LogDebug(const char* file, int line, const char* func,
			std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			LogImple(file, line, func, system, LogLevel::Debug, fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void LogInfo(const char* file, int line, const char* func,
			std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			LogImple(file, line, func, system, LogLevel::Info, fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void LogWarn(const char* file, int line, const char* func,
			std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			LogImple(file, line, func, system, LogLevel::Warn, fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void LogError(const char* file, int line, const char* func,
			std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			LogImple(file, line, func, system, LogLevel::Error, fmt, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void LogCritical(const char* file, int line, const char* func,
			std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			LogImple(file, line, func, system, LogLevel::Critical, fmt, std::forward<Args>(args)...);
		}
		
		void ClearGameLogs() {}

		// 手動フラッシュ（重要なログの場合）
		void Flush() {}

		// ログレベル動的変更
		void SetLogLevel(LogLevel level) {
			m_logLevel.store(level);
		}

	private:
		template<typename... Args>
		void LogImple(const char* file, int line, const char* func,
			std::string_view system,
			LogLevel level,
			std::format_string<Args...> fmt, Args&&... args) {

			if (!IsLogLevelEnabled(level)) {
				return;
			}

			std::string message = std::format(fmt, args...);
			LogImpl(file, line, func, system, level, message);
		}

		void LogImpl(const char* fileName, int line, const char* func, std::string_view system, LogLevel level, const std::string& message);

		bool IsLogLevelEnabled(LogLevel level) const {
			return level >= m_logLevel.load(std::memory_order_relaxed);
		}

	private:
		std::unique_ptr <elk::logger::LogBuffer> m_logBuffer;
		std::vector<std::shared_ptr<logger::ILogSink>> m_sinks;
		std::atomic<LogLevel> m_logLevel = LogLevel::Info;
	};

} // namespace elk
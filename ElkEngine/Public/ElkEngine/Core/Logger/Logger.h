#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <format>

#include "Core/Logger/LogLevel.h"
#include "Core/Logger/LogBuffer.h"

namespace elk::logger {
	class LogBuffer;
}

namespace elk {
	class Logger {
	public:
		Logger();
		~Logger() = default;
		

	public:
		template<typename... Args>
		void Log(const char* fileName, int line, const std::string& tag, LogLevel level, std::format_string<Args...> fmtstr, Args&&... args) {
			auto body = std::format(fmtstr, std::forward<Args>(args)...);
			LogImpl(fileName, line, tag, level, body);
		}

	private:
		void LogImpl(const char* fileName, int line, const std::string& tag, LogLevel level, const std::string& message);

	private:
		std::unique_ptr <elk::logger::LogBuffer> m_logBuffer;
	};

} // namespace elk
#include "Core/Logger/Logger.h"

#include <sstream>
#include <iostream>
#include <mutex>
#include <format>
#include <string>
#include <string_view>

namespace elk {
	
	void LogRaw(LogLevel level, const std::string& message) {
		// fallback
		const char* lvl = "INFO";
		switch (level) {
		case LogLevel::Debug: lvl = "DEBUG"; break;
		case LogLevel::Warn: lvl = "WARN"; break;
		case LogLevel::Error: lvl = "ERROR"; break;
		case LogLevel::Critical: lvl = "CRIT"; break;
		default: lvl = "INFO"; break;
		}
		std::cerr << "[" << lvl << "] " << message << std::endl;
	}

#if 0
	Logger::Logger() {
		m_logBuffer = std::make_unique<elk::logger::LogBuffer>();
		m_logBuffer->Initialize(100 * 1024 * 1024, 1000000); // 100MB, 1,000,000 messages
	}

	void Logger::LogImpl(const char* fileName, int line, const std::string& tag, LogLevel level, const std::string& message)
	{
		m_logBuffer->Add(level, tag, message, 0);
		LogRaw(level, std::format("{} ({}:{})", message, fileName, line));
	}
#endif

} // namespace elk::core
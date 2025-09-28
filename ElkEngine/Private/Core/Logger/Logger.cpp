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

	bool Logger::Initialize(const std::string_view& log_file_path) {
		m_logBuffer = std::make_unique<elk::logger::LogBuffer>();
		m_logBuffer->Initialize(100 * 1024 * 1024, 1000000); // 100MB, 1,000,000 messages

		return true;
	}

	void Logger::LogImpl(const char* fileName, int line, const char* func, std::string_view system, LogLevel level, const std::string& message)
	{
		(void*)func; // 未使用
		m_logBuffer->Add(level, system, message, 0);
		LogRaw(level, std::format("{} ({}:{})", message, fileName, line));
	}

} // namespace elk::core
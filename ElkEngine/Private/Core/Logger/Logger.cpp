#include "Core/Logger/Logger.h"

#include <sstream>
#include <iostream>
#include <mutex>
#include <format>
#include <string>
#include <string_view>
#include <chrono>

#include "Core/Logger/Details/LogBuffer.h"

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
		m_logBuffer->Initialize(100 *1024 *1024,1000000); //100MB,1,000,000 messages

		return true;
	}

	void Logger::AddSink(std::shared_ptr<logger::ILogSink> sink)
	{
		if (!sink) return;
		std::lock_guard<std::mutex> lk(m_sinksMutex);
		m_sinks.push_back(std::move(sink));
	}

	void Logger::LogImpl(const char* fileName, int line, const char* func, std::string_view system, LogLevel level, const std::string& message)
	{
		(void*)func; // 未使用
		m_logBuffer->Add(level, system, message,0);
		LogRaw(level, std::format("{} ({}:{})", message, fileName, line));

		// Copy sinks under lock to avoid holding lock during write callbacks
		std::vector<std::shared_ptr<logger::ILogSink>> sinksCopy;
		{
			std::lock_guard<std::mutex> lk(m_sinksMutex);
			sinksCopy = m_sinks;
		}

		// Prepare a LogMessage to send to sinks. Try to obtain accurate metadata from LogBuffer.
		logger::LogMessage msg{};
		msg.level = level;
		msg.offset =0;
		msg.length = message.size();
		msg.tagId =0;
		msg.frameNumber =0;
		msg.timestamp = std::chrono::system_clock::now();
		msg.message = std::string_view(message);

		if (m_logBuffer) {
			size_t count = m_logBuffer->Count();
			if (count >0) {
				// At(count-1) returns a snapshot of the last stored LogMessage
				auto bufMsg = m_logBuffer->At(count -1);
				msg.level = bufMsg.level;
				msg.offset = bufMsg.offset;
				msg.length = bufMsg.length;
				msg.tagId = bufMsg.tagId;
				msg.frameNumber = bufMsg.frameNumber;
				msg.timestamp = bufMsg.timestamp;
				msg.message = bufMsg.message; // view into LogBuffer's internal ring buffer
			}
		}

		for (auto& s : sinksCopy) {
			if (s) {
				try {
					s->Write(msg);
				}
				catch (...) {
					// swallow exceptions from sinks to avoid crashing the logger
				}
			}
		}
	}

} // namespace elk::core
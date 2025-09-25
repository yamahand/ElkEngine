#include "Core/Logger/Logger.h"

#include <sstream>
#include <iostream>
#include <mutex>
#include <format>
#include <string>
#include <string_view>

#ifdef ELK_USE_SPDLOG
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#endif

namespace elk {

	namespace {
		std::mutex g_logMutex;
#ifdef ELK_USE_SPDLOG
		std::shared_ptr<spdlog::logger> g_logger = nullptr;
#endif
	}

	void InitLogger(const std::string& filename, LogLevel level) {
		std::lock_guard lk(g_logMutex);
#ifdef ELK_USE_SPDLOG
		try {
			auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
			if (!filename.empty()) {
				auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, 1024 * 1024 * 5, 3);
				g_logger = std::make_shared<spdlog::logger>("elk", spdlog::sinks_init_list{ console_sink, file_sink });
			}
			else {
				g_logger = std::make_shared<spdlog::logger>("elk", console_sink);
			}
			spdlog::set_default_logger(g_logger);
			g_logger->set_level(spdlog::level::info);
		}
		catch (...) {
			g_logger = nullptr;
		}
#else
		(void)filename; (void)level;
#endif
	}

	void ShutdownLogger() {
		std::lock_guard lk(g_logMutex);
#ifdef ELK_USE_SPDLOG
		spdlog::drop_all();
		g_logger.reset();
#endif
	}

	void LogRaw(LogLevel level, const std::string& message) {
		std::lock_guard lk(g_logMutex);
#ifdef ELK_USE_SPDLOG
		if (g_logger) {
			switch (level) {
			case LogLevel::Trace:    g_logger->trace(message); break;
			case LogLevel::Debug:    g_logger->debug(message); break;
			case LogLevel::Info:     g_logger->info(message); break;
			case LogLevel::Warn:     g_logger->warn(message); break;
			case LogLevel::Error:    g_logger->error(message); break;
			case LogLevel::Critical: g_logger->critical(message); break;
			default:                 g_logger->info(message); break;
			}
			return;
		}
#endif
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

	Logger::Logger() {
		m_logBuffer = std::make_unique<elk::logger::LogBuffer>();
		m_logBuffer->Initialize(100 * 1024 * 1024, 1000000); // 100MB, 1,000,000 messages
	}

	void Logger::LogImpl(const char* fileName, int line, const std::string& tag, LogLevel level, const std::string& message)
	{
		m_logBuffer->Add(level, tag, message, 0);
		LogRaw(level, std::format("{} ({}:{})", message, fileName, line));
	}

} // namespace elk::core
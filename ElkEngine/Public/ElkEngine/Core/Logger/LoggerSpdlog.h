#pragma once
#ifdef ELK_USE_SPDLOG
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <deque>
#include <functional>

#include "Core/Logger/LogLevel.h"

#include "spdlog/spdlog.h"

namespace elk {
	// インゲームログウィンドウ用のカスタムシンク
	class GameWindowSink;
	// ログエントリ構造体
	struct LogEntry;

	// spdlog 実装
	class SpdLogSystem {
	private:
		std::shared_ptr<spdlog::logger> multi_logger_;
		std::shared_ptr<GameWindowSink> game_sink_;
		bool initialized_;

	public:
		SpdLogSystem() : initialized_(false) {}

		~SpdLogSystem() {
			if (initialized_) {
				spdlog::shutdown();
			}
		}

		bool Initialize(const std::string_view& log_file_path = "game.log");

		template<typename... Args>
		void LogTrace(const char* file, int line, const char* func,
			[[maybe_unused]] std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			if (multi_logger_) {
				spdlog::source_loc loc{ file, line, func };
				multi_logger_->log(loc, spdlog::level::trace, fmt, std::forward<Args>(args)...);
			}
		}
		template<typename... Args>
		void LogDebug(const char* file, int line, const char* func,
			[[maybe_unused]] std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			if (multi_logger_) {
				spdlog::source_loc loc{ file, line, func };
				multi_logger_->log(loc, spdlog::level::debug, fmt, std::forward<Args>(args)...);
			}
		}
		template<typename... Args>
		void LogInfo(const char* file, int line, const char* func,
			[[maybe_unused]] std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			if (multi_logger_) {
				spdlog::source_loc loc{ file, line, func };
				multi_logger_->log(loc, spdlog::level::info, fmt, std::forward<Args>(args)...);
			}
		}
		template<typename... Args>
		void LogWarn(const char* file, int line, const char* func,
			[[maybe_unused]] std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			if (multi_logger_) {
				spdlog::source_loc loc{ file, line, func };
				multi_logger_->log(loc, spdlog::level::warn, fmt, std::forward<Args>(args)...);
			}
		}
		template<typename... Args>
		void LogError(const char* file, int line, const char* func,
			[[maybe_unused]] std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			if (multi_logger_) {
				spdlog::source_loc loc{ file, line, func };
				multi_logger_->log(loc, spdlog::level::err, fmt, std::forward<Args>(args)...);
			}
		}
		template<typename... Args>
		void LogCritical(const char* file, int line, const char* func,
			[[maybe_unused]] std::string_view system,
			std::format_string<Args...> fmt, Args&&... args) {
			if (multi_logger_) {
				spdlog::source_loc loc{ file, line, func };
				multi_logger_->log(loc, spdlog::level::critical, fmt, std::forward<Args>(args)...);
			}
		}

		// インゲームUI用のインターフェース
		void SetGameLogCallback(std::function<void(const LogEntry&)> callback);

		// NOTE: 呼び出し先が非 const になったため、ここも非 const に変更しました
		std::vector<LogEntry> GetRecentLogs(size_t count = 100);

		void ClearGameLogs();

		// 手動フラッシュ（重要なログの場合）
		void Flush() {
			if (multi_logger_) {
				multi_logger_->flush();
			}
		}

		// ログレベル動的変更
		void SetLogLevel(LogLevel level) {
			spdlog::level::level_enum logLevel = spdlog::level::level_enum::info;
			// LogLevelをspdlog::level::level_enumに変換
			switch (level) {
			case LogLevel::Trace:
				logLevel = spdlog::level::trace;
				break;
			case LogLevel::Debug:
				logLevel = spdlog::level::debug;
				break;
			case LogLevel::Info:
				logLevel = spdlog::level::info;
				break;
			case LogLevel::Warn:
				logLevel = spdlog::level::warn;
				break;
			case LogLevel::Error:
				logLevel = spdlog::level::err;
				break;
			case LogLevel::Critical:
				logLevel = spdlog::level::critical;
				break;
			case LogLevel::Off:
				logLevel = spdlog::level::off;
				break;
			default:
				logLevel = spdlog::level::info;
				break;
			}

			if (multi_logger_) {
				multi_logger_->set_level(logLevel);
			}
		}
	};

} // namespace elk

#endif // ELK_USE_SPDLOG
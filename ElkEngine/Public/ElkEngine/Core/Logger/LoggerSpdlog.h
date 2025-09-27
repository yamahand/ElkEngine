#pragma once
#ifdef ELK_USE_SPDLOG
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <deque>
#include <functional>

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

		bool Initialize(const std::string& log_file_path = "game.log");

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
		void SetLogLevel(spdlog::level::level_enum level) {
			if (multi_logger_) {
				multi_logger_->set_level(level);
			}
		}
	};

} // namespace elk

#endif // ELK_USE_SPDLOG
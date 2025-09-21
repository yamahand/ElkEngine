#pragma once
#ifdef ELK_USE_SPDLOG
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <deque>
#include <functional>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

#ifdef _WIN32
#include "spdlog/sinks/msvc_sink.h"  // Visual Studio出力ウィンドウ用
#include <Windows.h>
#endif

namespace elk {

	// インゲームログウィンドウ用のカスタムシンク
	class GameWindowSink : public spdlog::sinks::base_sink<std::mutex> {
	public:
		struct LogEntry {
			spdlog::level::level_enum level;
			std::string message;
			std::chrono::system_clock::time_point timestamp;
		};

	private:
		std::deque<LogEntry> log_buffer_;
		size_t max_entries_;
		std::function<void(const LogEntry&)> on_new_log_;

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override {
			spdlog::memory_buf_t formatted;
			base_sink<std::mutex>::formatter_->format(msg, formatted);

			LogEntry entry;
			entry.level = msg.level;
			entry.message = fmt::to_string(formatted);
			entry.timestamp = msg.time;

			// バッファサイズ制限
			if (log_buffer_.size() >= max_entries_) {
				log_buffer_.pop_front();
			}

			log_buffer_.push_back(entry);

			// UI更新コールバック
			if (on_new_log_) {
				on_new_log_(entry);
			}
		}

		void flush_() override {
			// ゲームUIの場合、特別なフラッシュ処理は不要
		}

	public:
		explicit GameWindowSink(size_t max_entries = 1000)
			: max_entries_(max_entries) {
		}

		void set_new_log_callback(std::function<void(const LogEntry&)> callback) {
			std::lock_guard<std::mutex> lock(base_sink<std::mutex>::mutex_);
			on_new_log_ = callback;
		}

		// NOTE: mutex をロックするため const を外しました
		std::vector<LogEntry> get_recent_logs(size_t count = 100) {
			std::lock_guard<std::mutex> lock(base_sink<std::mutex>::mutex_);

			size_t start_idx = 0;
			if (log_buffer_.size() > count) {
				start_idx = log_buffer_.size() - count;
			}

			return std::vector<LogEntry>(
				log_buffer_.begin() + start_idx,
				log_buffer_.end()
			);
		}

		void clear_logs() {
			std::lock_guard<std::mutex> lock(base_sink<std::mutex>::mutex_);
			log_buffer_.clear();
		}
	};

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

		bool Initialize(const std::string& log_file_path = "game.log") {
			try {
				// 非同期ログ用スレッドプール初期化
				spdlog::init_thread_pool(32768, 1);

				// 複数のシンクを作成
				std::vector<spdlog::sink_ptr> sinks;

				auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
					log_file_path,
					1024 * 1024 * 10,  // 10MB
					5                   // 最大5ファイル
				);
				file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");
				sinks.push_back(file_sink);

#ifdef _WIN32
				auto vs_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#ifdef _DEBUG
				vs_sink->set_pattern("%s(%#): [%l] %v");  // デバッグ時はジャンプ可能
#else
				vs_sink->set_pattern("[%l] %v");          // リリース時はシンプル
#endif
				sinks.push_back(vs_sink);
#endif

				game_sink_ = std::make_shared<GameWindowSink>(1000);
				game_sink_->set_pattern("[%H:%M:%S] [%L] %v");
				sinks.push_back(game_sink_);

				multi_logger_ = std::make_shared<spdlog::async_logger>(
					"game_multi",
					sinks.begin(),
					sinks.end(),
					spdlog::thread_pool(),
					spdlog::async_overflow_policy::block
				);

#ifdef _DEBUG
				multi_logger_->set_level(spdlog::level::debug);
#else
				multi_logger_->set_level(spdlog::level::info);
#endif

				multi_logger_->flush_on(spdlog::level::err);

				spdlog::register_logger(multi_logger_);
				spdlog::set_default_logger(multi_logger_);

				initialized_ = true;

				// 初期化完了ログ（フォーマット済み呼び出しを使うので非テンプレート版を呼ぶ）
				LogInfo("SpdLogSystem", "ログシステムが初期化されました");

				return true;
			}
			catch (const spdlog::spdlog_ex& ex) {
#ifdef _WIN32
				OutputDebugStringA(("ログシステム初期化エラー: " + std::string(ex.what()) + "\n").c_str());
#endif
				return false;
			}
		}

		// 既存のテンプレート形式はそのまま維持（フォーマット付き呼び出し用）
		template<typename... Args>
		void LogTrace(const std::string& system, const std::string& format, Args&&... args) {
			if (multi_logger_) {
				multi_logger_->trace("[{}] " + format, system, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		void LogDebug(const std::string& system, const std::string& format, Args&&... args) {
			if (multi_logger_) {
				multi_logger_->debug("[{}] " + format, system, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		void LogInfo(const std::string& system, const std::string& format, Args&&... args) {
			if (multi_logger_) {
				multi_logger_->info("[{}] " + format, system, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		void LogWarn(const std::string& system, const std::string& format, Args&&... args) {
			if (multi_logger_) {
				multi_logger_->warn("[{}] " + format, system, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		void LogError(const std::string& system, const std::string& format, Args&&... args) {
			if (multi_logger_) {
				multi_logger_->error("[{}] " + format, system, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		void LogCritical(const std::string& system, const std::string& format, Args&&... args) {
			if (multi_logger_) {
				multi_logger_->critical("[{}] " + format, system, std::forward<Args>(args)...);
			}
		}

		// non-template のシンプルな受け口（concept が要求するシグネチャを満たす）
		void LogTrace(const std::string& system, const std::string& message) {
			if (multi_logger_) multi_logger_->trace("[{}] {}", system, message);
		}
		void LogDebug(const std::string& system, const std::string& message) {
			if (multi_logger_) multi_logger_->debug("[{}] {}", system, message);
		}
		void LogInfo(const std::string& system, const std::string& message) {
			if (multi_logger_) multi_logger_->info("[{}] {}", system, message);
		}
		void LogWarn(const std::string& system, const std::string& message) {
			if (multi_logger_) multi_logger_->warn("[{}] {}", system, message);
		}
		void LogError(const std::string& system, const std::string& message) {
			if (multi_logger_) multi_logger_->error("[{}] {}", system, message);
		}
		void LogCritical(const std::string& system, const std::string& message) {
			if (multi_logger_) multi_logger_->critical("[{}] {}", system, message);
		}

		// インゲームUI用のインターフェース
		void SetGameLogCallback(std::function<void(const GameWindowSink::LogEntry&)> callback) {
			if (game_sink_) {
				game_sink_->set_new_log_callback(callback);
			}
		}

		// NOTE: 呼び出し先が非 const になったため、ここも非 const に変更しました
		std::vector<GameWindowSink::LogEntry> GetRecentLogs(size_t count = 100) {
			if (game_sink_) {
				return game_sink_->get_recent_logs(count);
			}
			return {};
		}

		void ClearGameLogs() {
			if (game_sink_) {
				game_sink_->clear_logs();
			}
		}

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
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
#if false
	class Logger {
	public:
		Logger();
		~Logger() = default;
		static Logger& GetInstance() {
			static std::unique_ptr<Logger> instance;
			static std::once_flag flag;
			std::call_once(flag, []() {
				instance.reset(new Logger);
				});
			return *instance;
		}

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
#endif

	// 初期化/終了（実装側で LogWithMeta を使って出力する）
	void InitLogger(const std::string& filename = "", LogLevel level = LogLevel::Info);
	void ShutdownLogger();

	// 低レベルアクセス（テスト/拡張用）
	// 既存の LogRaw は互換のため残す（シンプルなメッセージ出力）
	void LogRaw(LogLevel level, const std::string& message);

	// 新しいマクロ：カテゴリ／サブカテゴリを文字列で指定して LogWithMeta を呼ぶ。
	// ファイル名・行番号はマクロ側で自動付与。
#define ELK_LOG_WITH_META(file, line, level, tag, ...) \
    do { \
        \
    } while(0)

// レベル別の簡易カテゴリ指定マクロ
#define ELK_LOG_INFO(tag, ...)  ELK_LOG_WITH_META(__FILE__, __LINE__, elk::LogLevel::Info,    tag, __VA_ARGS__)
#define ELK_LOG_WARN(tag, ...)  ELK_LOG_WITH_META(__FILE__, __LINE__, elk::LogLevel::Warn,    tag, __VA_ARGS__)
#define ELK_LOG_ERROR(tag, ...) ELK_LOG_WITH_META(__FILE__, __LINE__, elk::LogLevel::Error,   tag, __VA_ARGS__)
#define ELK_LOG_DEBUG(tag, ...) ELK_LOG_WITH_META(__FILE__, __LINE__, elk::LogLevel::Debug,   tag, __VA_ARGS__)
#define ELK_LOG_TRACE(tag, ...) ELK_LOG_WITH_META(__FILE__, __LINE__, elk::LogLevel::Trace,   tag, __VA_ARGS__)
#define ELK_LOG_CRIT(tag, ...)  ELK_LOG_WITH_META(__FILE__, __LINE__, elk::LogLevel::Critical,tag, __VA_ARGS__)

} // namespace elk
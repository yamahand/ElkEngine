#pragma once

#include <string>
#include <memory>

namespace elk::core {
enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical, Off };

// 初期化/終了
void InitLogger(const std::string& filename = "", LogLevel level = LogLevel::Info);
void ShutdownLogger();

// 低レベルアクセス（テスト/拡張用）
void LogRaw(LogLevel level, const std::string& message);

// 便利マクロ
#define ELK_LOG_TRACE(...) elk::core::LogRaw(elk::core::LogLevel::Trace,  (std::ostringstream() << __VA_ARGS__).str())
#define ELK_LOG_DEBUG(...) elk::core::LogRaw(elk::core::LogLevel::Debug,  (std::ostringstream() << __VA_ARGS__).str())
#define ELK_LOG_INFO(...)  elk::core::LogRaw(elk::core::LogLevel::Info,   (std::ostringstream() << __VA_ARGS__).str())
#define ELK_LOG_WARN(...)  elk::core::LogRaw(elk::core::LogLevel::Warn,   (std::ostringstream() << __VA_ARGS__).str())
#define ELK_LOG_ERROR(...) elk::core::LogRaw(elk::core::LogLevel::Error,  (std::ostringstream() << __VA_ARGS__).str())
#define ELK_LOG_CRIT(...)  elk::core::LogRaw(elk::core::LogLevel::Critical,(std::ostringstream() << __VA_ARGS__).str())

} // namespace elk::core
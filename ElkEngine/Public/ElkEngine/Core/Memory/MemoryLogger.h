#pragma once

#include <string_view>
#include <mutex>
#include <fstream>
#include <atomic>
#include <string>
#include <format>

#include "Core/EngineAPI.h"

namespace elk::memory {

enum class MemoryLogLevel : int {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

class ENGINE_API MemoryLogger {
public:
    static MemoryLogger& GetInstance() noexcept;

    // 初期化 / 終了
    void Initialize(MemoryLogLevel level = MemoryLogLevel::Info, const char* logFile = nullptr) noexcept;
    void Shutdown() noexcept;

    // 基本ログ
    void Log(MemoryLogLevel level, std::string_view allocatorName, std::string_view message) noexcept;

    template<typename... Args>
    void LogFormat(MemoryLogLevel level, std::string_view allocatorName, const std::string& fmt, Args&&... args) noexcept {
        if (!ShouldLog(level)) {
            return;
        }

        try {
            // C++20 std::format を使用
            std::string msg = std::format(fmt, std::forward<Args>(args)...);
            Log(level, allocatorName, msg);
        } catch (...) {
            // フォーマット失敗時は安全なフォールバック
            Log(level, allocatorName, "[FormatError]");
        }
    }

    void SetLogLevel(MemoryLogLevel level) noexcept { m_logLevel = level; }
    MemoryLogLevel GetLogLevel() const noexcept { return m_logLevel; }

    bool IsInitialized() const noexcept { return m_initialized.load(std::memory_order_acquire); }
    bool ShouldLog(MemoryLogLevel level) const noexcept { return IsInitialized() && static_cast<int>(level) >= static_cast<int>(m_logLevel); }

private:
    MemoryLogger() = default;
    ~MemoryLogger() = default;

    MemoryLogger(const MemoryLogger&) = delete;
    MemoryLogger& operator=(const MemoryLogger&) = delete;

    void WriteToConsole(MemoryLogLevel level, std::string_view allocatorName, std::string_view message) noexcept;
    void WriteToFile(MemoryLogLevel level, std::string_view allocatorName, std::string_view message) noexcept;
    const char* LevelToString(MemoryLogLevel level) const noexcept;

private:
    std::mutex m_mutex;
    std::ofstream m_logFile;
    bool m_useFile{false};
    MemoryLogLevel m_logLevel{MemoryLogLevel::Info};
    std::atomic_bool m_initialized{false};
};

// 便利マクロ
#define MEMORY_LOG_DEBUG(allocator, message) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Debug)) { \
            _ml.Log(elk::memory::MemoryLogLevel::Debug, allocator, message); \
        } \
    } while(0)

#define MEMORY_LOG_INFO(allocator, message) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Info)) { \
            _ml.Log(elk::memory::MemoryLogLevel::Info, allocator, message); \
        } \
    } while(0)

#define MEMORY_LOG_WARN(allocator, message) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Warn)) { \
            _ml.Log(elk::memory::MemoryLogLevel::Warn, allocator, message); \
        } \
    } while(0)

#define MEMORY_LOG_ERROR(allocator, message) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Error)) { \
            _ml.Log(elk::memory::MemoryLogLevel::Error, allocator, message); \
        } \
    } while(0)

// フォーマット付きマクロ（std::format を使うため fmt 形式の文字列を渡す）
#define MEMORY_LOG_INFO_F(allocator, fmt, ...) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Info)) { \
            _ml.LogFormat(elk::memory::MemoryLogLevel::Info, allocator, fmt, __VA_ARGS__); \
        } \
    } while(0)

#define MEMORY_LOG_ERROR_F(allocator, fmt, ...) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Error)) { \
            _ml.LogFormat(elk::memory::MemoryLogLevel::Error, allocator, fmt, __VA_ARGS__); \
        } \
    } while(0)

#define MEMORY_LOG_WARN_F(allocator, fmt, ...) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Warn)) { \
            _ml.LogFormat(elk::memory::MemoryLogLevel::Warn, allocator, fmt, __VA_ARGS__); \
        } \
    } while(0)

#define MEMORY_LOG_DEBUG_F(allocator, fmt, ...) \
    do { \
        auto& _ml = elk::memory::MemoryLogger::GetInstance(); \
        if (_ml.ShouldLog(elk::memory::MemoryLogLevel::Debug)) { \
            _ml.LogFormat(elk::memory::MemoryLogLevel::Debug, allocator, fmt, __VA_ARGS__); \
        } \
    } while(0)

} // namespace elk::memory

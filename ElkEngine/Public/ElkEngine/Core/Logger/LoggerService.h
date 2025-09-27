#pragma once
#include <string>
#include <mutex>
#include <utility>
#include <type_traits>
#include <functional>
#include <string_view>
#include <format>

#include "Core/Logger/LogLevel.h"

#ifdef ELK_USE_SPDLOG
#include "LoggerSpdlog.h"
#else
#error "対応するログバックエンドがありません。CMake で ELK_USE_SPDLOG を指定してください。"
#endif

namespace elk {

template<typename Backend>
class LoggerService {
public:
    // インスタンスベースのサービスとして利用するため、static メンバーを持たせない
    LoggerService() = default;
    ~LoggerService() = default;

    bool Initialize(const std::string& path = "logs/engine.log") {
        return m_backend.Initialize(path);
    }

    void SetGameLogCallback(std::function<void(const LogEntry&)> callback) {
        m_backend.SetGameLogCallback(std::move(callback));
    }

    void ClearGameLogs() {
        m_backend.ClearGameLogs();
    }
    
    template<typename... Args>
    void LogTrace(const char* file, int line, const char* func,
                  std::string_view system,
                  std::format_string<Args...> fmt, Args&&... args) {
        m_backend.LogTrace(file, line, func, system, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogInfo(const char* file, int line, const char* func,
                 std::string_view system,
                 std::format_string<Args...> fmt, Args&&... args) {
        m_backend.LogInfo(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogWarn(const char* file, int line, const char* func,
                 std::string_view system,
                 std::format_string<Args...> fmt, Args&&... args) {
        m_backend.LogWarn(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogError(const char* file, int line, const char* func,
                  std::string_view system,
                  std::format_string<Args...> fmt, Args&&... args) {
        m_backend.LogError(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogCritical(const char* file, int line, const char* func,
                     std::string_view system,
                     std::format_string<Args...> fmt, Args&&... args) {
        m_backend.LogCritical(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogDebug(const char* file, int line, const char* func,
                  std::string_view system,
                  std::format_string<Args...> fmt, Args&&... args) {
        m_backend.LogDebug(file, line, func, system, fmt, std::forward<Args>(args)...);
    }

    void Flush() {
        m_backend.Flush();
    }

    void SetLogLevel(LogLevel level) {
        m_backend.SetLogLevel(level);
    }

private:
    Backend m_backend;
};

#ifdef ELK_USE_SPDLOG
using DefaultBackend = SpdLogSystem;
#else
#error "バックエンドが未指定です"
#endif

using DefaultLoggerService = LoggerService<DefaultBackend>;

// ServiceLocator を使ってアクセスするためのヘルパーマクロ
#define LOGGER_SERVICE() (::elk::ServiceLocator::Get<::elk::DefaultLoggerService>())

#define ELK_LOG_TRACE(system, ...) do { if (auto s = LOGGER_SERVICE()) s->LogTrace(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#define ELK_LOG_INFO(system, ...)  do { if (auto s = LOGGER_SERVICE()) s->LogInfo(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#define ELK_LOG_WARN(system, ...)  do { if (auto s = LOGGER_SERVICE()) s->LogWarn(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#define ELK_LOG_ERROR(system, ...) do { if (auto s = LOGGER_SERVICE()) s->LogError(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#define ELK_LOG_CRITICAL(system, ...) do { if (auto s = LOGGER_SERVICE()) s->LogCritical(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#ifdef _DEBUG
#define ELK_LOG_DEBUG(system, ...) do { if (auto s = LOGGER_SERVICE()) s->LogDebug(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#else
#define ELK_LOG_DEBUG(system, ...) ((void)0)
#endif

} // namespace elk

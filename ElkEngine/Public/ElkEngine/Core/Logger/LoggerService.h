#pragma once
#include <string>
#include <mutex>
#include <utility>
#include <type_traits>
#include <functional>
#include <string_view>
#include <format>

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
        return backend_.Initialize(path);
    }

    // Backend へそのまま転送 (Args... が空でも OK)
    template<typename... Args>
    void LogInfo(const char* file, int line, const char* func,
                 std::string_view system,
                 std::format_string<Args...> fmt, Args&&... args) {
        backend_.LogInfo(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogWarn(const char* file, int line, const char* func,
                 std::string_view system,
                 std::format_string<Args...> fmt, Args&&... args) {
        backend_.LogWarn(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogError(const char* file, int line, const char* func,
                  std::string_view system,
                  std::format_string<Args...> fmt, Args&&... args) {
        backend_.LogError(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void LogDebug(const char* file, int line, const char* func,
                  std::string_view system,
                  std::format_string<Args...> fmt, Args&&... args) {
        backend_.LogDebug(file, line, func, system, fmt, std::forward<Args>(args)...);
    }

    void Flush() {
        backend_.Flush();
    }

private:
    Backend backend_;
};

#ifdef ELK_USE_SPDLOG
using DefaultBackend = SpdLogSystem;
#else
#error "バックエンドが未指定です"
#endif

using DefaultLoggerService = LoggerService<DefaultBackend>;

// ServiceLocator を使ってアクセスするためのヘルパーマクロ
#define LOGGER_SERVICE() (::elk::ServiceLocator::Get<::elk::DefaultLoggerService>())

#define GAME_LOG_INFO(system, ...)  do { if (auto s = LOGGER_SERVICE()) s->LogInfo(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#define GAME_LOG_WARN(system, ...)  do { if (auto s = LOGGER_SERVICE()) s->LogWarn(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#define GAME_LOG_ERROR(system, ...) do { if (auto s = LOGGER_SERVICE()) s->LogError(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#ifdef _DEBUG
#define GAME_LOG_DEBUG(system, ...) do { if (auto s = LOGGER_SERVICE()) s->LogDebug(__FILE__, __LINE__, __func__, system, __VA_ARGS__); } while(0)
#else
#define GAME_LOG_DEBUG(system, ...) ((void)0)
#endif

} // namespace elk

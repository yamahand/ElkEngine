#pragma once
#include <string>
#include <mutex>
#include <utility>
#include <type_traits>
#include <functional>
#include <string_view>
#include <format> // SPDLOG_USE_STD_FORMAT 有効時は std::format 使用

#ifdef ELK_USE_SPDLOG
#include "LoggerSpdlog.h"
#else
#error "対応するログバックエンドがありません。CMake で ELK_USE_SPDLOG を指定してください。"
#endif

namespace elk {

template<typename Backend>
class GameLogger {
public:
    inline static Backend backend_;
    inline static std::once_flag init_flag_;

    static Backend& GetInstance() {
        std::call_once(init_flag_, []() {});
        return backend_;
    }

    static bool Initialize(const std::string& path = "logs/game.log") {
        return GetInstance().Initialize(path);
    }

private:
    static constexpr const char* BaseName(const char* path) {
        const char* last = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '/' || *p == '\\')
                last = p + 1;
        }
        return last;
    }

public:
    // そのまま backend に転送 (Args... が空でも OK)
    template<typename... Args>
    static void Info(const char* file, int line, const char* func,
                     std::string_view system,
                     std::format_string<Args...> fmt, Args&&... args) {
        GetInstance().LogInfo(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void Warn(const char* file, int line, const char* func,
                     std::string_view system,
                     std::format_string<Args...> fmt, Args&&... args) {
        GetInstance().LogWarn(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void Error(const char* file, int line, const char* func,
                      std::string_view system,
                      std::format_string<Args...> fmt, Args&&... args) {
        GetInstance().LogError(file, line, func, system, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    static void Debug(const char* file, int line, const char* func,
                      std::string_view system,
                      std::format_string<Args...> fmt, Args&&... args) {
        GetInstance().LogDebug(file, line, func, system, fmt, std::forward<Args>(args)...);
    }

    static void Flush() {
        GetInstance().Flush();
    }
};

#ifdef ELK_USE_SPDLOG
using DefaultBackend = SpdLogSystem;
#else
#error "バックエンドが未指定です"
#endif

using DefaultLogger = GameLogger<DefaultBackend>;

#define GAME_LOG_INFO(system, ...)  ::elk::DefaultLogger::Info(__FILE__, __LINE__, __func__, system, __VA_ARGS__)
#define GAME_LOG_WARN(system, ...)  ::elk::DefaultLogger::Warn(__FILE__, __LINE__, __func__, system, __VA_ARGS__)
#define GAME_LOG_ERROR(system, ...) ::elk::DefaultLogger::Error(__FILE__, __LINE__, __func__, system, __VA_ARGS__)
#ifdef _DEBUG
#define GAME_LOG_DEBUG(system, ...) ::elk::DefaultLogger::Debug(__FILE__, __LINE__, __func__, system, __VA_ARGS__)
#else
#define GAME_LOG_DEBUG(system, ...) ((void)0)
#endif

} // namespace elk


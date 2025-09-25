#pragma once
#include <string>
#include <mutex>
#include <utility>
#include <format>
#include <type_traits>
#include <functional>
#include <string_view>

#ifdef ELK_USE_SPDLOG
#include "LoggerSpdlog.h"
#else
#error "対応するログバックエンドがありません。CMake で ELK_USE_SPDLOG を指定してください。"
#endif

namespace elk {

template<typename T>
concept LoggerBackend = requires(T t, const std::string& s, const std::string& m) {
    { t.Initialize(std::declval<const std::string&>()) } -> std::convertible_to<bool>;
    { t.LogInfo(s, m) } -> std::same_as<void>;
    { t.LogWarn(s, m) } -> std::same_as<void>;
    { t.LogError(s, m) } -> std::same_as<void>;
    { t.LogDebug(s, m) } -> std::same_as<void>;
};

template<LoggerBackend Backend>
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

    // --- 追加: ファイル名短縮ヘルパとメッセージ構築 ---
private:
    static constexpr const char* BaseName(const char* path) {
        const char* last = path;
        for (const char* p = path; *p; ++p) {
            if (*p == '/' || *p == '\\')
                last = p + 1;
        }
        return last;
    }

    static std::string PrefixFileLine(const char* file, int line, std::string_view body) {
        return std::format("[{}:{}] {}", BaseName(file), line, body);
    }

public:
    // 既存 API (互換)
    template<typename... Args>
    static void Info(const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        GetInstance().LogInfo(system, std::format(fmtstr, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void Warn(const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        GetInstance().LogWarn(system, std::format(fmtstr, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void Error(const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        GetInstance().LogError(system, std::format(fmtstr, std::forward<Args>(args)...));
    }
    template<typename... Args>
    static void Debug(const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        GetInstance().LogDebug(system, std::format(fmtstr, std::forward<Args>(args)...));
    }

    // --- 追加: ファイル/行付きバージョン (マクロ用) ---
    template<typename... Args>
    static void InfoFL(const char* file, int line, const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        auto body = std::format(fmtstr, std::forward<Args>(args)...);
        GetInstance().LogInfo(system, PrefixFileLine(file, line, body));
    }
    template<typename... Args>
    static void WarnFL(const char* file, int line, const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        auto body = std::format(fmtstr, std::forward<Args>(args)...);
        GetInstance().LogWarn(system, PrefixFileLine(file, line, body));
    }
    template<typename... Args>
    static void ErrorFL(const char* file, int line, const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        auto body = std::format(fmtstr, std::forward<Args>(args)...);
        GetInstance().LogError(system, PrefixFileLine(file, line, body));
    }
    template<typename... Args>
    static void DebugFL(const char* file, int line, const std::string& system, std::format_string<Args...> fmtstr, Args&&... args) {
        auto body = std::format(fmtstr, std::forward<Args>(args)...);
        GetInstance().LogDebug(system, PrefixFileLine(file, line, body));
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

    // マクロ: __FILE__ / __LINE__ を自動付与
#define GAME_LOG_INFO(system, ...)  ::elk::DefaultLogger::InfoFL(__FILE__, __LINE__, system, __VA_ARGS__)
#define GAME_LOG_WARN(system, ...)  ::elk::DefaultLogger::WarnFL(__FILE__, __LINE__, system, __VA_ARGS__)
#define GAME_LOG_ERROR(system, ...) ::elk::DefaultLogger::ErrorFL(__FILE__, __LINE__, system, __VA_ARGS__)
#ifdef _DEBUG
#define GAME_LOG_DEBUG(system, ...) ::elk::DefaultLogger::DebugFL(__FILE__, __LINE__, system, __VA_ARGS__)
#else
#define GAME_LOG_DEBUG(system, ...) ((void)0)
#endif

} // namespace elk


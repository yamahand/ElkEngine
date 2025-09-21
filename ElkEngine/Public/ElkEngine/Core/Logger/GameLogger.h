#pragma once
#include <string>
#include <mutex>
#include <utility>
#include <format>
#include <type_traits>
#include <functional>
//#include <once_flag>

#ifdef ELK_USE_SPDLOG
#include "LoggerSpdlog.h"
#else
#error "対応するログバックエンドがありません。CMake で ELK_USE_SPDLOG を指定してください。"
#endif

namespace elk {

// C++20 concepts を使ったバックエンド要求
template<typename T>
concept LoggerBackend = requires(T t, const std::string& s, const std::string& m) {
    { t.Initialize(std::declval<const std::string&>()) } -> std::convertible_to<bool>;
    { t.LogInfo(s, m) } -> std::same_as<void>;
    { t.LogWarn(s, m) } -> std::same_as<void>;
    { t.LogError(s, m) } -> std::same_as<void>;
    { t.LogDebug(s, m) } -> std::same_as<void>;
    // 必要ならその他レベルも追加
};

// テンプレートロガー（バックエンドをコンパイル時に固定）
template<LoggerBackend Backend>
class GameLogger {
public:
    inline static Backend backend_;               // C++17 inline 変数でヘッダだけに定義
    inline static std::once_flag init_flag_;

    static Backend& GetInstance() {
        std::call_once(init_flag_, []() {
            // 既定コンストラクタで backend_ が構築される想定
        });
        return backend_;
    }

    // 初期化ラッパー
    static bool Initialize(const std::string& path = "logs/game.log") {
        return GetInstance().Initialize(path);
    }

    // テンプレート側でフォーマットを行い、バックエンドの非テンプレート LogX に渡す
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

    static void Flush() {
        GetInstance().Flush();
    }
};

// デフォルトバックエンドのエイリアス（CMake で ELK_USE_SPDLOG を制御）
#ifdef ELK_USE_SPDLOG
using DefaultBackend = SpdLogSystem;
#else
#error "バックエンドが未指定です"
#endif

using DefaultLogger = GameLogger<DefaultBackend>;

// 既存マクロの互換ラッパー
#define GAME_LOG_INFO(system, ...)  ::elk::DefaultLogger::Info(system, __VA_ARGS__)
#define GAME_LOG_WARN(system, ...)  ::elk::DefaultLogger::Warn(system, __VA_ARGS__)
#define GAME_LOG_ERROR(system, ...) ::elk::DefaultLogger::Error(system, __VA_ARGS__)
#ifdef _DEBUG
#define GAME_LOG_DEBUG(system, ...) ::elk::DefaultLogger::Debug(system, __VA_ARGS__)
#else
#define GAME_LOG_DEBUG(system, ...) ((void)0)
#endif

} // namespace elk
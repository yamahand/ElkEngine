#pragma once

#include <string>
#include <memory>
#include <sstream>

namespace elk {

    // ログレベル
    enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical, Off };

    // ログ用メタデータ（カテゴリ/サブカテゴリは文字列で自由に設定可能）
    struct LogMeta {
        LogLevel level;
        std::string category;     // 例: "Engine", "Editor", "Game"
        std::string subcategory;  // 例: "Memory", "Collision"（空文字列許容）
        const char* file;         // __FILE__
        int line;                 // __LINE__
    };

    // 初期化/終了（実装側で LogWithMeta を使って出力する）
    void InitLogger(const std::string& filename = "", LogLevel level = LogLevel::Info);
    void ShutdownLogger();

    // 低レベルアクセス（テスト/拡張用）
    // 既存の LogRaw は互換のため残す（シンプルなメッセージ出力）
    void LogRaw(LogLevel level, const std::string& message);

    // 新しい構造化ログ受け口：メタ情報とメッセージを渡す
    void LogWithMeta(const LogMeta& meta, const std::string& message);


// 新しいマクロ：カテゴリ／サブカテゴリを文字列で指定して LogWithMeta を呼ぶ。
// ファイル名・行番号はマクロ側で自動付与。
#define ELK_LOG_WITH_META(level, category_str, subcategory_str, ...) \
    do { \
        elk::LogMeta _meta{ level, std::string(category_str), std::string(subcategory_str), __FILE__, __LINE__ }; \
        elk::LogWithMeta(_meta, (std::ostringstream() << __VA_ARGS__).str()); \
    } while(0)

// レベル別の簡易カテゴリ指定マクロ
#define ELK_LOG_INFO(category, subcategory, ...)  ELK_LOG_WITH_META(elk::LogLevel::Info,    category, subcategory, __VA_ARGS__)
#define ELK_LOG_WARN(category, subcategory, ...)  ELK_LOG_WITH_META(elk::LogLevel::Warn,    category, subcategory, __VA_ARGS__)
#define ELK_LOG_ERROR(category, subcategory, ...) ELK_LOG_WITH_META(elk::LogLevel::Error,   category, subcategory, __VA_ARGS__)
#define ELK_LOG_DEBUG(category, subcategory, ...) ELK_LOG_WITH_META(elk::LogLevel::Debug,   category, subcategory, __VA_ARGS__)
#define ELK_LOG_TRACE(category, subcategory, ...) ELK_LOG_WITH_META(elk::LogLevel::Trace,   category, subcategory, __VA_ARGS__)
#define ELK_LOG_CRIT(category, subcategory, ...)  ELK_LOG_WITH_META(elk::LogLevel::Critical,category, subcategory, __VA_ARGS__)

} // namespace elk
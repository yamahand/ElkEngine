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
        : max_entries_(max_entries) {}
    
    void set_new_log_callback(std::function<void(const LogEntry&)> callback) {
        std::lock_guard<std::mutex> lock(base_sink<std::mutex>::mutex_);
        on_new_log_ = callback;
    }
    
    std::vector<LogEntry> get_recent_logs(size_t count = 100) const {
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

// ゲーム用マルチスレッドログシステム
class GameLogSystem {
private:
    std::shared_ptr<spdlog::logger> multi_logger_;
    std::shared_ptr<GameWindowSink> game_sink_;
    bool initialized_;

public:
    GameLogSystem() : initialized_(false) {}
    
    ~GameLogSystem() {
        if (initialized_) {
            spdlog::shutdown();
        }
    }
    
    bool Initialize(const std::string& log_file_path = "game.log") {
        try {
            // 非同期ログ用スレッドプール初期化
            // キューサイズ: 32768、バックグラウンドスレッド数: 1
            spdlog::init_thread_pool(32768, 1);
            
            // 複数のシンクを作成
            std::vector<spdlog::sink_ptr> sinks;
            
            // 1. ファイル出力（ローテーション機能付き）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                log_file_path, 
                1024 * 1024 * 10,  // 10MB
                5                   // 最大5ファイル
            );
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");
            sinks.push_back(file_sink);
            
#ifdef _WIN32
            // 2. Visual Studio出力ウィンドウ
            auto vs_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
            vs_sink->set_pattern("[%t] [%l] %v");
            sinks.push_back(vs_sink);
#endif
            
            // 3. インゲームログウィンドウ用カスタムシンク
            game_sink_ = std::make_shared<GameWindowSink>(1000);
            game_sink_->set_pattern("[%H:%M:%S] [%L] %v");
            sinks.push_back(game_sink_);
            
            // マルチシンクロガー作成（非同期）
            multi_logger_ = std::make_shared<spdlog::async_logger>(
                "game_multi",
                sinks.begin(),
                sinks.end(),
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
            
            // ログレベル設定
#ifdef _DEBUG
            multi_logger_->set_level(spdlog::level::debug);
#else
            multi_logger_->set_level(spdlog::level::info);
#endif
            
            // エラー時の即座フラッシュ
            multi_logger_->flush_on(spdlog::level::err);
            
            // グローバル登録
            spdlog::register_logger(multi_logger_);
            spdlog::set_default_logger(multi_logger_);
            
            initialized_ = true;
            
            // 初期化完了ログ
            LogInfo("GameLogSystem", "ログシステムが初期化されました");
            
            return true;
        }
        catch (const spdlog::spdlog_ex& ex) {
#ifdef _WIN32
            OutputDebugStringA(("ログシステム初期化エラー: " + std::string(ex.what()) + "\n").c_str());
#endif
            return false;
        }
    }
    
    // レベル別ログ出力関数
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
    
    // インゲームUI用のインターフェース
    void SetGameLogCallback(std::function<void(const GameWindowSink::LogEntry&)> callback) {
        if (game_sink_) {
            game_sink_->set_new_log_callback(callback);
        }
    }
    
    std::vector<GameWindowSink::LogEntry> GetRecentLogs(size_t count = 100) const {
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

// グローバルインスタンス（シングルトンパターン）
class GameLogger {
private:
    static std::unique_ptr<GameLogSystem> instance_;
    static std::once_flag init_flag_;
    
public:
    static GameLogSystem& GetInstance() {
        std::call_once(init_flag_, []() {
            instance_ = std::make_unique<GameLogSystem>();
        });
        return *instance_;
    }
    
    // 便利なマクロ用のラッパー関数
    template<typename... Args>
    static void Info(const std::string& system, const std::string& format, Args&&... args) {
        GetInstance().LogInfo(system, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    static void Warn(const std::string& system, const std::string& format, Args&&... args) {
        GetInstance().LogWarn(system, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    static void Error(const std::string& system, const std::string& format, Args&&... args) {
        GetInstance().LogError(system, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    static void Debug(const std::string& system, const std::string& format, Args&&... args) {
        GetInstance().LogDebug(system, format, std::forward<Args>(args)...);
    }
};

// 静的メンバーの定義（.cppファイルに記述）
std::unique_ptr<GameLogSystem> GameLogger::instance_ = nullptr;
std::once_flag GameLogger::init_flag_;

// 便利なマクロ定義
#define GAME_LOG_INFO(system, ...) GameLogger::Info(system, __VA_ARGS__)
#define GAME_LOG_WARN(system, ...) GameLogger::Warn(system, __VA_ARGS__)
#define GAME_LOG_ERROR(system, ...) GameLogger::Error(system, __VA_ARGS__)
#ifdef _DEBUG
#define GAME_LOG_DEBUG(system, ...) GameLogger::Debug(system, __VA_ARGS__)
#else
#define GAME_LOG_DEBUG(system, ...) ((void)0)
#endif

// 使用例
/*
int main() {
    // 初期化
    GameLogger::GetInstance().Initialize("logs/game.log");
    
    // 様々なスレッドからログ出力
    std::thread render_thread([]() {
        for (int i = 0; i < 100; ++i) {
            GAME_LOG_INFO("RENDERER", "フレーム {} をレンダリング中", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    });
    
    std::thread audio_thread([]() {
        for (int i = 0; i < 50; ++i) {
            GAME_LOG_DEBUG("AUDIO", "オーディオバッファを更新: {} samples", i * 1024);
            std::this_thread::sleep_for(std::chrono::milliseconds(32));
        }
    });
    
    std::thread network_thread([]() {
        GAME_LOG_INFO("NETWORK", "サーバーに接続中...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        GAME_LOG_INFO("NETWORK", "接続完了");
    });
    
    render_thread.join();
    audio_thread.join();
    network_thread.join();
    
    // 手動フラッシュ（プログラム終了前に確実に出力）
    GameLogger::GetInstance().Flush();
    
    return 0;
}
*/
#endif // ELK_USE_SPDLOG
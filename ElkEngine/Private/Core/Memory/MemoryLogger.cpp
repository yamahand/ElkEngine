#include "Core/Memory/MemoryLogger.h"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

#ifdef ELK_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace elk::memory {

MemoryLogger& MemoryLogger::GetInstance() noexcept {
    static MemoryLogger instance;
    return instance;
}

void MemoryLogger::Initialize(MemoryLogLevel level, const char* logFile) noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized.load(std::memory_order_acquire)) {
        m_logLevel = level;
        return;
    }

    m_logLevel = level;

    if (logFile && logFile[0] != '\0') {
        m_logFile.open(logFile, std::ios::out | std::ios::app);
        m_useFile = m_logFile.is_open();
        if (m_useFile) {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            m_logFile << "\n=== ElkEngine Memory Logger Started ===\n";
            m_logFile << "Time: " << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << "\n";
            m_logFile << "========================================\n\n";
            m_logFile.flush();
        }
    }

    m_initialized.store(true, std::memory_order_release);
    WriteToConsole(MemoryLogLevel::Info, "MemoryLogger", "Memory logger initialized");
}

void MemoryLogger::Shutdown() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized.load(std::memory_order_acquire)) return;

    WriteToConsole(MemoryLogLevel::Info, "MemoryLogger", "Memory logger shutting down");

    if (m_useFile && m_logFile.is_open()) {
        m_logFile << "\n=== ElkEngine Memory Logger Shutdown ===\n\n";
        m_logFile.close();
        m_useFile = false;
    }

    m_initialized.store(false, std::memory_order_release);
}

void MemoryLogger::Log(MemoryLogLevel level, std::string_view allocatorName, std::string_view message) noexcept {
    if (!ShouldLog(level)) return;

    std::lock_guard<std::mutex> lock(m_mutex);
    try {
        WriteToConsole(level, allocatorName, message);
        if (m_useFile) WriteToFile(level, allocatorName, message);
    } catch (...) {
        // ロガーは例外を投げない
    }
}

void MemoryLogger::WriteToConsole(MemoryLogLevel level, std::string_view allocatorName, std::string_view message) noexcept {
    try {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        std::time_t t = system_clock::to_time_t(now);

#ifdef ELK_PLATFORM_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // default white
        switch (level) {
            case MemoryLogLevel::Debug: color = FOREGROUND_BLUE | FOREGROUND_GREEN; break; // cyan-ish
            case MemoryLogLevel::Info:  color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break; // white
            case MemoryLogLevel::Warn:  color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break; // yellow
            case MemoryLogLevel::Error: color = FOREGROUND_RED | FOREGROUND_INTENSITY; break; // red
        }
        SetConsoleTextAttribute(hConsole, color);
#endif

        std::tm localTm;
#ifdef _MSC_VER
        localtime_s(&localTm, &t);
#else
        localTm = *std::localtime(&t);
#endif

        std::cout << "[" << std::put_time(&localTm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
                  << "[" << LevelToString(level) << "] "
                  << "[" << allocatorName << "] "
                  << message << std::endl;

#ifdef ELK_PLATFORM_WINDOWS
        // reset color
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif

    } catch (...) {
        // 最低限の出力
        std::cout << "[MEMORY] " << allocatorName << ": " << message << std::endl;
    }
}

void MemoryLogger::WriteToFile(MemoryLogLevel level, std::string_view allocatorName, std::string_view message) noexcept {
    if (!m_logFile.is_open()) return;
    try {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        std::time_t t = system_clock::to_time_t(now);

        std::tm localTm;
#ifdef _MSC_VER
        localtime_s(&localTm, &t);
#else
        localTm = *std::localtime(&t);
#endif

        m_logFile << "[" << std::put_time(&localTm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
                  << "[" << LevelToString(level) << "] "
                  << "[" << allocatorName << "] "
                  << message << std::endl;
        m_logFile.flush();
    } catch (...) {
        // 無視
    }
}

const char* MemoryLogger::LevelToString(MemoryLogLevel level) const noexcept {
    switch (level) {
        case MemoryLogLevel::Debug: return "DEBUG";
        case MemoryLogLevel::Info:  return "INFO ";
        case MemoryLogLevel::Warn:  return "WARN ";
        case MemoryLogLevel::Error: return "ERROR";
        default: return "UNK  ";
    }
}

} // namespace elk::memory

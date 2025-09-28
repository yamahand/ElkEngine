#pragma once
#include <string>
#include <mutex>
#include <utility>
#include <type_traits>
#include <functional>
#include <string_view>
#include <format>

#include "Core/Logger/LogLevel.h"

#undef ELK_USE_SPDLOG
#define ELK_USE_LOGGER

#if  defined(ELK_USE_SPDLOG)
#include "LoggerSpdlog.h"
#elif defined(ELK_USE_LOGGER)
#include "Core/Logger/Logger.h"
#else
#error "対応するログバックエンドがありません。CMake で ELK_USE_SPDLOG を指定してください。"
#endif

namespace elk {

	struct LogEntry;

	template<typename Backend>
	class LoggerService {
	public:
		// インスタンスベースのサービスとして利用するため、static メンバーを持たせない
		LoggerService() = default;
		~LoggerService() = default;

		bool Initialize(const std::string_view& path = "logs/engine.log") {
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

#if defined(ELK_USE_SPDLOG)
	using DefaultBackend = SpdLogSystem;
#elif defined(ELK_USE_LOGGER)
	using DefaultBackend = Logger;
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

// 既存コードはそのまま … （ELK_LOG_* マクロ定義まで）

// ---- ここから追加（LOG_ マクロ実装用ユーティリティ） ----
#include <vector>
#include <utility>
#include <cctype>
#include "Core/Logger/LoggerService.h"
#include "Core/Utility/ServiceLocator.h"

namespace elk::detail {

	inline std::vector<std::string_view> SplitArgNames(const char* csv) {
		std::vector<std::string_view> result;
		if (!csv || *csv == '\0') return result;

		const char* start = csv;
		const char* p = csv;
		int paren = 0;      // () ネスト（式中のカンマ抑止簡易対応）
		int angle = 0;      // <> （テンプレート型用の簡易対応、完全ではない）
		int brace = 0;      // {} （初期化子リスト簡易対応）
		while (true) {
			char c = *p;
			bool endToken = false;
			if (c == '\0') {
				endToken = true;
			}
			else {
				switch (c) {
				case '(': ++paren; break;
				case ')': if (paren > 0) --paren; break;
				case '<': ++angle; break;
				case '>': if (angle > 0) --angle; break;
				case '{': ++brace; break;
				case '}': if (brace > 0) --brace; break;
				case ',':
					if (paren == 0 && angle == 0 && brace == 0) {
						endToken = true;
					}
					break;
				default: break;
				}
			}

			if (endToken) {
				// トリム
				const char* tokEnd = p;
				// 前方スペース除去
				while (start < tokEnd && std::isspace(static_cast<unsigned char>(*start))) ++start;
				// 後方スペース除去
				while (tokEnd > start && std::isspace(static_cast<unsigned char>(*(tokEnd - 1)))) --tokEnd;
				if (tokEnd > start)
					result.emplace_back(start, static_cast<size_t>(tokEnd - start));
				if (c == '\0') break;
				start = p + 1;
			}
			++p;
		}
		return result;
	}

	template<class T>
	inline std::string ToStringOne(T&& v) {
		// formattable でない型に対してはコンパイルエラーを出したくなければ if constexpr + コンセプトを拡張
		return std::format("{}", std::forward<T>(v));
	}

	template<typename... Args>
	inline std::vector<std::pair<std::string_view, std::string>>
		BuildNameValuePairs(const char* namesCsv, Args&&... args) {
		std::vector<std::pair<std::string_view, std::string>> out;
		out.reserve(sizeof...(Args));
		auto names = SplitArgNames(namesCsv);

		size_t idx = 0;
		(
			[&] {
				std::string_view name =
					(idx < names.size()) ? names[idx] : std::string_view{};
				out.emplace_back(name, ToStringOne(args));
				++idx;
			}(),
				...
				);
		return out;
	}

	// 実際のログ出力を統合するヘルパ
	template<typename... Args>
	inline void LogWithNames(LogLevel level,
		const char* file, int line, const char* func,
		std::string_view system,
		std::format_string<Args...> fmt,
		const char* argNamesCsv,
		Args&&... args)
	{
		// フォーマット済みメッセージ
		std::string base = std::format(fmt, std::forward<Args>(args)...);

		// 変数名 + 値
		auto pairs = BuildNameValuePairs(argNamesCsv, std::forward<Args>(args)...);

		if (!pairs.empty()) {
			base += " [";
			bool first = true;
			for (auto& p : pairs) {
				if (!first) base += ", ";
				first = false;
				// 名前が無ければ (式が欠落) -> argN
				if (p.first.empty()) {
					base += std::format("arg{}={}", &p - &pairs[0], p.second);
				}
				else {
					base += std::format("{}={}", p.first, p.second);
				}
			}
			base += "]";
		}

		if (auto svc = LOGGER_SERVICE()) {
			switch (level) {
			case LogLevel::Trace:    svc->LogTrace(file, line, func, system, "{}", base); break;
			case LogLevel::Debug:    svc->LogDebug(file, line, func, system, "{}", base); break;
			case LogLevel::Info:     svc->LogInfo(file, line, func, system, "{}", base); break;
			case LogLevel::Warn:     svc->LogWarn(file, line, func, system, "{}", base); break;
			case LogLevel::Error:    svc->LogError(file, line, func, system, "{}", base); break;
			case LogLevel::Critical: svc->LogCritical(file, line, func, system, "{}", base); break;
			default:                 svc->LogInfo(file, line, func, system, "{}", base); break;
			}
		}
	}
} // namespace elk::detail

// LOG_ マクロ: 既定で Info レベル + system="User" (必要なら引数拡張)
#define LOG_(fmt, ...) \
	do { \
		::elk::detail::LogWithNames(::elk::LogLevel::Info, \
			__FILE__, __LINE__, __func__, "User", fmt, #__VA_ARGS__ __VA_OPT__(,) __VA_ARGS__); \
	} while(0)

// ---- ここまで追加 ----

// 例: 既存のテスト用関数
void func() {
	int a = 42;
	int b = 100;
	//LOG_("value: {}", a);
	LOG_("value: {1}, {2}, {3}, {0}", a, b, a + 1, "abc");
}
// 出力例:
// value: 42 [a=42]
// value: 42, 100 [a=42, b=100]


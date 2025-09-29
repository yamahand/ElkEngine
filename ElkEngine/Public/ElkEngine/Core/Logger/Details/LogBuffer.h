#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <mutex> // ヘッダ内で std::mutex / std::lock_guard を使用するため		

#include "Core/Logger/LogLevel.h"
#include "Core/Logger/TagRegistry.h"

namespace elk {
	class Logger;
}


namespace elk::logger {
	/// <summary>
	/// ログエントリの情報を保持する構造体
	/// </summary>
	struct LogMessage {
		LogLevel level;					// ログレベル
		size_t offset;					// メッセージ本文が m_buffer 内に格納されている先頭オフセット
		size_t length;					// メッセージ長（終端の null は含まない）
		TagRegistry::TagId tagId;		// タグID（タグ名は TagRegistry から逆引き可能）
		uint64_t frameNumber;			// 出力時のフレーム番号
		std::chrono::system_clock::time_point timestamp; // 出力時刻
		std::string_view message;		// メッセージ本文へのビュー
	};

	/// <summary>
	/// 固定サイズの文字バッファと、ログメタデータ配列を持つリングバッファ型のログ蓄積クラス。
	/// スレッドセーフ：すべての公開メソッドは m_mutex で保護される。
	/// </summary>
	class LogBuffer {
	public:
		LogBuffer();
		~LogBuffer();

		/// <summary>
		/// 内部バッファ容量（バイト）と保持できる最大メッセージ数を設定して初期化します。
		/// マルチスレッド環境から安全に呼び出せます。
		/// </summary>
		void Initialize(size_t capacityBytes, size_t maxMessages);

		/// <summary>
		/// ログを追加します。メッセージ本文は m_buffer のリング領域に書き込み、
		/// メタ情報は m_logMessages に保存します。
		/// バッファが一周した場合は m_swapRequested が true になります。
		/// スレッドセーフです（内部でロック）。
		/// </summary>
		void Add(LogLevel level, const std::string_view& tag, const std::string_view& message, uint64_t frameNumber);

		// スワップ要求の有無を返す（ロック済みの読み取り）
		bool NeedsSwap() {
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_swapRequested;
		}

		// 現在のメッセージ数を返す（ロック済みの読み取り）
		size_t Count() const {
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_messageCount;
		}

		/// <summary>
		/// i 番目のログメタ情報を取得します。値で返すことで、ロック解放後に他スレッドが
		/// データを書き換えても呼び出し側は一貫したスナップショットを得られます。
		/// 範囲外のインデックスの場合はデフォルト構築された LogMessage を返します。
		/// スレッドセーフです（内部でロック）。
		/// </summary>
		LogMessage At(size_t i) const {
			std::lock_guard<std::mutex> lock(m_mutex);
			if (i >= m_messageCount) {
				return LogMessage{};
			}
			return m_logMessages[i];
		}

	private:
		// メッセージ本文を格納するリングバッファ（null 終端付きで連続配置）
		std::vector<char> m_buffer;
		// 各ログのメタ情報を格納（本文は m_buffer を参照）
		std::vector<LogMessage> m_logMessages;

		size_t m_head = 0;         // 次に書き込む m_buffer 内のオフセット（バイト単位）
		size_t m_messageCount = 0;  // 現在保持しているメッセージ数（最大 m_logMessages.size()）
		bool m_swapRequested = false; // バッファが一周したことを示すフラグ
		mutable std::mutex m_mutex;   // すべての公開APIを保護するミューテックス
	};
}
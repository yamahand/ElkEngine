#include "Core/Logger/LogBuffer.h"

#include "Core/Utility/ServiceLocator.h"

namespace elk::logger {
	LogBuffer::LogBuffer()
	{
	}

	LogBuffer::~LogBuffer()
	{
	}

	void LogBuffer::Initialize(size_t capacityBytes, size_t maxMessages)
	{
		std::lock_guard lk(m_mutex);
		m_buffer.resize(capacityBytes);
		m_logMessages.resize(maxMessages);
		m_head = 0;
		m_messageCount = 0;
		m_swapRequested = false;
	}

	void LogBuffer::Add(LogLevel level, const std::string& tag, const std::string& message, uint64_t frameNumber)
	{
		std::lock_guard lk(m_mutex);
		// タグ登録
		auto tagId = ServiceLocator::Get<TagRegistry>()->GetOrRegister(tag);

		// メッセージをバッファに追加
		size_t msgLen = message.size();
		if (msgLen >= m_buffer.size()) {
			// メッセージが大きすぎる場合は切り詰める
			msgLen = m_buffer.size() - 1;
		}

		size_t writePos = m_head;
		if (writePos + msgLen + 1 > m_buffer.size()) {
			// バッファの終端を超える場合、先頭に戻る
			writePos = 0;
			m_head = 0;
			m_swapRequested = true; // バッファが一周したことを通知
		}

		std::memcpy(&m_buffer[writePos], message.data(), msgLen);
		m_buffer[writePos + msgLen] = '\0'; // null 終端

		// ログメッセージ情報を保存
		LogMessage logMsg;
		logMsg.level = level;
		logMsg.offset = writePos;
		logMsg.length = msgLen;
		logMsg.tagId = tagId;
		logMsg.frameNumber = frameNumber;
		logMsg.timestamp = std::chrono::system_clock::now();
		logMsg.message = std::string_view(&m_buffer[writePos], msgLen);
		if (m_messageCount < m_logMessages.size()) {
			m_logMessages[m_messageCount] = logMsg;
			m_messageCount++;
		}
		
		if(m_messageCount == m_logMessages.size()) {
			// メッセージ数が上限に達した場合、スワップフラグを立てる
			m_swapRequested = true;
		}

		m_head += msgLen + 1; // 次の書き込み位置を更新
		if (m_head >= m_buffer.size()) {
			m_head = 0;
			m_swapRequested = true; // バッファが一周したことを通知
		}
	}
}



#pragma once

#include <string>
#include <vector>
#include <chrono>

#include "ElkEngine/Core/Logger/Logger.h"
#include "ElkEngine/Core/Logger/TagRegistry.h"


namespace elk::logger {
	/// <summary>
	/// ログエントリの情報を保持する構造体
	/// </summary>
	struct LogMessage {
		LogLevel level;
		size_t offset; // バッファ内の位置
		size_t length; // メッセージ長
		TagRegistry::TagId tagId; // タグID
		uint64_t frameNumber; // フレーム番号
		std::chrono::system_clock::time_point timestamp;
	};

	class LogBuffer {
	public:

	};
}
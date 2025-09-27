#pragma once

namespace elk
{
    // ログレベル
    enum class LogLevel
    {
        Trace,      // 関数の入出力、ループの各イテレーション
		Debug,      // 詳細な情報、変数の中身など
		Info,       // 通常の動作情報、状態変化など
		Warn,       // 注意が必要な事象、回復可能なエラーなど
		Error,      // 処理を継続できないエラー、例外発生など
		Critical,   // システム全体に影響する重大なエラー、データ損失など
		Off
    };
}
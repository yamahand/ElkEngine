#pragma once

namespace elk::memory {

    enum class AllocatorType {
        Stack,       // スタックアロケータ（LIFO）
        Pool,        // プールアロケータ（固定サイズ）
        Heap,        // ヒープアロケータ（汎用）
        ThreadLocal, // スレッドローカルアロケータ
        Linear,      // リニアアロケータ（フレーム単位）
    };
}
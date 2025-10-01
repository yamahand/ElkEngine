#pragma once

namespace elk::memory {

    /// <summary>
    /// メモリ全体を管理するマネージャークラス
    /// 初期化時に全体で使用するメモリを確保し、各アロケータ作成時に必要なメモリを分配する
    /// </summary>
    class MemoryManager {
public:
        // 初期化パラメータ
        struct InitParams {
            // 全体のメモリサイズ
            size_t totalSize = 1024 * 1024 * 1024; // 1GB
            // アロケータに割り当てるメモリサイズの最小単位
            size_t minAllocSize = 16; // 16バイト
        };

    public:
        MemoryManager() = default;
        ~MemoryManager() = default;

        // 初期化
        bool Initialize(size_t size);
        void Shutdown();


    private:
        size_t m_totalSize = 0;
        void* m_memoryPool = nullptr;
    };

}
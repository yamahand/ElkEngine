#pragma once

#include "ElkEngine/Core/EngineAPI.h"
#include "ElkEngine/Core/Memory/IAllocator.h"
#include "ElkEngine/Core/Memory/MemoryConfig.h"

#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>
#include <array>

namespace elk::memory {

    // ========================================
    // メモリマネージャー
    // ========================================

    /**
     * @brief グローバルメモリ管理クラス
     *
     * ゲームエンジン全体のメモリを一元管理します。
     * - 大きなメモリプールを事前確保
     * - 各アロケータにメモリを分配
     * - 統計情報の収集
     * - デバッグ支援
     */
    class ENGINE_API MemoryManager {
    public:
        // シングルトンアクセス
        static MemoryManager& Instance() noexcept;

        // === 初期化・終了 ===

        /**
         * @brief メモリマネージャーを初期化
         * @param budget メモリバジェット設定
         * @return 成功時 true
         */
        [[nodiscard]] bool Initialize(const MemoryBudget& budget);

        /**
         * @brief メモリマネージャーを終了
         */
        void Shutdown();

        /**
         * @brief 初期化済みか確認
         */
        [[nodiscard]] bool IsInitialized() const noexcept;

        // === アロケータ作成 ===

        /**
         * @brief スタックアロケータを作成
         * @param zone メモリゾーン
         * @param size サイズ（0の場合は推奨サイズ）
         * @param name 名前（デバッグ用）
         */
        [[nodiscard]] std::unique_ptr<IAllocator> CreateStackAllocator(
            MemoryZone zone,
            size_t size = 0,
            const char* name = "StackAllocator"
        );

        /**
         * @brief プールアロケータを作成
         * @param zone メモリゾーン
         * @param elementSize 要素サイズ
         * @param elementCount 要素数
         * @param name 名前（デバッグ用）
         */
        [[nodiscard]] std::unique_ptr<IAllocator> CreatePoolAllocator(
            MemoryZone zone,
            size_t elementSize,
            size_t elementCount,
            const char* name = "PoolAllocator"
        );

        /**
         * @brief ヒープアロケータを作成
         * @param zone メモリゾーン
         * @param size サイズ（0の場合は推奨サイズ）
         * @param name 名前（デバッグ用）
         */
        [[nodiscard]] std::unique_ptr<IAllocator> CreateHeapAllocator(
            MemoryZone zone,
            size_t size = 0,
            const char* name = "HeapAllocator"
        );

        /**
         * @brief リニアアロケータを作成
         * @param zone メモリゾーン
         * @param size サイズ（0の場合は推奨サイズ）
         * @param name 名前（デバッグ用）
         */
        [[nodiscard]] std::unique_ptr<IAllocator> CreateLinearAllocator(
            MemoryZone zone,
            size_t size = 0,
            const char* name = "LinearAllocator"
        );

        // === 統計情報 ===

        /**
         * @brief 全体の統計情報
         */
        struct GlobalStats {
            size_t totalReserved;           // 予約済み総メモリ
            size_t totalUsed;                // 使用中の総メモリ
            size_t totalAvailable;           // 利用可能なメモリ
            size_t peakUsage;                // ピーク使用量
            size_t allocatorCount;           // アロケータ数
            size_t activeAllocationCount;    // アクティブな割り当て数

            // ゾーン別統計
            std::unordered_map<MemoryZone, size_t> zoneUsage;
            std::unordered_map<MemoryZone, size_t> zoneReserved;
        };

        /**
         * @brief グローバル統計を取得
         */
        [[nodiscard]] GlobalStats GetGlobalStats() const;

        /**
         * @brief ゾーン別の使用状況を取得
         */
        [[nodiscard]] size_t GetZoneUsage(MemoryZone zone) const;

        /**
         * @brief ゾーン別の予約済みサイズを取得
         */
        [[nodiscard]] size_t GetZoneReserved(MemoryZone zone) const;

        /**
         * @brief デバッグ用の詳細情報を文字列で取得
         */
        [[nodiscard]] std::string GetDebugReport() const;

        // === デバッグ・検証 ===

        /**
         * @brief すべてのアロケータの整合性をチェック
         */
        [[nodiscard]] bool ValidateAllAllocators() const;

        /**
         * @brief メモリリークをチェック
         */
        void CheckMemoryLeaks() const;

        /**
         * @brief アロケータの登録（デバッグ用）
         * 通常は Create* 関数内部で自動登録されます
         */
        void RegisterAllocator(IAllocator* allocator, MemoryZone zone);

        /**
         * @brief アロケータの登録解除（デバッグ用）
         */
        void UnregisterAllocator(IAllocator* allocator);

        // === 詳細制御（上級者向け） ===

        /**
         * @brief ゾーンから直接メモリを確保
         * @warning 通常は Create* 関数を使用してください
         */
        [[nodiscard]] void* AllocateFromZone(MemoryZone zone, size_t size);

        /**
         * @brief ゾーンへメモリを返却
         * @warning 通常は Create* 関数を使用してください
         */
        void DeallocateToZone(MemoryZone zone, void* ptr, size_t size);

        /**
         * @brief ゾーン間でメモリを移動（リバランス）
         */
        bool RebalanceZones();

    private:
        MemoryManager() = default;
        ~MemoryManager();

        // コピー・ムーブ禁止
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;
        MemoryManager(MemoryManager&&) = delete;
        MemoryManager& operator=(MemoryManager&&) = delete;

        // === 内部実装 ===

        struct ZoneData {
            void* baseAddress;              // ゾーンの開始アドレス
            size_t totalSize;                // ゾーンの総サイズ
            std::atomic<size_t> usedSize;   // 使用済みサイズ
            std::atomic<size_t> offset;     // 現在のオフセット（リニア割り当て用）
            bool canGrow;                    // 拡張可能か
            std::mutex mutex;                // ゾーンアクセス用ミューテックス
        };

        struct AllocatorInfo {
            IAllocator* allocator;
            MemoryZone zone;
            size_t size;
            std::string name;
            uint64_t creationTime;
        };

        // メンバ変数
        bool initialized_ = false;
        void* globalMemory_ = nullptr;          // グローバルメモリプール
        size_t globalMemorySize_ = 0;           // グローバルメモリサイズ
        MemoryBudget budget_;                    // メモリバジェット

        // ゾーン管理
        std::array<ZoneData, static_cast<size_t>(MemoryZone::Count)> zones_;

        // アロケータ管理
        mutable std::mutex allocatorsMutex_;
        std::vector<AllocatorInfo> allocators_;  // 登録されたアロケータ

        // 統計情報
        std::atomic<size_t> peakUsage_{ 0 };
        std::atomic<size_t> totalAllocatorCount_{ 0 };

        // 内部ヘルパー
        bool InitializeZones();
        void* AllocateOSMemory(size_t size);
        void FreeOSMemory(void* ptr, size_t size);
        size_t CalculateActualUsage() const;
        void UpdatePeakUsage();
    };

    // ========================================
    // グローバルヘルパー関数
    // ========================================

    /**
     * @brief メモリマネージャーの初期化（デフォルト設定）
     */
    inline bool InitializeMemoryManager() {
        return MemoryManager::Instance().Initialize(
            MemoryBudget::DefaultGameEngine()
        );
    }

    /**
     * @brief メモリマネージャーの初期化（カスタム設定）
     */
    inline bool InitializeMemoryManager(const MemoryBudget& budget) {
        return MemoryManager::Instance().Initialize(budget);
    }

    /**
     * @brief メモリマネージャーの終了
     */
    inline void ShutdownMemoryManager() {
        MemoryManager::Instance().Shutdown();
    }

    /**
     * @brief メモリ使用状況のログ出力
     */
    void LogMemoryStats();

    // ========================================
    // RAII ヘルパー
    // ========================================

    /**
     * @brief メモリマネージャーのライフタイム管理
     */
    class MemoryManagerScope {
    public:
        explicit MemoryManagerScope(const MemoryBudget& budget) {
            success_ = MemoryManager::Instance().Initialize(budget);
        }

        ~MemoryManagerScope() {
            MemoryManager::Instance().Shutdown();
        }

        [[nodiscard]] bool IsInitialized() const noexcept {
            return success_;
        }

    private:
        bool success_;
    };

    // ========================================
    // デバッグマクロ
    // ========================================

#ifdef ELK_DEBUG
#define ELK_MEMORY_CHECKPOINT() \
        elk::memory::MemoryManager::Instance().CheckMemoryLeaks()

#define ELK_MEMORY_VALIDATE() \
        do { \
            if (!elk::memory::MemoryManager::Instance().ValidateAllAllocators()) { \
                GAME_LOG_ERROR("Memory", "Memory validation failed!"); \
            } \
        } while(0)

#define ELK_MEMORY_LOG_STATS() \
        elk::memory::LogMemoryStats()
#else
#define ELK_MEMORY_CHECKPOINT() ((void)0)
#define ELK_MEMORY_VALIDATE() ((void)0)
#define ELK_MEMORY_LOG_STATS() ((void)0)
#endif

} // namespace elk::memory
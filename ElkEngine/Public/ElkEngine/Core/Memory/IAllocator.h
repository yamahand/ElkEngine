#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

#include "Core/EngineAPI.h"
#include "Core/Memory/AllocatorType.h"

namespace elk::memory {
    // アロケーション情報（デバッグ用）
    struct AllocationInfo {
        void* address;
        size_t size;
        size_t alignment;
        const char* file;
        int line;
        uint64_t timestamp;
    };

    // アロケータ統計情報
    struct AllocatorStats {
        size_t totalAllocated;      // 総割り当てサイズ
        size_t totalUsed;            // 現在使用中のサイズ
        size_t peakUsage;            // ピーク使用量
        size_t allocationCount;      // 割り当て回数
        size_t deallocationCount;    // 解放回数
        size_t activeAllocations;    // アクティブな割り当て数

        // パフォーマンス指標
        double averageAllocationSize;
        double fragmentationRatio;   // 断片化率（0.0～1.0）
    };

    // メモリアロケータ基底インターフェース
    class ENGINE_API IAllocator {
    public:
        virtual ~IAllocator() = default;

        // === 基本操作 ===

        /**
         * @brief メモリを割り当てる
         * @param size 割り当てサイズ（バイト）
         * @param alignment アライメント（デフォルト: 16バイト）
         * @return 割り当てたメモリのポインタ（失敗時は nullptr）
         */
        [[nodiscard]] virtual void* Allocate(
            size_t size,
            size_t alignment = 16
        ) = 0;

        /**
         * @brief メモリを解放する
         * @param ptr 解放するメモリのポインタ
         */
        virtual void Deallocate(void* ptr) noexcept = 0;

        /**
         * @brief メモリを再割り当てする（オプション）
         * @param ptr 元のメモリポインタ
         * @param newSize 新しいサイズ
         * @param alignment アライメント
         * @return 再割り当てしたメモリのポインタ（失敗時は nullptr）
         */
        [[nodiscard]] virtual void* Reallocate(
            void* ptr,
            size_t newSize,
            size_t alignment = 16
        ) {
            // デフォルト実装: 新規割り当て + コピー + 解放
            if (!ptr) {
                return Allocate(newSize, alignment);
            }

            if (newSize == 0) {
                Deallocate(ptr);
                return nullptr;
            }

            void* newPtr = Allocate(newSize, alignment);
            if (newPtr && ptr) {
                // 元のサイズを知る方法がないため、派生クラスでオーバーライド推奨
                Deallocate(ptr);
            }
            return newPtr;
        }

        /**
         * @brief アロケータをリセット（すべてのメモリを解放）
         */
        virtual void Reset() = 0;

        // === 情報取得 ===

        /**
         * @brief 現在使用中のメモリサイズを取得
         */
        [[nodiscard]] virtual size_t GetUsedMemory() const noexcept = 0;

        /**
         * @brief 総メモリサイズを取得
         */
        [[nodiscard]] virtual size_t GetTotalMemory() const noexcept = 0;

        /**
         * @brief 利用可能なメモリサイズを取得
         */
        [[nodiscard]] virtual size_t GetAvailableMemory() const noexcept {
            size_t total = GetTotalMemory();
            size_t used = GetUsedMemory();
            return (total > used) ? (total - used) : 0;
        }

        /**
         * @brief アロケータの種類を取得
         */
        [[nodiscard]] virtual AllocatorType GetType() const noexcept = 0;

        /**
         * @brief アロケータの名前を取得
         */
        [[nodiscard]] virtual const char* GetName() const noexcept = 0;

        /**
         * @brief 統計情報を取得
         */
        [[nodiscard]] virtual AllocatorStats GetStats() const noexcept {
            AllocatorStats stats{};
            stats.totalAllocated = GetTotalMemory();
            stats.totalUsed = GetUsedMemory();
            stats.peakUsage = GetUsedMemory();
            return stats;
        }

        // === 検証・デバッグ ===

        /**
         * @brief ポインタがこのアロケータから割り当てられたか確認
         * @param ptr 確認するポインタ
         * @return true: このアロケータから割り当てられた
         */
        [[nodiscard]] virtual bool OwnsPointer(const void* ptr) const noexcept = 0;

        /**
         * @brief アロケータの整合性をチェック
         * @return true: 整合性OK, false: 破損している
         */
        [[nodiscard]] virtual bool Validate() const noexcept {
            return true;  // デフォルトは常にOK
        }

        /**
         * @brief デバッグ情報を文字列で取得
         */
        [[nodiscard]] virtual std::string GetDebugInfo() const {
            return std::string(GetName()) +
                " [Used: " + std::to_string(GetUsedMemory()) +
                " / Total: " + std::to_string(GetTotalMemory()) + "]";
        }

        // === 機能フラグ ===

        /**
         * @brief スレッドセーフか
         */
        [[nodiscard]] virtual bool IsThreadSafe() const noexcept {
            return false;  // デフォルトは非スレッドセーフ
        }

        /**
         * @brief 個別解放をサポートするか
         */
        [[nodiscard]] virtual bool SupportsDeallocate() const noexcept {
            return true;  // ほとんどのアロケータはサポート
        }

        /**
         * @brief リアロケーションを効率的にサポートするか
         */
        [[nodiscard]] virtual bool SupportsRealloc() const noexcept {
            return false;  // 多くのアロケータは非効率
        }

    protected:
        // ヘルパー関数

        /**
         * @brief アライメント調整
         */
        static constexpr size_t AlignUp(size_t size, size_t alignment) noexcept {
            return (size + alignment - 1) & ~(alignment - 1);
        }

        /**
         * @brief アライメントされているか確認
         */
        static const bool IsAligned(const void* ptr, size_t alignment) noexcept {
            return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
        }

        /**
         * @brief 2のべき乗か確認
         */
        static constexpr bool IsPowerOfTwo(size_t value) noexcept {
            return value > 0 && (value & (value - 1)) == 0;
        }

        /**
         * @brief ポインタがメモリ範囲内か確認
         */
        static bool IsInRange(
            const void* ptr,
            const void* rangeStart,
            size_t rangeSize
        ) noexcept {
            uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
            uintptr_t start = reinterpret_cast<uintptr_t>(rangeStart);
            uintptr_t end = start + rangeSize;
            return address >= start && address < end;
        }
    };
}
#pragma once

#include "ElkEngine/Core/Memory/AllocatorType.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace elk::memory {

// ========================================
// メモリサイズの定数
// ========================================

namespace sizes {
    // 基本単位
    constexpr size_t KB = 1024;
    constexpr size_t MB = 1024 * KB;
    constexpr size_t GB = 1024 * MB;

    // 推奨される最小サイズ（用途別）
    constexpr size_t MIN_TINY_ALLOCATOR    = 64 * KB;   // 64KB  - 非常に小さいシステム用
    constexpr size_t MIN_SMALL_ALLOCATOR   = 256 * KB;  // 256KB - 小規模システム用
    constexpr size_t MIN_MEDIUM_ALLOCATOR  = 1 * MB;    // 1MB   - 中規模システム用
    constexpr size_t MIN_LARGE_ALLOCATOR   = 16 * MB;   // 16MB  - 大規模システム用
    constexpr size_t MIN_HUGE_ALLOCATOR    = 64 * MB;   // 64MB  - 巨大なシステム用

    // 推奨される最大サイズ
    constexpr size_t MAX_ALLOCATOR_SIZE    = 256 * MB;  // 256MB - 1つのアロケータの上限

    // デフォルトサイズ
    constexpr size_t DEFAULT_STACK_SIZE    = 2 * MB;
    constexpr size_t DEFAULT_POOL_SIZE     = 4 * MB;
    constexpr size_t DEFAULT_HEAP_SIZE     = 32 * MB;
    constexpr size_t DEFAULT_THREAD_SIZE   = 1 * MB;
}

// ========================================
// アロケータサイズ設定
// ========================================

struct AllocatorSizeConfig {
    size_t minSize;       // 最小サイズ
    size_t defaultSize;   // デフォルトサイズ
    size_t maxSize;       // 最大サイズ
    bool allowResize;     // 動的リサイズを許可するか
    
    // 用途別のプリセット
    static AllocatorSizeConfig Tiny() {
        return {
            sizes::MIN_TINY_ALLOCATOR,
            sizes::MIN_SMALL_ALLOCATOR,
            sizes::MIN_MEDIUM_ALLOCATOR,
            true
        };
    }
    
    static AllocatorSizeConfig Small() {
        return {
            sizes::MIN_SMALL_ALLOCATOR,
            sizes::MIN_MEDIUM_ALLOCATOR,
            sizes::MIN_LARGE_ALLOCATOR,
            true
        };
    }
    
    static AllocatorSizeConfig Medium() {
        return {
            sizes::MIN_MEDIUM_ALLOCATOR,
            sizes::DEFAULT_HEAP_SIZE,
            sizes::MIN_HUGE_ALLOCATOR,
            true
        };
    }
    
    static AllocatorSizeConfig Large() {
        return {
            sizes::MIN_LARGE_ALLOCATOR,
            sizes::MIN_HUGE_ALLOCATOR,
            sizes::MAX_ALLOCATOR_SIZE,
            true
        };
    }
};

// ========================================
// メモリゾーン設定
// ========================================

enum class MemoryZone : uint8_t {
    // 高頻度・小サイズ割り当て
    FrameTemp = 0,      // フレーム単位の一時メモリ（毎フレームリセット）
    ThreadLocal,        // スレッドローカルメモリ
    
    // 中頻度・中サイズ割り当て
    Entities,           // エンティティ・コンポーネント
    Physics,            // 物理シミュレーション
    Rendering,          // レンダリング用バッファ
    
    // 低頻度・大サイズ割り当て
    Assets,             // アセット（テクスチャ、モデル等）
    Audio,              // オーディオバッファ
    
    // 汎用
    General,            // 汎用ヒープ
    Debug,              // デバッグ専用
    
    Count
};

// ========================================
// グローバルメモリ設定
// ========================================

struct MemoryBudget {
    // 総メモリ
    size_t totalSize = 1 * sizes::GB;  // デフォルト: 1GB
    
    // ゾーン別の割合（合計100%）
    struct ZoneAllocation {
        MemoryZone zone;
        float percentage;      // 0.0 ~ 1.0
        size_t minSize;        // 最小保証サイズ
        size_t maxSize;        // 最大サイズ
        bool canGrow;          // 他のゾーンから借りられるか
    };
    
    std::vector<ZoneAllocation> zoneAllocations;
    
    // デフォルト設定（ゲームエンジン向け）
    static MemoryBudget DefaultGameEngine() {
        MemoryBudget budget;
        budget.totalSize = 1 * sizes::GB;
        
        budget.zoneAllocations = {
            // ゾーン          割合    最小      最大          拡張可
            {MemoryZone::FrameTemp,    0.05f,  4*sizes::MB,  32*sizes::MB,  true},   // 5%   (フレーム一時)
            {MemoryZone::ThreadLocal,  0.03f,  2*sizes::MB,  16*sizes::MB,  true},   // 3%   (スレッド)
            {MemoryZone::Entities,     0.20f,  32*sizes::MB, 256*sizes::MB, true},   // 20%  (ECS)
            {MemoryZone::Physics,      0.10f,  16*sizes::MB, 128*sizes::MB, true},   // 10%  (物理)
            {MemoryZone::Rendering,    0.25f,  64*sizes::MB, 384*sizes::MB, true},   // 25%  (レンダリング)
            {MemoryZone::Assets,       0.30f,  128*sizes::MB, 512*sizes::MB, false}, // 30%  (アセット)
            {MemoryZone::Audio,        0.05f,  8*sizes::MB,  64*sizes::MB,  true},   // 5%   (オーディオ)
            {MemoryZone::General,      0.10f,  16*sizes::MB, 128*sizes::MB, true},   // 10%  (汎用)
            {MemoryZone::Debug,        0.02f,  2*sizes::MB,  16*sizes::MB,  true},   // 2%   (デバッグ)
        };
        
        return budget;
    }
    
    // エディタ向け設定（より多くのメモリを使用）
    static MemoryBudget DefaultEditor() {
        MemoryBudget budget;
        budget.totalSize = 2 * sizes::GB;  // エディタは2GB
        
        budget.zoneAllocations = {
            {MemoryZone::FrameTemp,    0.03f,  8*sizes::MB,   64*sizes::MB,  true},
            {MemoryZone::ThreadLocal,  0.02f,  4*sizes::MB,   32*sizes::MB,  true},
            {MemoryZone::Entities,     0.15f,  64*sizes::MB,  384*sizes::MB, true},
            {MemoryZone::Physics,      0.05f,  16*sizes::MB,  128*sizes::MB, true},
            {MemoryZone::Rendering,    0.20f,  128*sizes::MB, 512*sizes::MB, true},
            {MemoryZone::Assets,       0.40f,  256*sizes::MB, 1*sizes::GB,   false},
            {MemoryZone::Audio,        0.03f,  8*sizes::MB,   64*sizes::MB,  true},
            {MemoryZone::General,      0.10f,  32*sizes::MB,  256*sizes::MB, true},
            {MemoryZone::Debug,        0.02f,  4*sizes::MB,   32*sizes::MB,  true},
        };
        
        return budget;
    }
    
    // モバイル向け設定（メモリ制限が厳しい）
    static MemoryBudget DefaultMobile() {
        MemoryBudget budget;
        budget.totalSize = 512 * sizes::MB;  // モバイルは512MB
        
        budget.zoneAllocations = {
            {MemoryZone::FrameTemp,    0.05f,  2*sizes::MB,   8*sizes::MB,   true},
            {MemoryZone::ThreadLocal,  0.02f,  1*sizes::MB,   4*sizes::MB,   true},
            {MemoryZone::Entities,     0.20f,  16*sizes::MB,  64*sizes::MB,  true},
            {MemoryZone::Physics,      0.10f,  8*sizes::MB,   32*sizes::MB,  true},
            {MemoryZone::Rendering,    0.25f,  32*sizes::MB,  128*sizes::MB, true},
            {MemoryZone::Assets,       0.30f,  64*sizes::MB,  192*sizes::MB, false},
            {MemoryZone::Audio,        0.05f,  4*sizes::MB,   16*sizes::MB,  true},
            {MemoryZone::General,      0.08f,  8*sizes::MB,   32*sizes::MB,  true},
            {MemoryZone::Debug,        0.00f,  0,             0,             false}, // デバッグなし
        };
        
        return budget;
    }
    
    // 実際のサイズを計算
    size_t GetZoneSize(MemoryZone zone) const {
        for (const auto& alloc : zoneAllocations) {
            if (alloc.zone == zone) {
                size_t calculatedSize = static_cast<size_t>(totalSize * alloc.percentage);
                
                // 最小/最大サイズでクランプ
                if (calculatedSize < alloc.minSize) {
                    calculatedSize = alloc.minSize;
                }
                if (calculatedSize > alloc.maxSize) {
                    calculatedSize = alloc.maxSize;
                }
                
                return calculatedSize;
            }
        }
        return 0;
    }
};

// ========================================
// システム別の推奨サイズ例
// ========================================

struct SystemMemoryRequirements {
    const char* systemName;
    size_t estimatedMin;   // 最小必要サイズ
    size_t estimatedTypical; // 標準的なサイズ
    size_t estimatedMax;   // 最大サイズ
    
    // 各システムの推奨値
    static constexpr SystemMemoryRequirements ParticleSystem() {
        return {"ParticleSystem", 256*sizes::KB, 2*sizes::MB, 16*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements AnimationSystem() {
        return {"AnimationSystem", 512*sizes::KB, 4*sizes::MB, 32*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements PhysicsSystem() {
        return {"PhysicsSystem", 2*sizes::MB, 16*sizes::MB, 128*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements RenderingSystem() {
        return {"RenderingSystem", 8*sizes::MB, 64*sizes::MB, 256*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements AudioSystem() {
        return {"AudioSystem", 1*sizes::MB, 8*sizes::MB, 32*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements ECSSystem() {
        return {"ECSSystem", 4*sizes::MB, 32*sizes::MB, 256*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements AssetLoaderSystem() {
        return {"AssetLoaderSystem", 16*sizes::MB, 128*sizes::MB, 512*sizes::MB};
    }
    
    static constexpr SystemMemoryRequirements UISystem() {
        return {"UISystem", 512*sizes::KB, 4*sizes::MB, 16*sizes::MB};
    }
};

// ========================================
// アロケータ作成時のサイズ検証
// ========================================

class MemorySizeValidator {
public:
    // サイズが妥当か検証
    static bool ValidateSize(size_t requestedSize, AllocatorType type) {
        // 最小サイズチェック
        constexpr size_t ABSOLUTE_MIN = 4 * sizes::KB;  // 絶対最小: 4KB
        if (requestedSize < ABSOLUTE_MIN) {
            return false;
        }
        
        // 型別の推奨チェック
        switch (type) {
        case AllocatorType::Stack:
            return requestedSize >= sizes::MIN_SMALL_ALLOCATOR &&
                   requestedSize <= sizes::MAX_ALLOCATOR_SIZE;
                   
        case AllocatorType::Pool:
            // プールは小さくてもOK（固定サイズオブジェクト用）
            return requestedSize >= 4 * sizes::KB &&
                   requestedSize <= sizes::MAX_ALLOCATOR_SIZE;
                   
        case AllocatorType::Heap:
            return requestedSize >= sizes::MIN_MEDIUM_ALLOCATOR &&
                   requestedSize <= sizes::MAX_ALLOCATOR_SIZE;
                   
        case AllocatorType::ThreadLocal:
            return requestedSize >= sizes::MIN_SMALL_ALLOCATOR &&
                   requestedSize <= sizes::MIN_LARGE_ALLOCATOR;
                   
        case AllocatorType::Linear:
            return requestedSize >= sizes::MIN_TINY_ALLOCATOR &&
                   requestedSize <= sizes::MAX_ALLOCATOR_SIZE;
                   
        default:
            return true;
        }
    }
    
    // サイズを推奨値に調整
    static size_t AdjustToRecommended(size_t requestedSize, AllocatorType type) {
        if (ValidateSize(requestedSize, type)) {
            return requestedSize;
        }
        
        // 型に応じたデフォルトサイズを返す
        switch (type) {
        case AllocatorType::Stack:
            return sizes::DEFAULT_STACK_SIZE;
        case AllocatorType::Pool:
            return sizes::DEFAULT_POOL_SIZE;
        case AllocatorType::Heap:
            return sizes::DEFAULT_HEAP_SIZE;
        case AllocatorType::ThreadLocal:
            return sizes::DEFAULT_THREAD_SIZE;
        case AllocatorType::Linear:
            return sizes::MIN_MEDIUM_ALLOCATOR;
        default:
            return sizes::MIN_MEDIUM_ALLOCATOR;
        }
    }
};

// ========================================
// 使用例用のヘルパー
// ========================================

inline void PrintMemoryBudget(const MemoryBudget& budget) {
    std::cout << "Total Memory Budget: " << budget.totalSize / sizes::MB << " MB\n";
    std::cout << "Zone Allocations:\n";
    
    for (const auto& alloc : budget.zoneAllocations) {
        size_t zoneSize = budget.GetZoneSize(alloc.zone);
        std::cout << "  Zone " << static_cast<int>(alloc.zone) 
                  << ": " << zoneSize / sizes::MB << " MB "
                  << "(" << (alloc.percentage * 100) << "%)\n";
    }
}

} // namespace elk::memory
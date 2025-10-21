#include "Core/Memory/MemoryManager.h"
#include "Core/Logger/LoggerService.h"


#include <sstream>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace elk::memory {

// ========================================
// シングルトン
// ========================================

MemoryManager& MemoryManager::Instance() noexcept {
    static MemoryManager instance;
    return instance;
}

// ========================================
// 初期化・終了
// ========================================

bool MemoryManager::Initialize(const MemoryBudget& budget) {
    if (initialized_) {
        ELK_LOG_WARN("Memory", "MemoryManager already initialized");
        return true;
    }

    ELK_LOG_INFO("Memory", "Initializing MemoryManager with {} MB", 
                  budget.totalSize / sizes::MB);

    budget_ = budget;
    globalMemorySize_ = budget.totalSize;

    // OSからメモリを確保
    globalMemory_ = AllocateOSMemory(globalMemorySize_);
    if (!globalMemory_) {
        ELK_LOG_ERROR("Memory", "Failed to allocate {} MB from OS", 
                      globalMemorySize_ / sizes::MB);
        return false;
    }

    // メモリをゼロクリア（デバッグビルドのみ）
#ifdef ELK_DEBUG
    std::memset(globalMemory_, 0, globalMemorySize_);
#endif

    // ゾーンを初期化
    if (!InitializeZones()) {
        ELK_LOG_ERROR("Memory", "Failed to initialize memory zones");
        FreeOSMemory(globalMemory_, globalMemorySize_);
        globalMemory_ = nullptr;
        return false;
    }

    initialized_ = true;
    ELK_LOG_INFO("Memory", "MemoryManager initialized successfully");
    
    return true;
}

void MemoryManager::Shutdown() {
    if (!initialized_) {
        return;
    }

    ELK_LOG_INFO("Memory", "Shutting down MemoryManager");

    // 統計情報を出力
#ifdef ELK_DEBUG
    auto stats = GetGlobalStats();
    ELK_LOG_INFO("Memory", "Final stats - Used: {} MB / Reserved: {} MB", 
                  stats.totalUsed / sizes::MB,
                  stats.totalReserved / sizes::MB);
    
    // メモリリークチェック
    CheckMemoryLeaks();
#endif

    // アロケータリストをクリア
    {
        std::lock_guard lock(allocatorsMutex_);
        if (!allocators_.empty()) {
            ELK_LOG_WARN("Memory", "{} allocators still registered", 
                         allocators_.size());
        }
        allocators_.clear();
    }

    // OSにメモリを返却
    if (globalMemory_) {
        FreeOSMemory(globalMemory_, globalMemorySize_);
        globalMemory_ = nullptr;
    }

    initialized_ = false;
    ELK_LOG_INFO("Memory", "MemoryManager shutdown complete");
}

bool MemoryManager::IsInitialized() const noexcept {
    return initialized_;
}

// ========================================
// ゾーン初期化
// ========================================

bool MemoryManager::InitializeZones() {
    uint8_t* currentAddress = static_cast<uint8_t*>(globalMemory_);

    for (const auto& allocation : budget_.zoneAllocations) {
        size_t zoneSize = budget_.GetZoneSize(allocation.zone);
        size_t zoneIndex = static_cast<size_t>(allocation.zone);

        auto& zone = zones_[zoneIndex];
        zone.baseAddress = currentAddress;
        zone.totalSize = zoneSize;
        zone.usedSize.store(0, std::memory_order_release);
        zone.offset.store(0, std::memory_order_release);
        zone.canGrow = allocation.canGrow;

        ELK_LOG_DEBUG("Memory", "Zone {} initialized: {} MB at 0x{:X}",
                      zoneIndex,
                      zoneSize / sizes::MB,
                      reinterpret_cast<uintptr_t>(currentAddress));

        currentAddress += zoneSize;
    }

    return true;
}

// ========================================
// アロケータ作成
// ========================================

std::unique_ptr<IAllocator> MemoryManager::CreateStackAllocator(
    MemoryZone zone,
    size_t size,
    const char* name) {
    
    if (!initialized_) {
        ELK_LOG_ERROR("Memory", "MemoryManager not initialized");
        return nullptr;
    }

    // サイズが0なら推奨値を使用
    if (size == 0) {
        size = sizes::DEFAULT_STACK_SIZE;
    }

    // サイズ検証
    size = MemorySizeValidator::AdjustToRecommended(size, AllocatorType::Stack);

    // ゾーンからメモリを確保
    void* memory = AllocateFromZone(zone, size);
    if (!memory) {
        ELK_LOG_ERROR("Memory", "Failed to allocate {} KB for StackAllocator from zone {}",
                      size / sizes::KB, static_cast<int>(zone));
        return nullptr;
    }

    // アロケータを作成（実装は後で作成します）
    // 今は nullptr を返す
    ELK_LOG_INFO("Memory", "Created StackAllocator '{}': {} KB in zone {}",
                  name, size / sizes::KB, static_cast<int>(zone));
    
    return nullptr;  // TODO: 実際のアロケータ実装
}

std::unique_ptr<IAllocator> MemoryManager::CreatePoolAllocator(
    MemoryZone zone,
    size_t elementSize,
    size_t elementCount,
    const char* name) {
    
    if (!initialized_) {
        ELK_LOG_ERROR("Memory", "MemoryManager not initialized");
        return nullptr;
    }

    size_t totalSize = elementSize * elementCount;
    
    // サイズ検証
    totalSize = MemorySizeValidator::AdjustToRecommended(totalSize, AllocatorType::Pool);

    void* memory = AllocateFromZone(zone, totalSize);
    if (!memory) {
        ELK_LOG_ERROR("Memory", "Failed to allocate {} KB for PoolAllocator from zone {}",
                      totalSize / sizes::KB, static_cast<int>(zone));
        return nullptr;
    }

    ELK_LOG_INFO("Memory", "Created PoolAllocator '{}': {} elements x {} bytes in zone {}",
                  name, elementCount, elementSize, static_cast<int>(zone));
    
    return nullptr;  // TODO: 実際のアロケータ実装
}

std::unique_ptr<IAllocator> MemoryManager::CreateHeapAllocator(
    MemoryZone zone,
    size_t size,
    const char* name) {
    
    if (!initialized_) {
        ELK_LOG_ERROR("Memory", "MemoryManager not initialized");
        return nullptr;
    }

    if (size == 0) {
        size = sizes::DEFAULT_HEAP_SIZE;
    }

    size = MemorySizeValidator::AdjustToRecommended(size, AllocatorType::Heap);

    void* memory = AllocateFromZone(zone, size);
    if (!memory) {
        ELK_LOG_ERROR("Memory", "Failed to allocate {} MB for HeapAllocator from zone {}",
                      size / sizes::MB, static_cast<int>(zone));
        return nullptr;
    }

    ELK_LOG_INFO("Memory", "Created HeapAllocator '{}': {} MB in zone {}",
                  name, size / sizes::MB, static_cast<int>(zone));
    
    return nullptr;  // TODO: 実際のアロケータ実装
}

std::unique_ptr<IAllocator> MemoryManager::CreateLinearAllocator(
    MemoryZone zone,
    size_t size,
    const char* name) {
    
    if (!initialized_) {
        ELK_LOG_ERROR("Memory", "MemoryManager not initialized");
        return nullptr;
    }

    if (size == 0) {
        size = sizes::MIN_MEDIUM_ALLOCATOR;
    }

    size = MemorySizeValidator::AdjustToRecommended(size, AllocatorType::Linear);

    void* memory = AllocateFromZone(zone, size);
    if (!memory) {
        ELK_LOG_ERROR("Memory", "Failed to allocate {} KB for LinearAllocator from zone {}",
                      size / sizes::KB, static_cast<int>(zone));
        return nullptr;
    }

    ELK_LOG_INFO("Memory", "Created LinearAllocator '{}': {} KB in zone {}",
                  name, size / sizes::KB, static_cast<int>(zone));
    
    return nullptr;  // TODO: 実際のアロケータ実装
}

// ========================================
// ゾーンメモリ管理
// ========================================

void* MemoryManager::AllocateFromZone(MemoryZone zone, size_t size) {
    size_t zoneIndex = static_cast<size_t>(zone);
    auto& zoneData = zones_[zoneIndex];

    std::lock_guard lock(zoneData.mutex);

    // 現在のオフセットを取得
    size_t currentOffset = zoneData.offset.load(std::memory_order_acquire);
    size_t newOffset = currentOffset + size;

    // 容量チェック
    if (newOffset > zoneData.totalSize) {
        ELK_LOG_WARN("Memory", "Zone {} out of memory: requested {} KB, available {} KB",
                     zoneIndex,
                     size / sizes::KB,
                     (zoneData.totalSize - currentOffset) / sizes::KB);
        return nullptr;
    }

    // オフセットを更新
    zoneData.offset.store(newOffset, std::memory_order_release);
    zoneData.usedSize.fetch_add(size, std::memory_order_release);

    void* ptr = static_cast<uint8_t*>(zoneData.baseAddress) + currentOffset;

    ELK_LOG_DEBUG("Memory", "Allocated {} KB from zone {} at offset {} KB",
                  size / sizes::KB, zoneIndex, currentOffset / sizes::KB);

    // ピーク使用量を更新
    UpdatePeakUsage();

    return ptr;
}

void MemoryManager::DeallocateToZone(MemoryZone zone, void* ptr, size_t size) {
    size_t zoneIndex = static_cast<size_t>(zone);
    auto& zoneData = zones_[zoneIndex];

    std::lock_guard lock(zoneData.mutex);

    // 使用量を減らす
    size_t currentUsed = zoneData.usedSize.load(std::memory_order_acquire);
    if (currentUsed >= size) {
        zoneData.usedSize.fetch_sub(size, std::memory_order_release);
    }

    ELK_LOG_DEBUG("Memory", "Deallocated {} KB to zone {}",
                  size / sizes::KB, zoneIndex);
}

// ========================================
// 統計情報
// ========================================

MemoryManager::GlobalStats MemoryManager::GetGlobalStats() const {
    GlobalStats stats{};

    stats.totalReserved = globalMemorySize_;
    stats.totalUsed = CalculateActualUsage();
    stats.totalAvailable = stats.totalReserved - stats.totalUsed;
    stats.peakUsage = peakUsage_.load(std::memory_order_acquire);

    {
        std::lock_guard lock(allocatorsMutex_);
        stats.allocatorCount = allocators_.size();
        
        size_t totalActive = 0;
        for (const auto& info : allocators_) {
            if (info.allocator) {
                totalActive += info.allocator->GetStats().activeAllocations;
            }
        }
        stats.activeAllocationCount = totalActive;
    }

    // ゾーン別統計
    for (size_t i = 0; i < zones_.size(); ++i) {
        auto zone = static_cast<MemoryZone>(i);
        stats.zoneUsage[zone] = zones_[i].usedSize.load(std::memory_order_acquire);
        stats.zoneReserved[zone] = zones_[i].totalSize;
    }

    return stats;
}

size_t MemoryManager::GetZoneUsage(MemoryZone zone) const {
    size_t zoneIndex = static_cast<size_t>(zone);
    return zones_[zoneIndex].usedSize.load(std::memory_order_acquire);
}

size_t MemoryManager::GetZoneReserved(MemoryZone zone) const {
    size_t zoneIndex = static_cast<size_t>(zone);
    return zones_[zoneIndex].totalSize;
}

std::string MemoryManager::GetDebugReport() const {
    std::ostringstream oss;
    
    oss << "=== Memory Manager Debug Report ===\n\n";
    
    auto stats = GetGlobalStats();
    
    oss << "Global Statistics:\n";
    oss << "  Total Reserved: " << stats.totalReserved / sizes::MB << " MB\n";
    oss << "  Total Used:     " << stats.totalUsed / sizes::MB << " MB\n";
    oss << "  Total Available:" << stats.totalAvailable / sizes::MB << " MB\n";
    oss << "  Peak Usage:     " << stats.peakUsage / sizes::MB << " MB\n";
    oss << "  Allocators:     " << stats.allocatorCount << "\n";
    oss << "  Active Allocs:  " << stats.activeAllocationCount << "\n\n";
    
    oss << "Zone Statistics:\n";
    const char* zoneNames[] = {
        "FrameTemp", "ThreadLocal", "Entities", "Physics",
        "Rendering", "Assets", "Audio", "General", "Debug"
    };
    
    for (size_t i = 0; i < static_cast<size_t>(MemoryZone::Count); ++i) {
        auto zone = static_cast<MemoryZone>(i);
        size_t used = GetZoneUsage(zone);
        size_t reserved = GetZoneReserved(zone);
        float percentage = reserved > 0 ? (100.0f * used / reserved) : 0.0f;
        
        oss << "  " << std::setw(12) << std::left << zoneNames[i] << ": "
            << std::setw(6) << std::right << used / sizes::MB << " / "
            << std::setw(6) << reserved / sizes::MB << " MB "
            << "(" << std::fixed << std::setprecision(1) << percentage << "%)\n";
    }
    
    return oss.str();
}

// ========================================
// デバッグ・検証
// ========================================

void MemoryManager::RegisterAllocator(IAllocator* allocator, MemoryZone zone) {
    if (!allocator) return;

    std::lock_guard lock(allocatorsMutex_);
    
    AllocatorInfo info{};
    info.allocator = allocator;
    info.zone = zone;
    info.size = allocator->GetTotalMemory();
    info.name = allocator->GetName();
    info.creationTime = std::chrono::steady_clock::now().time_since_epoch().count();
    
    allocators_.push_back(info);
    totalAllocatorCount_.fetch_add(1, std::memory_order_release);
    
    ELK_LOG_DEBUG("Memory", "Registered allocator '{}' (zone {})",
                  info.name, static_cast<int>(zone));
}

void MemoryManager::UnregisterAllocator(IAllocator* allocator) {
    if (!allocator) return;

    std::lock_guard lock(allocatorsMutex_);
    
    auto it = std::find_if(allocators_.begin(), allocators_.end(),
        [allocator](const AllocatorInfo& info) {
            return info.allocator == allocator;
        });
    
    if (it != allocators_.end()) {
        ELK_LOG_DEBUG("Memory", "Unregistered allocator '{}'", it->name);
        allocators_.erase(it);
    }
}

bool MemoryManager::ValidateAllAllocators() const {
    std::lock_guard lock(allocatorsMutex_);
    
    bool allValid = true;
    for (const auto& info : allocators_) {
        if (info.allocator && !info.allocator->Validate()) {
            ELK_LOG_ERROR("Memory", "Allocator '{}' validation failed", info.name);
            allValid = false;
        }
    }
    
    return allValid;
}

void MemoryManager::CheckMemoryLeaks() const {
    std::lock_guard lock(allocatorsMutex_);
    
    size_t totalLeaks = 0;
    
    for (const auto& info : allocators_) {
        if (info.allocator) {
            auto stats = info.allocator->GetStats();
            if (stats.activeAllocations > 0) {
                ELK_LOG_WARN("Memory", "Allocator '{}' has {} active allocations (potential leak)",
                             info.name, stats.activeAllocations);
                totalLeaks += stats.activeAllocations;
            }
        }
    }
    
    if (totalLeaks > 0) {
        ELK_LOG_WARN("Memory", "Total potential leaks: {} allocations", totalLeaks);
    } else {
        ELK_LOG_INFO("Memory", "No memory leaks detected");
    }
}

bool MemoryManager::RebalanceZones() {
    ELK_LOG_INFO("Memory", "Rebalancing zones...");
    
    // 各ゾーンの使用率を計算
    std::vector<std::pair<MemoryZone, float>> zoneUsageRatios;
    
    for (size_t i = 0; i < zones_.size(); ++i) {
        auto zone = static_cast<MemoryZone>(i);
        size_t used = GetZoneUsage(zone);
        size_t reserved = GetZoneReserved(zone);
        
        if (reserved > 0) {
            float ratio = static_cast<float>(used) / reserved;
            zoneUsageRatios.emplace_back(zone, ratio);
        }
    }
    
    // 使用率でソート
    std::sort(zoneUsageRatios.begin(), zoneUsageRatios.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // 高使用率ゾーンと低使用率ゾーンを表示
    ELK_LOG_DEBUG("Memory", "Zone usage after rebalance:");
    for (const auto& [zone, ratio] : zoneUsageRatios) {
        ELK_LOG_DEBUG("Memory", "  Zone {}: {:.1f}%",
                      static_cast<int>(zone), ratio * 100.0f);
    }
    
    // 実際のリバランス処理は複雑なので、今回は統計表示のみ
    return true;
}

// ========================================
// OS メモリ管理
// ========================================

void* MemoryManager::AllocateOSMemory(size_t size) {
#ifdef _WIN32
    // Windows: VirtualAlloc
    void* ptr = VirtualAlloc(
        nullptr,
        size,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    
    if (!ptr) {
        ELK_LOG_ERROR("Memory", "VirtualAlloc failed with error {}", GetLastError());
    }
    
    return ptr;
    
#elif defined(__linux__) || defined(__APPLE__)
    // Linux/macOS: mmap
    void* ptr = mmap(
        nullptr,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    
    if (ptr == MAP_FAILED) {
        ELK_LOG_ERROR("Memory", "mmap failed");
        return nullptr;
    }
    
    return ptr;
    
#else
    #error "Unsupported platform"
#endif
}

void MemoryManager::FreeOSMemory(void* ptr, size_t size) {
    if (!ptr) return;
    
#ifdef _WIN32
    // Windows: VirtualFree
    if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
        ELK_LOG_ERROR("Memory", "VirtualFree failed with error {}", GetLastError());
    }
    
#elif defined(__linux__) || defined(__APPLE__)
    // Linux/macOS: munmap
    if (munmap(ptr, size) != 0) {
        ELK_LOG_ERROR("Memory", "munmap failed");
    }
    
#else
    #error "Unsupported platform"
#endif
}

// ========================================
// 内部ヘルパー
// ========================================

size_t MemoryManager::CalculateActualUsage() const {
    size_t total = 0;
    
    for (const auto& zone : zones_) {
        total += zone.usedSize.load(std::memory_order_acquire);
    }
    
    return total;
}

void MemoryManager::UpdatePeakUsage() {
    size_t currentUsage = CalculateActualUsage();
    size_t currentPeak = peakUsage_.load(std::memory_order_acquire);
    
    while (currentUsage > currentPeak) {
        if (peakUsage_.compare_exchange_weak(
            currentPeak, currentUsage,
            std::memory_order_release,
            std::memory_order_acquire)) {
            break;
        }
    }
}

MemoryManager::~MemoryManager() {
    if (initialized_) {
        Shutdown();
    }
}

// ========================================
// グローバルヘルパー関数
// ========================================

void LogMemoryStats() {
    auto& manager = MemoryManager::Instance();
    
    if (!manager.IsInitialized()) {
        ELK_LOG_WARN("Memory", "MemoryManager not initialized");
        return;
    }
    
    std::string report = manager.GetDebugReport();
    ELK_LOG_INFO("Memory", "\n{}", report);
}

} // namespace elk::memory


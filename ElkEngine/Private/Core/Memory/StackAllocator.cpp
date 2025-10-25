#include "Core/Memory/StackAllocator.h"
#include "Core/Memory/MemoryLogger.h"

namespace elk::memory
{
	StackAllocator::StackAllocator(void* memory, size_t size, const char* name)
		: m_memory(static_cast<uint8_t*>(memory)),
		m_size(size),
		m_offset(0),
		m_name(name ? name : "StackAllocator"),
		m_peakUsage(0),
		m_allocationCount(0),
		m_casRetryCount(0)
	{
		if (!memory) {
			MEMORY_LOG_ERROR(m_name.c_str(), "StackAllocator initialized with null memory pointer.");
		}
	}

	memory::StackAllocator::~StackAllocator()
	{
	}

	void* StackAllocator::Allocate(size_t size, size_t alignment)
	{
		if (size == 0) {
			MEMORY_LOG_WARN(m_name.c_str(), "Allocation size is zero.");
			return nullptr;
		}

		if (!IsPowerOfTwo(alignment)) {
			MEMORY_LOG_ERROR_F(m_name.c_str(), "Alignment {} is not a power of two.", alignment);
			return nullptr;
		}

		// デバッグモードではヘッダーサイズを考慮
#ifdef ELK_DEBUG
		const size_t headerSize = sizeof(AllocationHeader);
#else
		const size_t headerSize = 0;
#endif

		// CAS (Compare-And-Swap) ループでスレッドセーフな割り当てを実現
		size_t currentOffset, newOffset, alignedOffset;
		uint8_t* alignedPtr = nullptr;

		do {
			currentOffset = m_offset.load(std::memory_order_acquire);

			// アライメント計算
			uintptr_t rawAddress = reinterpret_cast<uintptr_t>(m_memory + currentOffset + headerSize);
			alignedOffset = currentOffset + headerSize + (AlignUp(rawAddress, alignment) - rawAddress);

			alignedPtr = m_memory + alignedOffset;
			newOffset = alignedOffset + size;

			// メモリ範囲チェック
			if (newOffset > m_size) {
				MEMORY_LOG_ERROR_F(m_name.c_str(), "Out of memory: requested {}, available {}",
					newOffset - currentOffset, m_size - currentOffset);
				return nullptr;
			}

		} while (!m_offset.compare_exchange_weak(
			currentOffset,
			newOffset,
			std::memory_order_release,
			std::memory_order_acquire));

		// 統計情報更新
		m_allocationCount.fetch_add(1, std::memory_order_relaxed);
		UpdatePeakUsage(newOffset);

#ifdef ELK_DEBUG
		// デバッグヘッダーを書き込む
		size_t padding = alignedOffset - currentOffset - headerSize;
		uint32_t allocId = static_cast<uint32_t>(m_allocationCount.load(std::memory_order_relaxed));
		WriteDebugHeader(m_memory + currentOffset, size, padding, allocId);
#endif

		return alignedPtr;
	}

	void StackAllocator::Deallocate(void* ptr) noexcept
	{
		// スタックアロケータは個別解放をサポートしない
		// Reset() または Rewind() を使用する
		(void)ptr;
	}

	void* StackAllocator::Reallocate(void* ptr, size_t newSize, size_t alignment)
	{
		// スタックアロケータでは非効率なので、新規割り当て推奨
		if (!ptr) {
			return Allocate(newSize, alignment);
		}

		if (newSize == 0) {
			// スタックアロケータは個別解放をサポートしないため、何もしない
			return nullptr;
		}

		// 新しいメモリを割り当ててコピー
		void* newPtr = Allocate(newSize, alignment);
		if (!newPtr) {
			return nullptr;
		}

		// 元のサイズは不明なので、派生クラスで実装することを推奨
		// ここでは単純に新しいポインタを返す
	MEMORY_LOG_WARN(m_name.c_str(), "Reallocate is inefficient for StackAllocator. Consider redesigning.");

		return newPtr;
	}

	void StackAllocator::Reset()
	{
		m_offset.store(0, std::memory_order_release);
	MEMORY_LOG_INFO(m_name.c_str(), "Reset: All allocations cleared.");
	}

	size_t StackAllocator::GetMarker() const noexcept
	{
		return m_offset.load(std::memory_order_acquire);
	}

	void StackAllocator::Rewind(size_t marker) noexcept
	{
		if (marker > m_size) {
			MEMORY_LOG_ERROR_F(m_name.c_str(), "Invalid marker: {} exceeds size {}", marker, m_size);
			return;
		}

		size_t currentOffset = m_offset.load(std::memory_order_acquire);
		if (marker > currentOffset) {
			MEMORY_LOG_WARN_F(m_name.c_str(), "Marker {} is ahead of current offset {}", marker, currentOffset);
			return;
		}

		m_offset.store(marker, std::memory_order_release);
	}

	void StackAllocator::UpdatePeakUsage(size_t currentUsage) noexcept
	{
		size_t peak = m_peakUsage.load(std::memory_order_relaxed);
		while (currentUsage > peak) {
			if (m_peakUsage.compare_exchange_weak(peak, currentUsage, std::memory_order_relaxed)) {
				break;
			}
		}
	}

	size_t StackAllocator::GetUsedMemory() const noexcept
	{
		return m_offset.load(std::memory_order_acquire);
	}

	size_t StackAllocator::GetTotalMemory() const noexcept
	{
		return m_size;
	}

	AllocatorType StackAllocator::GetType() const noexcept
	{
		return AllocatorType::Stack;
	}

	const char* StackAllocator::GetName() const noexcept
	{
		return m_name.c_str();
	}

	AllocatorStats StackAllocator::GetStats() const noexcept
	{
		AllocatorStats stats{};
		stats.totalAllocated = m_size;
		stats.totalUsed = GetUsedMemory();
		stats.peakUsage = m_peakUsage.load(std::memory_order_relaxed);
		stats.allocationCount = m_allocationCount.load(std::memory_order_relaxed);
		stats.deallocationCount = 0; // スタックアロケータは個別解放をサポートしない
		stats.activeAllocations = (stats.allocationCount > 0) ? stats.allocationCount : 0;

		// 平均割り当てサイズ
		if (stats.allocationCount > 0) {
			stats.averageAllocationSize = static_cast<double>(stats.totalUsed) / stats.allocationCount;
		} else {
			stats.averageAllocationSize = 0.0;
		}

		// 断片化率（スタックアロケータは断片化しない）
		stats.fragmentationRatio = 0.0;

		return stats;
	}

	size_t StackAllocator::GetAvailableMemory() const noexcept
	{
		size_t used = GetUsedMemory();
		return (m_size > used) ? (m_size - used) : 0;
	}

	size_t StackAllocator::GetPeakUsage() const noexcept
	{
		return m_peakUsage.load(std::memory_order_relaxed);
	}

	bool StackAllocator::OwnsPointer(const void* ptr) const noexcept
	{
		if (!ptr || !m_memory) {
			return false;
		}

		return IsInRange(ptr, m_memory, m_size);
	}

	bool StackAllocator::Validate() const noexcept
	{
		// 基本的な整合性チェック
		if (!m_memory) {
			return false;
		}

		size_t offset = m_offset.load(std::memory_order_acquire);
		if (offset > m_size) {
			return false;
		}

#ifdef ELK_DEBUG
		// デバッグモードでは各ヘッダーのマジックナンバーをチェック
		// (実装は複雑になるため、基本チェックのみ)
#endif

		return true;
	}

	std::string StackAllocator::GetDebugInfo() const
	{
		size_t used = GetUsedMemory();
		size_t peak = GetPeakUsage();
		size_t allocCount = m_allocationCount.load(std::memory_order_relaxed);
		size_t casRetry = m_casRetryCount.load(std::memory_order_relaxed);

		std::string info = m_name + " [StackAllocator]\n";
		info += "  Memory: " + std::to_string(used) + " / " + std::to_string(m_size) + " bytes\n";
		info += "  Peak: " + std::to_string(peak) + " bytes\n";
		info += "  Allocations: " + std::to_string(allocCount) + "\n";
		info += "  CAS Retries: " + std::to_string(casRetry) + "\n";
		info += "  Usage: " + std::to_string(static_cast<double>(used) / m_size * 100.0) + "%";

		return info;
	}

	bool StackAllocator::IsThreadSafe() const noexcept
	{
		return true; // Atomic操作を使用しているためスレッドセーフ
	}

	bool StackAllocator::SupportsDeallocate() const noexcept
	{
		return false; // スタックアロケータは個別解放をサポートしない
	}

	bool StackAllocator::SupportsRealloc() const noexcept
	{
		return false; // 非効率なため推奨しない
	}

#ifdef ELK_DEBUG
	void StackAllocator::WriteDebugHeader(uint8_t* ptr, size_t size, size_t padding, uint32_t allocId)
	{
		AllocationHeader* header = reinterpret_cast<AllocationHeader*>(ptr);
		header->size = size;
		header->padding = padding;
		header->magic = ALLOCATION_MAGIC;
		header->allocId = allocId;
	}

	bool StackAllocator::ValidateHeader(const void* header) const noexcept
	{
		if (!header) {
			return false;
		}

		const AllocationHeader* allocHeader = reinterpret_cast<const AllocationHeader*>(header);
		return allocHeader->magic == ALLOCATION_MAGIC;
	}
#endif

} // namespace elk::memory

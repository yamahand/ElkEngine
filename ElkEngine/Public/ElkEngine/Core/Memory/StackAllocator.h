#pragma once

#include "Core/EngineAPI.h"
#include "IAllocator.h"

#include <atomic>
#include <string>
#include <cstdint>

namespace elk::memory {
	class ENGINE_API StackAllocator : public IAllocator {
	public:
		StackAllocator(void* memory, size_t size, const char* name = "StackAllocator");
		~StackAllocator() override;

		// === 基本操作 ===

		[[nodiscard]] void* Allocate(size_t size, size_t alignment = 16) override;
		void Deallocate(void* ptr) noexcept override;
		[[nodiscard]] void* Reallocate(void* ptr, size_t newSize, size_t alignment = 16) override;
		void Reset() override;

		// === 情報取得 ===

		[[nodiscard]] size_t GetUsedMemory() const noexcept override;
		[[nodiscard]] size_t GetTotalMemory() const noexcept override;
		[[nodiscard]] AllocatorType GetType() const noexcept override;
		[[nodiscard]] const char* GetName() const noexcept override;
		[[nodiscard]] AllocatorStats GetStats() const noexcept override;

		// === 検証・デバッグ ===

		[[nodiscard]] bool OwnsPointer(const void* ptr) const noexcept override;
		[[nodiscard]] bool Validate() const noexcept override;
		[[nodiscard]] std::string GetDebugInfo() const override;

		[[nodiscard]] bool IsThreadSafe() const noexcept override;
		[[nodiscard]] bool SupportsDeallocate() const noexcept override;
		[[nodiscard]] bool SupportsRealloc() const noexcept override;

		[[nodiscard]] size_t GetMarker() const noexcept;
		void Rewind(size_t marker) noexcept;
		[[nodiscard]] size_t GetAvailableMemory() const noexcept override;
		[[nodiscard]] size_t GetPeakUsage() const noexcept;

	private:
		uint8_t* m_memory;
		size_t m_size;
		std::atomic<size_t> m_offset;
		std::string m_name;

		// 統計情報
		std::atomic<size_t> m_peakUsage;
		std::atomic<size_t> m_allocationCount;
		mutable std::atomic<size_t> m_casRetryCount; // Compare-And-Swap 再試行回数

#ifdef ELK_DEBUG
		struct AllocationHeader {
			size_t size;    // 割り当てサイズ
			size_t padding; // パディング
			uint32_t magic;  // マジックナンバー（検証用）
			uint32_t allocId; // 割り当てID（デバッグ用）
		};
		static constexpr uint32_t ALLOCATION_MAGIC = 0xDEADBEEF;
#endif

		void UpdatePeakUsage(size_t currentUsage) noexcept;

#ifdef ELK_DEBUG
		void WriteDebugHeader(uint8_t* ptr, size_t size, size_t padding, uint32_t allocId);
		bool ValidateHeader(const void* header) const noexcept;
#endif
	};

	class StackAllocatorScope {
	public:
		explicit StackAllocatorScope(StackAllocator& allocator)
			: m_allocator(allocator), m_marker(allocator.GetMarker()) {
		}

		~StackAllocatorScope() {
			m_allocator.Rewind(m_marker);
		}

		// コピー禁止/ムーブ禁止
		StackAllocatorScope(const StackAllocatorScope&) = delete;
		StackAllocatorScope& operator=(const StackAllocatorScope&) = delete;
		StackAllocatorScope(StackAllocatorScope&&) = delete;
		StackAllocatorScope& operator=(StackAllocatorScope&&) = delete;

	private:
		StackAllocator& m_allocator;
		size_t m_marker;
	};

	class ENGINE_API StackAllocatorScopePtr {
	public:
		explicit StackAllocatorScopePtr(const std::unique_ptr<StackAllocator>& allocator)
			: m_allocator(allocator.get()), m_marker(allocator ? allocator->GetMarker() : 0) {
			if (!allocator) {
				// ログ出す
			}
		}

		explicit StackAllocatorScopePtr(const std::shared_ptr<StackAllocator>& allocator)
			: m_allocator(allocator.get()), m_marker(allocator ? allocator->GetMarker() : 0) {
			if (!allocator) {
				// ログ出す
			}
		}

		~StackAllocatorScopePtr() {
			if (m_allocator) {
				m_allocator->Rewind(m_marker);
			}
		}

		// コピー禁止/ムーブ禁止
		StackAllocatorScopePtr(const StackAllocatorScopePtr&) = delete;
		StackAllocatorScopePtr& operator=(const StackAllocatorScopePtr&) = delete;
		StackAllocatorScopePtr(StackAllocatorScopePtr&&) = delete;
		StackAllocatorScopePtr& operator=(StackAllocatorScopePtr&&) = delete;

	private:
		StackAllocator* m_allocator;
		size_t m_marker;
	};
} // namespace elk::memory

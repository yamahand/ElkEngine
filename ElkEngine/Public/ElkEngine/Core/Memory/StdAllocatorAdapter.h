#pragma once

#include "Core/Memory/IAllocator.h"

namespace elk::memory {

    template<typename T>
    class StdAllocatorAdapter {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using propagate_on_container_move_assignment = std::true_type;

        explicit StdAllocatorAdapter(IAllocator* allocator) noexcept
            : m_allocator(allocator) {
        }

        template<typename U>
        StdAllocatorAdapter(const StdAllocatorAdapter<U>& other) noexcept
            : m_allocator(other.m_allocator) {
        }

        [[nodiscard]] T* allocate(size_type n) {
            if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
                throw std::bad_array_new_length();
            }

            void* ptr = m_allocator->Allocate(n * sizeof(T), alignof(T));
            if (!ptr) {
                throw std::bad_alloc();
            }

            return static_cast<T*>(ptr);
        }

        void deallocate(T* ptr, size_type) noexcept {
            m_allocator->Deallocate(ptr);
        }

        template<typename U>
        bool operator==(const StdAllocatorAdapter<U>& other) const noexcept {
            return m_allocator == other.m_allocator;
        }

        template<typename U>
        bool operator!=(const StdAllocatorAdapter<U>& other) const noexcept {
            return !(*this == other);
        }

        template<typename U>
        friend class StdAllocatorAdapter;

    private:
        IAllocator* m_allocator;
    };

} // namespace elk::memory} // namespace elk::memory
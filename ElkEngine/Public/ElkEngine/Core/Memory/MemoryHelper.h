#pragma once

#include "Core/Memory/IAllocator.h"
#include "Core/Memory/StdAllocatorAdapter.h"

namespace elk::memory {


    // === スマートポインタ用デリータ ===

    /**
     * @brief IAllocator用のカスタムデリータ
     */
    template<typename T>
    class AllocatorDeleter {
    public:
        explicit AllocatorDeleter(IAllocator* allocator = nullptr) noexcept
            : allocator_(allocator) {
        }

        void operator()(T* ptr) const noexcept {
            if (ptr && allocator_) {
                ptr->~T();  // デストラクタ呼び出し
                allocator_->Deallocate(ptr);
            }
        }

    private:
        IAllocator* allocator_;
    };

    template<typename T>
    using UniquePtr = std::unique_ptr<T, AllocatorDeleter<T>>;

    // === ヘルパー関数 ===

    /**
     * @brief アロケータを使って new
     */
    template<typename T, typename... Args>
    [[nodiscard]] T* AllocateNew(IAllocator* allocator, Args&&... args) {
        void* memory = allocator->Allocate(sizeof(T), alignof(T));
        if (!memory) {
            return nullptr;
        }

        try {
            return new(memory) T(std::forward<Args>(args)...);
        }
        catch (...) {
            allocator->Deallocate(memory);
            throw;
        }
    }

    /**
     * @brief アロケータを使って delete
     */
    template<typename T>
    void AllocateDelete(IAllocator* allocator, T* ptr) noexcept {
        if (ptr && allocator) {
            ptr->~T();
            allocator->Deallocate(ptr);
        }
    }

    /**
     * @brief アロケータを使って UniquePtr を作成
     */
    template<typename T, typename... Args>
    [[nodiscard]] UniquePtr<T> MakeUnique(IAllocator* allocator, Args&&... args) {
        T* ptr = AllocateNew<T>(allocator, std::forward<Args>(args)...);
        return UniquePtr<T>(ptr, AllocatorDeleter<T>(allocator));
    }
}
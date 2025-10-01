#include "Core/Memory/MemoryManager.h"

#include <cstdlib> // for malloc, free

namespace elk::memory
{
    bool MemoryManager::Initialize(size_t size)
    {
        m_totalSize = size;
        m_memoryPool = malloc(size);
        if (!m_memoryPool) {
            return false;
        }
        return true;
    }

    void MemoryManager::Shutdown()
    {
        free(m_memoryPool);
        m_memoryPool = nullptr;
        m_totalSize = 0;
    }

} // namespace elk::memory

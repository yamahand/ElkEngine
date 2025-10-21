#include "Core/Memory/StackAllocator.h"
#include "Core/Logger/Logger.h"
#include "Core/Logger/LoggerService.h"

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
            ELK_LOG_ERROR("memory", "StackAllocator initialized with null memory pointer.");
        }
	}

	memory::StackAllocator::~StackAllocator()
	{
	}

} // namespace elk::memory

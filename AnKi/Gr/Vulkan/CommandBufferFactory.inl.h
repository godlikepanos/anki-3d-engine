// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/CommandBufferFactory.h>

namespace anki {

inline HeapMemoryPool& MicroCommandBuffer::getMemoryPool()
{
	return m_threadAlloc->getMemoryPool();
}

inline void MicroCommandBufferPtrDeleter::operator()(MicroCommandBuffer* ptr)
{
	ANKI_ASSERT(ptr);
	ptr->m_threadAlloc->deleteCommandBuffer(ptr);
}

inline HeapMemoryPool& CommandBufferThreadAllocator::getMemoryPool()
{
	return *m_factory->m_pool;
}

} // end namespace anki

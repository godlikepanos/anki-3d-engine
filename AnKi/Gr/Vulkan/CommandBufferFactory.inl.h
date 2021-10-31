// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/CommandBufferFactory.h>

namespace anki {

inline GrAllocator<U8>& MicroCommandBuffer::getAllocator()
{
	return m_threadAlloc->getAllocator();
}

inline void MicroCommandBufferPtrDeleter::operator()(MicroCommandBuffer* ptr)
{
	ANKI_ASSERT(ptr);
	ptr->m_threadAlloc->deleteCommandBuffer(ptr);
}

inline GrAllocator<U8>& CommandBufferThreadAllocator::getAllocator()
{
	return m_factory->m_alloc;
}

} // end namespace anki

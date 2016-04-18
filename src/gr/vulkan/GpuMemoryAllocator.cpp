// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GpuMemoryAllocator.h>

namespace anki
{

//==============================================================================
// GpuMemoryAllocatorChunk                                                     =
//==============================================================================
class GpuMemoryAllocatorChunk
	: public IntrusiveListEnabled<GpuMemoryAllocatorChunk>
{
public:
	/// GPU mem.
	VkDeviceMemory m_mem;

	/// Size of the allocation.
	PtrSize m_size;

	/// Current offset.
	PtrSize m_offset;

	/// Number of allocations.
	U32 m_allocationCount;
};

//==============================================================================
// GpuMemoryAllocator                                                          =
//==============================================================================

//==============================================================================
GpuMemoryAllocator::~GpuMemoryAllocator()
{
}

//==============================================================================
void GpuMemoryAllocator::init(GenericMemoryPoolAllocator<U8> alloc,
	VkDevice dev,
	U memoryTypeIdx,
	PtrSize chunkInitialSize,
	F32 nextChunkScale,
	PtrSize nextChunkBias)
{
	m_alloc = alloc;
	m_dev = dev;
	m_memIdx = memoryTypeIdx;
	m_initSize = chunkInitialSize;
	m_scale = nextChunkScale;
	m_bias = nextChunkBias;
}

//==============================================================================
Bool GpuMemoryAllocator::allocateFromChunk(
	PtrSize size, U alignment, Chunk& ch, GpuMemoryAllocationHandle& handle)
{
	alignRoundUp(alignment, ch.m_offset);
	if(ch.m_offset + size <= ch.m_size)
	{
		++ch.m_allocationCount;

		handle.m_memory = ch.m_mem;
		handle.m_offset = ch.m_offset;
		handle.m_chunk = &ch;
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
void GpuMemoryAllocator::createNewChunk()
{
	// Determine new chunk size
	PtrSize newChunkSize;
	if(!m_activeChunks.isEmpty())
	{
		newChunkSize = m_activeChunks.getBack().m_size * m_scale + m_bias;
	}
	else
	{
		newChunkSize = m_initSize;
	}

	Chunk* chunk = m_alloc.newInstance<Chunk>();
	chunk->m_size = newChunkSize;
	chunk->m_offset = 0;
	chunk->m_allocationCount = 0;

	VkMemoryAllocateInfo inf;
	inf.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	inf.pNext = nullptr;
	inf.allocationSize = newChunkSize;
	inf.memoryTypeIndex = m_memIdx;

	if(vkAllocateMemory(m_dev, &inf, nullptr, &chunk->m_mem))
	{
		ANKI_LOGF("Out of GPU memory");
	}

	m_activeChunks.pushBack(chunk);
}

//==============================================================================
void GpuMemoryAllocator::allocate(
	PtrSize size, U alignment, GpuMemoryAllocationHandle& handle)
{
	handle.m_memory = VK_NULL_HANDLE;

	LockGuard<Mutex> lock(m_mtx);

	if(m_activeChunks.isEmpty()
		|| allocateFromChunk(size, alignment, m_activeChunks.getBack(), handle))
	{
		createNewChunk();
	}

	if(handle.m_memory == VK_NULL_HANDLE)
	{
		Bool success = allocateFromChunk(
			size, alignment, m_activeChunks.getBack(), handle);
		(void)success;
		ANKI_ASSERT(success && "The chunk should have space");
	}
}

//==============================================================================
void GpuMemoryAllocator::free(const GpuMemoryAllocationHandle& handle)
{
	ANKI_ASSERT(handle.m_memory && handle.m_chunk);

	LockGuard<Mutex> lock(m_mtx);

	ANKI_ASSERT(handle.m_chunk->m_allocationCount > 0);

	--handle.m_chunk->m_allocationCount;
	if(handle.m_chunk->m_allocationCount == 0)
	{
		m_activeChunks.erase(handle.m_chunk);
		vkFreeMemory(m_dev, handle.m_chunk->m_mem, nullptr);
		m_alloc.deleteInstance(handle.m_chunk);
	}
}

} // end namespace anki
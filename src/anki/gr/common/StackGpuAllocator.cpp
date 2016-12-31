// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/StackGpuAllocator.h>

namespace anki
{

class StackGpuAllocatorChunk
{
public:
	StackGpuAllocatorChunk* m_next;
	StackGpuAllocatorMemory* m_mem;
	Atomic<U32> m_offset;
	PtrSize m_size;
};

StackGpuAllocator::~StackGpuAllocator()
{
	Chunk* chunk = m_chunkListHead;
	while(chunk)
	{
		if(chunk->m_mem)
		{
			m_iface->free(chunk->m_mem);
		}

		Chunk* next = chunk->m_next;
		m_alloc.deleteInstance(chunk);
		chunk = next;
	}
}

void StackGpuAllocator::init(GenericMemoryPoolAllocator<U8> alloc, StackGpuAllocatorInterface* iface)
{
	ANKI_ASSERT(iface);
	m_alloc = alloc;
	m_iface = iface;
	iface->getChunkGrowInfo(m_scale, m_bias, m_initialSize);
	ANKI_ASSERT(m_scale >= 1.0);
	ANKI_ASSERT(m_initialSize > 0);

	m_alignment = iface->getMaxAlignment();
	ANKI_ASSERT(m_alignment > 0);
	ANKI_ASSERT(m_initialSize >= m_alignment);
	ANKI_ASSERT((m_initialSize % m_alignment) == 0);
}

Error StackGpuAllocator::allocate(PtrSize size, StackGpuAllocatorHandle& handle)
{
	alignRoundUp(m_alignment, size);
	ANKI_ASSERT(size > 0);
	ANKI_ASSERT(size <= m_initialSize && "The chunks should have enough space to hold at least one allocation");

	Chunk* crntChunk;
	Bool retry = true;

	do
	{
		crntChunk = m_crntChunk.load();
		PtrSize offset;

		if(crntChunk && ((offset = crntChunk->m_offset.fetchAdd(size)) + size) <= crntChunk->m_size)
		{
			// All is fine, there is enough space in the chunk

			handle.m_memory = crntChunk->m_mem;
			handle.m_offset = offset;

			retry = false;
		}
		else
		{
			// Need new chunk

			LockGuard<Mutex> lock(m_lock);

			// Make sure that only one thread will create a new chunk
			if(m_crntChunk.load() == crntChunk)
			{
				// We can create a new chunk

				if(crntChunk == nullptr || crntChunk->m_next == nullptr)
				{
					// Need to create a new chunk

					Chunk* newChunk = m_alloc.newInstance<Chunk>();

					if(crntChunk)
					{
						crntChunk->m_next = newChunk;
						newChunk->m_size = crntChunk->m_size * m_scale + m_bias;
					}
					else
					{
						newChunk->m_size = m_initialSize;

						if(m_chunkListHead == nullptr)
						{
							m_chunkListHead = newChunk;
						}
					}
					alignRoundUp(m_alignment, newChunk->m_size);

					newChunk->m_next = nullptr;
					newChunk->m_offset.set(0);
					ANKI_CHECK(m_iface->allocate(newChunk->m_size, newChunk->m_mem));

					m_crntChunk.store(newChunk);
				}
				else
				{
					// Need to recycle one

					crntChunk->m_next->m_offset.set(0);

					m_crntChunk.store(crntChunk->m_next);
				}
			}
		}
	} while(retry);

	return ErrorCode::NONE;
}

void StackGpuAllocator::reset()
{
	m_crntChunk.set(m_chunkListHead);
	if(m_chunkListHead)
	{
		m_chunkListHead->m_offset.set(0);
	}
}

} // end namespace anki

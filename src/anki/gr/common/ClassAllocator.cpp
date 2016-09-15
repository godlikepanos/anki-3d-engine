// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/ClassAllocator.h>
#include <anki/util/List.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// Max number of sub allocations (aka slots) per chunk.
const U MAX_SLOTS_PER_CHUNK = 128;

class ClassAllocatorChunk : public IntrusiveListEnabled<ClassAllocatorChunk>
{
public:
	/// mem.
	ClassAllocatorMemory* m_mem;

	/// The in use slots mask.
	BitSet<MAX_SLOTS_PER_CHUNK, U8> m_inUseSlots = {false};

	/// The number of in-use slots.
	U32 m_inUseSlotCount = 0;

	/// The owner.
	ClassAllocatorClass* m_class = nullptr;
};

class ClassAllocatorClass
{
public:
	/// The active chunks.
	IntrusiveList<ClassAllocatorChunk> m_inUseChunks;

	/// The size of each chunk.
	PtrSize m_chunkSize = 0;

	/// The max slot size for this class.
	U32 m_maxSlotSize = 0;

	/// The number of slots for a single chunk.
	U32 m_slotsPerChunkCount = 0;

	Mutex m_mtx;
};

void ClassAllocator::init(GenericMemoryPoolAllocator<U8> alloc, ClassAllocatorInterface* iface)
{
	ANKI_ASSERT(iface);
	m_alloc = alloc;
	m_iface = iface;

	//
	// Initialize the classes
	//
	U classCount = iface->getClassCount();
	m_classes.create(m_alloc, classCount);
	
	for(U i = 0; i < classCount; ++i)
	{
		PtrSize slotSize, chunkSize;
		m_iface->getClassInfo(i, slotSize, chunkSize);
		ANKI_ASSERT(isPowerOfTwo(slotSize));
		ANKI_ASSERT((chunkSize % slotSize) == 0);
		ANKI_ASSERT((chunkSize / slotSize) <= MAX_SLOTS_PER_CHUNK);

		Class& c = m_classes[i];
		c.m_chunkSize = chunkSize;
		c.m_maxSlotSize = slotSize;
		c.m_slotsPerChunkCount = chunkSize / slotSize;
	}
}

ClassAllocator::Class* ClassAllocator::findClass(PtrSize size, U alignment)
{
	ANKI_ASSERT(size > 0 && alignment > 0);

	PtrSize lowLimit = 0;
	Class* it = m_classes.getBegin();
	const Class* end = m_classes.getEnd();

	while(it != end)
	{
		PtrSize highLimit = it->m_maxSlotSize;

		if(size > lowLimit && size <= highLimit)
		{
			if(alignment <= highLimit)
			{
				// Found the class
				return it;
			}
			else
			{
				// The class found doesn't have the proper alignment. Need to go higher

				while(++it != end)
				{
					if(alignment <= it->m_maxSlotSize)
					{
						// Now found something
						return it;
					}
				}
			}
		}

		lowLimit = highLimit;
		++it;
	}

	ANKI_ASSERT(!"No class found");
	return nullptr;
}

ClassAllocator::Chunk* ClassAllocator::findChunkWithUnusedSlot(Class& cl)
{
	auto it = cl.m_inUseChunks.getBegin();
	const auto end = cl.m_inUseChunks.getEnd();
	while(it != end)
	{
		if(it->m_inUseSlotCount < cl.m_slotsPerChunkCount)
		{
			return &(*it);
		}

		++it;
	}

	return nullptr;
}

Error ClassAllocator::createChunk(Class& cl, Chunk*& chunk)
{
	chunk = m_alloc.newInstance<Chunk>();

	ANKI_CHECK(m_iface->allocate(&cl - &m_classes[0], chunk->m_mem));
	ANKI_ASSERT(chunk->m_mem);
	chunk->m_class = &cl;
	
	return ErrorCode::NONE;
}

void ClassAllocator::destroyChunk(Class& cl, Chunk& chunk)
{
	cl.m_inUseChunks.erase(&chunk);
	m_iface->free(chunk.m_mem);
	m_alloc.deleteInstance(&chunk);
}

Error ClassAllocator::allocate(PtrSize size, U alignment, ClassAllocatorHandle& handle)
{
	ANKI_ASSERT(!handle);
	ANKI_ASSERT(handle.valid());

	// Find the class for the given size
	Class* cl = findClass(size, alignment);

	LockGuard<Mutex> lock(cl->m_mtx);
	Chunk* chunk = findChunkWithUnusedSlot(*cl);

	// Create a new chunk if needed
	if(chunk == nullptr)
	{
		ANKI_CHECK(createChunk(*cl, chunk));
	}

	// Allocate from chunk
	U bitCount = cl->m_slotsPerChunkCount;
	for(U i = 0; i < bitCount; ++i)
	{
		if(!chunk->m_inUseSlots.get(i))
		{
			// Found an empty slot, allocate from it
			chunk->m_inUseSlots.set(i);
			++chunk->m_inUseSlotCount;

			handle.m_memory = chunk->m_mem;
			handle.m_offset = i * cl->m_maxSlotSize;
			handle.m_chunk = chunk;

			break;
		}
	}

	ANKI_ASSERT(handle.m_memory && handle.m_chunk);
	ANKI_ASSERT(isAligned(alignment, handle.m_offset));
	return ErrorCode::NONE;
}

void ClassAllocator::free(ClassAllocatorHandle& handle)
{
	ANKI_ASSERT(handle);
	ANKI_ASSERT(handle.valid());

	Chunk& chunk = *handle.m_chunk;
	Class& cl = *chunk.m_class;

	LockGuard<Mutex> lock(cl.m_mtx);
	U slotIdx = handle.m_offset / cl.m_maxSlotSize;

	ANKI_ASSERT(chunk.m_inUseSlots.get(slotIdx));
	ANKI_ASSERT(chunk.m_inUseSlotCount > 0);
	chunk.m_inUseSlots.unset(slotIdx);
	--chunk.m_inUseSlotCount;

	if(chunk.m_inUseSlotCount == 0)
	{
		destroyChunk(cl, chunk);
	}

	handle = {};
}

} // end namespace anki

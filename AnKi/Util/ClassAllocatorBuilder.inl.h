// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/ClassAllocatorBuilder.h>

namespace anki {

template<typename TChunk, typename TInterface, typename TLock>
void ClassAllocatorBuilder<TChunk, TInterface, TLock>::init(BaseMemoryPool* pool)
{
	ANKI_ASSERT(pool);
	m_pool = pool;

	m_classes.create(*m_pool, m_interface.getClassCount());

	for(U32 classIdx = 0; classIdx < m_classes.getSize(); ++classIdx)
	{
		Class& c = m_classes[classIdx];

		m_interface.getClassInfo(classIdx, c.m_chunkSize, c.m_suballocationSize);
		ANKI_ASSERT(c.m_suballocationSize > 0 && c.m_chunkSize > 0 && c.m_chunkSize >= c.m_suballocationSize);
		ANKI_ASSERT((c.m_chunkSize % c.m_suballocationSize) == 0);
	}
}

template<typename TChunk, typename TInterface, typename TLock>
void ClassAllocatorBuilder<TChunk, TInterface, TLock>::destroy()
{
	for([[maybe_unused]] const Class& c : m_classes)
	{
		ANKI_ASSERT(c.m_chunkList.isEmpty() && "Forgot to deallocate");
	}

	m_classes.destroy(*m_pool);
}

template<typename TChunk, typename TInterface, typename TLock>
typename ClassAllocatorBuilder<TChunk, TInterface, TLock>::Class*
ClassAllocatorBuilder<TChunk, TInterface, TLock>::findClass(PtrSize size, PtrSize alignment)
{
	ANKI_ASSERT(size > 0 && alignment > 0);

	PtrSize lowLimit = 0;
	Class* it = m_classes.getBegin();
	const Class* end = m_classes.getEnd();

	while(it != end)
	{
		const PtrSize highLimit = it->m_suballocationSize;

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
					if(alignment <= it->m_suballocationSize)
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

	ANKI_UTIL_LOGF("Memory class not found");
	return nullptr;
}

template<typename TChunk, typename TInterface, typename TLock>
Error ClassAllocatorBuilder<TChunk, TInterface, TLock>::allocate(PtrSize size, PtrSize alignment, TChunk*& chunk,
																 PtrSize& offset)
{
	ANKI_ASSERT(isInitialized());
	ANKI_ASSERT(size > 0 && alignment > 0);

	chunk = nullptr;
	offset = kMaxPtrSize;

	// Find the class for the given size
	Class* cl = findClass(size, alignment);
	const PtrSize maxSuballocationCount = cl->m_chunkSize / cl->m_suballocationSize;

	LockGuard<TLock> lock(cl->m_mtx);

	// Find chunk with free suballocation
	auto it = cl->m_chunkList.getBegin();
	const auto end = cl->m_chunkList.getEnd();
	while(it != end)
	{
		if(it->m_suballocationCount < maxSuballocationCount)
		{
			chunk = &(*it);
			break;
		}

		++it;
	}

	// Create a new chunk if needed
	if(chunk == nullptr)
	{
		ANKI_CHECK(m_interface.allocateChunk(U32(cl - &m_classes[0]), chunk));

		chunk->m_inUseSuballocations.unsetAll();
		chunk->m_suballocationCount = 0;
		chunk->m_class = cl;

		cl->m_chunkList.pushBack(chunk);
	}

	// Allocate from chunk
	const U32 bitCount = U32(maxSuballocationCount);
	for(U32 i = 0; i < bitCount; ++i)
	{
		if(!chunk->m_inUseSuballocations.get(i))
		{
			// Found an empty slot, allocate from it
			chunk->m_inUseSuballocations.set(i);
			++chunk->m_suballocationCount;

			offset = i * cl->m_suballocationSize;
			break;
		}
	}

	ANKI_ASSERT(chunk);
	ANKI_ASSERT(isAligned(alignment, offset));
	ANKI_ASSERT(offset + size <= cl->m_chunkSize);
	return Error::kNone;
}

template<typename TChunk, typename TInterface, typename TLock>
void ClassAllocatorBuilder<TChunk, TInterface, TLock>::free(TChunk* chunk, PtrSize offset)
{
	ANKI_ASSERT(isInitialized());
	ANKI_ASSERT(chunk);

	Class& cl = *static_cast<Class*>(chunk->m_class);

	ANKI_ASSERT(offset < cl.m_chunkSize);

	LockGuard<TLock> lock(cl.m_mtx);

	const U32 suballocationIdx = U32(offset / cl.m_suballocationSize);

	ANKI_ASSERT(chunk->m_inUseSuballocations.get(suballocationIdx));
	ANKI_ASSERT(chunk->m_suballocationCount > 0);
	chunk->m_inUseSuballocations.unset(suballocationIdx);
	--chunk->m_suballocationCount;

	if(chunk->m_suballocationCount == 0)
	{
		cl.m_chunkList.erase(chunk);
		m_interface.freeChunk(chunk);
	}
}

template<typename TChunk, typename TInterface, typename TLock>
void ClassAllocatorBuilder<TChunk, TInterface, TLock>::getStats(ClassAllocatorBuilderStats& stats) const
{
	stats = {};

	for(const Class& c : m_classes)
	{
		LockGuard<TLock> lock(c.m_mtx);

		for(const TChunk& chunk : c.m_chunkList)
		{
			stats.m_allocatedSize += c.m_chunkSize;
			stats.m_inUseSize += c.m_suballocationSize * chunk.m_suballocationCount;
			++stats.m_chunkCount;
		}
	}
}

} // end namespace anki

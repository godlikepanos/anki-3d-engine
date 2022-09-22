// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/StackAllocatorBuilder.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Logger.h>

namespace anki {

template<typename TChunk, typename TInterface, typename TLock>
void StackAllocatorBuilder<TChunk, TInterface, TLock>::destroy()
{
	// Free chunks
	TChunk* chunk = m_chunksListHead;
	while(chunk)
	{
		TChunk* next = chunk->m_nextChunk;
		m_interface.freeChunk(chunk);
		chunk = next;
	}

	// Do some error checks
	Atomic<U32>* allocationCount = m_interface.getAllocationCount();
	if(allocationCount)
	{
		if(!m_interface.ignoreDeallocationErrors() && allocationCount->load() != 0)
		{
			ANKI_UTIL_LOGW("Forgot to deallocate");
		}
	}

	m_crntChunk.setNonAtomically(nullptr);
	m_chunksListHead = nullptr;
	m_memoryCapacity = 0;
	m_chunkCount = 0;
}

template<typename TChunk, typename TInterface, typename TLock>
Error StackAllocatorBuilder<TChunk, TInterface, TLock>::allocate(PtrSize size, [[maybe_unused]] PtrSize alignment,
																 TChunk*& chunk, PtrSize& offset)
{
	ANKI_ASSERT(alignment <= m_interface.getMaxAlignment());

	size = getAlignedRoundUp(m_interface.getMaxAlignment(), size);
	ANKI_ASSERT(size > 0);

	chunk = nullptr;
	offset = kMaxPtrSize;

	while(true)
	{
		// Try to allocate from the current chunk, if there is one
		TChunk* crntChunk = m_crntChunk.load();
		if(crntChunk)
		{
			offset = crntChunk->m_offsetInChunk.fetchAdd(size);
		}

		if(crntChunk && offset + size <= crntChunk->m_chunkSize)
		{
			// All is fine, there is enough space in the chunk

			chunk = crntChunk;

			Atomic<U32>* allocationCount = m_interface.getAllocationCount();
			if(allocationCount)
			{
				allocationCount->fetchAdd(1);
			}

			break;
		}
		else
		{
			// Need new chunk

			LockGuard<TLock> lock(m_lock);

			// Make sure that only one thread will create a new chunk
			const Bool someOtherThreadCreatedAChunkWhileIWasHoldingTheLock = m_crntChunk.load() != crntChunk;
			if(someOtherThreadCreatedAChunkWhileIWasHoldingTheLock)
			{
				continue;
			}

			// We can create a new chunk

			// Compute the memory of the new chunk. Don't look at any previous chunk
			PtrSize nextChunkSize = m_interface.getInitialChunkSize();
			ANKI_ASSERT(nextChunkSize > 0);
			for(U32 i = 0; i < m_chunkCount; ++i)
			{
				const F64 scale = m_interface.getNextChunkGrowScale();
				ANKI_ASSERT(scale >= 1.0);
				nextChunkSize = PtrSize(F64(nextChunkSize) * scale) + m_interface.getNextChunkGrowBias();
				ANKI_ASSERT(nextChunkSize > 0);
			}

			nextChunkSize = max(size, nextChunkSize); // Can't have the allocation fail
			alignRoundUp(m_interface.getMaxAlignment(), nextChunkSize); // Align again

			TChunk* nextChunk = (crntChunk) ? crntChunk->m_nextChunk : nullptr;

			if(nextChunk && nextChunk->m_chunkSize == nextChunkSize)
			{
				// Will recycle

				crntChunk->m_nextChunk->m_offsetInChunk.store(0);
				m_interface.recycleChunk(*nextChunk);
				m_crntChunk.store(nextChunk);
			}
			else
			{
				// There is no chunk or there is but it's too small

				// Do that first because it might throw error
				TChunk* newNextChunk;
				ANKI_CHECK(m_interface.allocateChunk(nextChunkSize, newNextChunk));
				newNextChunk->m_nextChunk = nullptr;
				newNextChunk->m_offsetInChunk.setNonAtomically(0);
				newNextChunk->m_chunkSize = nextChunkSize;
				++m_chunkCount;

				// Remove the existing next chunk if there is one
				TChunk* nextNextChunk = nullptr;
				if(nextChunk)
				{
					nextNextChunk = nextChunk->m_nextChunk;
					m_interface.freeChunk(nextChunk);
					nextChunk = nullptr;
					--m_chunkCount;
				}

				// Do list stuff
				if(crntChunk)
				{
					crntChunk->m_nextChunk = newNextChunk;
					ANKI_ASSERT(m_chunksListHead != nullptr);
				}
				else
				{
					ANKI_ASSERT(m_chunksListHead == nullptr);
					m_chunksListHead = newNextChunk;
				}

				newNextChunk->m_nextChunk = nextNextChunk;

				m_crntChunk.store(newNextChunk);

				m_memoryCapacity += nextChunkSize;
			}
		}
	}

	ANKI_ASSERT(chunk && offset != kMaxPtrSize);
	return Error::kNone;
}

template<typename TChunk, typename TInterface, typename TLock>
void StackAllocatorBuilder<TChunk, TInterface, TLock>::free()
{
	Atomic<U32>* allocationCount = m_interface.getAllocationCount();
	if(allocationCount)
	{
		[[maybe_unused]] const U32 count = allocationCount->fetchSub(1);
		ANKI_ASSERT(count > 0);
	}
}

template<typename TChunk, typename TInterface, typename TLock>
void StackAllocatorBuilder<TChunk, TInterface, TLock>::reset()
{
	m_crntChunk.setNonAtomically(m_chunksListHead);

	if(m_chunksListHead)
	{
		m_chunksListHead->m_offsetInChunk.setNonAtomically(0);
	}

	// Reset allocation count and do some error checks
	Atomic<U32>* allocationCount = m_interface.getAllocationCount();
	if(allocationCount)
	{
		const U32 allocCount = allocationCount->exchange(0);
		if(!m_interface.ignoreDeallocationErrors() && allocCount != 0)
		{
			ANKI_UTIL_LOGW("Forgot to deallocate");
		}
	}
}

} // end namespace anki

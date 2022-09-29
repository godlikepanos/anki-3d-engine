// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/ObjectAllocator.h>

namespace anki {

template<PtrSize kTObjectSize, U32 kTObjectAlignment, U32 kTObjectsPerChunk, typename TIndexType>
template<typename T, typename TMemPool, typename... TArgs>
T* ObjectAllocator<kTObjectSize, kTObjectAlignment, kTObjectsPerChunk, TIndexType>::newInstance(TMemPool& pool,
																								TArgs&&... args)
{
	static_assert(alignof(T) <= kObjectAlignment, "Wrong object alignment");
	static_assert(sizeof(T) <= kObjectSize, "Wrong object size");

	T* out = nullptr;

	// Try find one in the chunks
	Chunk* chunk = m_chunksHead;
	while(chunk)
	{
		if(chunk->m_unusedCount > 0)
		{
			// Pop an element
			--chunk->m_unusedCount;
			out = reinterpret_cast<T*>(&chunk->m_objects[chunk->m_unusedStack[chunk->m_unusedCount]]);
			break;
		}

		chunk = chunk->m_next;
	}

	if(out == nullptr)
	{
		// Need to create a new chunk

		// Create the chunk
		chunk = anki::newInstance<Chunk>(pool);
		chunk->m_unusedCount = kObjectsPerChunk;

		for(U32 i = 0; i < kObjectsPerChunk; ++i)
		{
			chunk->m_unusedStack[i] = kObjectsPerChunk - (i + 1);
		}

		if(m_chunksTail)
		{
			ANKI_ASSERT(m_chunksHead);
			chunk->m_prev = m_chunksTail;
			m_chunksTail->m_next = chunk;
			m_chunksTail = chunk;
		}
		else
		{
			m_chunksTail = m_chunksHead = chunk;
		}

		// Allocate one object
		out = reinterpret_cast<T*>(&chunk->m_objects[0]);
		--chunk->m_unusedCount;
	}

	ANKI_ASSERT(out);

	// Construct it
	callConstructor(*out, std::forward<TArgs>(args)...);

	return out;
}

template<PtrSize kTObjectSize, U32 kTObjectAlignment, U32 kTObjectsPerChunk, typename TIndexType>
template<typename T, typename TMemPool>
void ObjectAllocator<kTObjectSize, kTObjectAlignment, kTObjectsPerChunk, TIndexType>::deleteInstance(TMemPool& pool,
																									 T* obj)
{
	static_assert(alignof(T) <= kObjectAlignment, "Wrong object alignment");
	static_assert(sizeof(T) <= kObjectSize, "Wrong object size");

	ANKI_ASSERT(obj);

	// Find the chunk the obj is in
	const Object* const mem = reinterpret_cast<Object*>(obj);
	Chunk* chunk = m_chunksHead;
	while(chunk)
	{
		const Object* const begin = chunk->m_objects.getBegin();
		const Object* const end = chunk->m_objects.getEnd();
		if(mem >= begin && mem < end)
		{
			// Found it, remove it from the chunk and maybe delete the chunk

			ANKI_ASSERT(chunk->m_unusedCount < kObjectsPerChunk);
			const U32 idx = U32(mem - begin);

			// Destroy the object
			callDestructor(*obj);

			// Remove from the chunk
			chunk->m_unusedStack[chunk->m_unusedCount] = idx;
			++chunk->m_unusedCount;

			// Delete the chunk if it's empty
			if(chunk->m_unusedCount == kObjectsPerChunk)
			{
				if(chunk == m_chunksTail)
				{
					m_chunksTail = chunk->m_prev;
				}

				if(chunk == m_chunksHead)
				{
					m_chunksHead = chunk->m_next;
				}

				if(chunk->m_prev)
				{
					ANKI_ASSERT(chunk->m_prev->m_next == chunk);
					chunk->m_prev->m_next = chunk->m_next;
				}

				if(chunk->m_next)
				{
					ANKI_ASSERT(chunk->m_next->m_prev == chunk);
					chunk->m_next->m_prev = chunk->m_prev;
				}

				anki::deleteInstance(pool, chunk);
			}

			break;
		}

		chunk = chunk->m_next;
	}

	ANKI_ASSERT(chunk != nullptr);
}

} // end namespace anki

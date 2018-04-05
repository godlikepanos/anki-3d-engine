// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/ObjectAllocator.h>

namespace anki
{

template<typename T, U32 T_CHUNK_SIZE>
template<typename TAlloc, typename... TArgs>
T* ObjectAllocator<T, T_CHUNK_SIZE>::newInstance(TAlloc& alloc, TArgs&&... args)
{
	Value* out = nullptr;

	// Try find one in the chunks
	Chunk* chunk = m_chunksHead;
	while(chunk)
	{
		if(chunk->m_unusedCount > 0)
		{
			// Pop an element
			--chunk->m_unusedCount;
			out = &chunk->m_objects[chunk->m_unusedStack[chunk->m_unusedCount]];
			break;
		}

		chunk = chunk->m_next;
	}

	if(out == nullptr)
	{
		// Need to create a new chunk

		// Create the chunk
		Chunk* newChunk = alloc.template newInstance<Chunk>();
		newChunk->m_unusedCount = CHUNK_SIZE;

		for(U i = 0; i < CHUNK_SIZE; ++i)
		{
			newChunk->m_unusedStack[i] = CHUNK_SIZE - (i + 1);
		}

		if(m_chunksTail)
		{
			ANKI_ASSERT(m_chunksHead);
			newChunk->m_prev = m_chunksTail;
			m_chunksTail->m_next = newChunk;
			m_chunksTail = newChunk;
		}
		else
		{
			m_chunksTail = m_chunksHead = newChunk;
		}

		// Allocate one object
		out = reinterpret_cast<Value*>(&chunk->m_objects[0].m_storage);
		--chunk->m_unusedCount;
	}

	ANKI_ASSERT(out);

	// Construct it
	::new(out) Value(std::forward<TArgs>(args)...);

	return out;
}

template<typename T, U32 T_CHUNK_SIZE>
template<typename TAlloc>
void ObjectAllocator<T, T_CHUNK_SIZE>::deleteInstance(TAlloc& alloc, Value* obj)
{
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
			// Found it

			ANKI_ASSERT(chunk->m_unusedCount < CHUNK_SIZE);
			const U idx = mem - begin;

			// Destroy the object
			obj->~Value();

			// Remove from the chunk
			chunk->m_unusedStack[chunk->m_unusedCount] = idx;
			++chunk->m_unusedCount;

			// If chunk is empty delete it
			if(chunk->m_unusedCount == CHUNK_SIZE)
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

				alloc.deleteInstance(chunk);
			}

			break;
		}

		chunk = chunk->m_next;
	}

	ANKI_ASSERT(chunk != nullptr);
}

} // end namespace anki

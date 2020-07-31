// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/ObjectAllocator.h>

namespace anki
{

template<PtrSize T_OBJECT_SIZE, U32 T_OBJECT_ALIGNMENT, U32 T_OBJECTS_PER_CHUNK, typename TIndexType>
template<typename T, typename TAlloc, typename... TArgs>
T* ObjectAllocator<T_OBJECT_SIZE, T_OBJECT_ALIGNMENT, T_OBJECTS_PER_CHUNK, TIndexType>::newInstance(TAlloc& alloc,
																									TArgs&&... args)
{
	static_assert(alignof(T) <= OBJECT_ALIGNMENT, "Wrong object alignment");
	static_assert(sizeof(T) <= OBJECT_SIZE, "Wrong object size");

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
		chunk = alloc.template newInstance<Chunk>();
		chunk->m_unusedCount = OBJECTS_PER_CHUNK;

		for(U32 i = 0; i < OBJECTS_PER_CHUNK; ++i)
		{
			chunk->m_unusedStack[i] = OBJECTS_PER_CHUNK - (i + 1);
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
	alloc.construct(out, std::forward<TArgs>(args)...);

	return out;
}

template<PtrSize T_OBJECT_SIZE, U32 T_OBJECT_ALIGNMENT, U32 T_OBJECTS_PER_CHUNK, typename TIndexType>
template<typename T, typename TAlloc>
void ObjectAllocator<T_OBJECT_SIZE, T_OBJECT_ALIGNMENT, T_OBJECTS_PER_CHUNK, TIndexType>::deleteInstance(TAlloc& alloc,
																										 T* obj)
{
	static_assert(alignof(T) <= OBJECT_ALIGNMENT, "Wrong object alignment");
	static_assert(sizeof(T) <= OBJECT_SIZE, "Wrong object size");

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

			ANKI_ASSERT(chunk->m_unusedCount < OBJECTS_PER_CHUNK);
			const U32 idx = U32(mem - begin);

			// Destroy the object
			obj->~T();

			// Remove from the chunk
			chunk->m_unusedStack[chunk->m_unusedCount] = idx;
			++chunk->m_unusedCount;

			// Delete the chunk if it's empty
			if(chunk->m_unusedCount == OBJECTS_PER_CHUNK)
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

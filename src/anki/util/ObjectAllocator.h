// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup util
/// @{

/// A simple allocator of objects of the same type.
template<typename T, U32 T_CHUNK_SIZE = 64>
class ObjectAllocator
{
public:
	using Value = T;
	static constexpr U32 CHUNK_SIZE = T_CHUNK_SIZE;

	/// Allocate and construct a new object instance.
	template<typename TAlloc, typename... TArgs>
	Value* newInstance(TAlloc& alloc, TArgs&&... args);

	/// Delete an object.
	template<typename TAlloc>
	void deleteInstance(TAlloc& alloc, Value* obj);

private:
	/// Storage with equal properties as the Value.
	struct alignas(alignof(Value)) Object
	{
		U8 m_storage[sizeof(Value)];
	};

	/// A  single allocation.
	class Chunk
	{
	public:
		Array<Object, CHUNK_SIZE> m_objects;
		Array<U32, CHUNK_SIZE> m_unusedStack;
		U32 m_unusedCount;

		Chunk* m_next = nullptr;
		Chunk* m_prev = nullptr;
	};

	Chunk* m_chunksHead = nullptr;
	Chunk* m_chunksTail = nullptr;
};
/// @}

} // end namespace anki

#include <anki/util/ObjectAllocator.inl.h>

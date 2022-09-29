// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Array.h>

namespace anki {

/// @addtogroup util_containers
/// @{

/// A simple allocator for objects of similar types.
/// @tparam kTObjectSize      The maximum size of the objects.
/// @tparam kTObjectAlignment The maximum alignment of the objects.
/// @tparam kTObjectsPerChunk How much memory (in objects) will be allocated at once.
/// @tparam TIndexType        If kTObjectsPerChunk>0xFF make it U16. If kTObjectsPerChunk>0xFFFF make it U32.
template<PtrSize kTObjectSize, U32 kTObjectAlignment, U32 kTObjectsPerChunk = 64, typename TIndexType = U8>
class ObjectAllocator
{
public:
	static constexpr PtrSize kObjectSize = kTObjectSize;
	static constexpr U32 kObjectAlignment = kTObjectAlignment;
	static constexpr U32 kObjectsPerChunk = kTObjectsPerChunk;

	ObjectAllocator()
	{
	}

	~ObjectAllocator()
	{
		ANKI_ASSERT(m_chunksHead == nullptr && m_chunksTail == nullptr && "Forgot to deallocate");
	}

	/// Allocate and construct a new object instance.
	/// @note Not thread-safe.
	template<typename T, typename TMemPool, typename... TArgs>
	T* newInstance(TMemPool& pool, TArgs&&... args);

	/// Delete an object.
	/// @note Not thread-safe.
	template<typename T, typename TMemPool>
	void deleteInstance(TMemPool& pool, T* obj);

private:
	/// Storage with equal properties as the object.
	struct alignas(kObjectAlignment) Object
	{
		U8 m_storage[kObjectSize];
	};

	/// A  single allocation.
	class Chunk
	{
	public:
		Array<Object, kObjectsPerChunk> m_objects;
		Array<U32, kObjectsPerChunk> m_unusedStack;
		U32 m_unusedCount;

		Chunk* m_next = nullptr;
		Chunk* m_prev = nullptr;
	};

	Chunk* m_chunksHead = nullptr;
	Chunk* m_chunksTail = nullptr;
};

/// Convenience wrapper for ObjectAllocator.
template<typename T, U32 kTObjectsPerChunk = 64, typename TIndexType = U8>
class ObjectAllocatorSameType : public ObjectAllocator<sizeof(T), U32(alignof(T)), kTObjectsPerChunk, TIndexType>
{
public:
	using Base = ObjectAllocator<sizeof(T), U32(alignof(T)), kTObjectsPerChunk, TIndexType>;

	/// Allocate and construct a new object instance.
	/// @note Not thread-safe.
	template<typename TMemPool, typename... TArgs>
	T* newInstance(TMemPool& pool, TArgs&&... args)
	{
		return Base::template newInstance<T>(pool, std::forward(args)...);
	}

	/// Delete an object.
	/// @note Not thread-safe.
	template<typename TMemPool>
	void deleteInstance(TMemPool& pool, T* obj)
	{
		Base::deleteInstance(pool, obj);
	}
};
/// @}

} // end namespace anki

#include <AnKi/Util/ObjectAllocator.inl.h>

// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// A simple allocator for objects of similar types.
/// @tparam T_OBJECT_SIZE       The maximum size of the objects.
/// @tparam T_OBJECT_ALIGNMENT  The maximum alignment of the objects.
/// @tparam T_OBJECTS_PER_CHUNK How much memory (in objects) will be allocated at once.
/// @tparam TIndexType          If T_OBJECTS_PER_CHUNK>0xFF make it U16. If T_OBJECTS_PER_CHUNK>0xFFFF make it U32.
template<PtrSize T_OBJECT_SIZE, U32 T_OBJECT_ALIGNMENT, U32 T_OBJECTS_PER_CHUNK = 64, typename TIndexType = U8>
class ObjectAllocator
{
public:
	static constexpr PtrSize OBJECT_SIZE = T_OBJECT_SIZE;
	static constexpr U32 OBJECT_ALIGNMENT = T_OBJECT_ALIGNMENT;
	static constexpr U32 OBJECTS_PER_CHUNK = T_OBJECTS_PER_CHUNK;

	ObjectAllocator()
	{
	}

	~ObjectAllocator()
	{
		ANKI_ASSERT(m_chunksHead == nullptr && m_chunksTail == nullptr && "Forgot to deallocate");
	}

	/// Allocate and construct a new object instance.
	/// @note Not thread-safe.
	template<typename T, typename TAlloc, typename... TArgs>
	T* newInstance(TAlloc& alloc, TArgs&&... args);

	/// Delete an object.
	/// @note Not thread-safe.
	template<typename T, typename TAlloc>
	void deleteInstance(TAlloc& alloc, T* obj);

private:
	/// Storage with equal properties as the object.
	struct alignas(OBJECT_ALIGNMENT) Object
	{
		U8 m_storage[OBJECT_SIZE];
	};

	/// A  single allocation.
	class Chunk
	{
	public:
		Array<Object, OBJECTS_PER_CHUNK> m_objects;
		Array<U32, OBJECTS_PER_CHUNK> m_unusedStack;
		U32 m_unusedCount;

		Chunk* m_next = nullptr;
		Chunk* m_prev = nullptr;
	};

	Chunk* m_chunksHead = nullptr;
	Chunk* m_chunksTail = nullptr;
};

/// Convenience wrapper for ObjectAllocator.
template<typename T, U32 T_OBJECTS_PER_CHUNK = 64, typename TIndexType = U8>
class ObjectAllocatorSameType : public ObjectAllocator<sizeof(T), alignof(T), T_OBJECTS_PER_CHUNK, TIndexType>
{
public:
	using Base = ObjectAllocator<sizeof(T), alignof(T), T_OBJECTS_PER_CHUNK, TIndexType>;

	/// Allocate and construct a new object instance.
	/// @note Not thread-safe.
	template<typename TAlloc, typename... TArgs>
	T* newInstance(TAlloc& alloc, TArgs&&... args)
	{
		return Base::template newInstance<T>(alloc, std::forward(args)...);
	}

	/// Delete an object.
	/// @note Not thread-safe.
	template<typename TAlloc>
	void deleteInstance(TAlloc& alloc, T* obj)
	{
		Base::deleteInstance(alloc, obj);
	}
};
/// @}

} // end namespace anki

#include <anki/util/ObjectAllocator.inl.h>

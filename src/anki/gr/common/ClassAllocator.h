// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

// Forward
class ClassAllocatorChunk;
class ClassAllocatorClass;

/// @addtogroup graphics
/// @{

/// The user defined output of an allocation.
class ClassAllocatorMemory
{
};

/// The user defined methods to allocate memory.
class ClassAllocatorInterface
{
public:
	virtual ~ClassAllocatorInterface()
	{
	}

	/// Allocate memory. Should be thread safe.
	virtual ANKI_USE_RESULT Error allocate(U classIdx, ClassAllocatorMemory*& mem) = 0;

	/// Free memory. Should be thread safe.
	virtual void free(ClassAllocatorMemory* mem) = 0;

	/// Get the number of classes.
	virtual U getClassCount() const = 0;

	/// Get info for a class. Each chunk will be chunkSize size and it can host chunkSize/slotSize sub-allocations in
	/// it.
	virtual void getClassInfo(U classIdx, PtrSize& slotSize, PtrSize& chunkSize) const = 0;
};

/// The output of an allocation.
class ClassAllocatorHandle
{
	friend class ClassAllocator;

public:
	ClassAllocatorMemory* m_memory = nullptr;
	PtrSize m_offset = 0;

	operator Bool() const
	{
		return m_memory != nullptr;
	}

private:
	ClassAllocatorChunk* m_chunk = nullptr;

	Bool valid() const
	{
		return (m_memory && m_chunk) || (m_memory == nullptr && m_chunk == nullptr);
	}
};

class ClassAllocatorStats
{
public:
	PtrSize m_totalMemoryUsage;
	PtrSize m_realMemoryUsage;
	F32 m_fragmentation; ///< XXX
};

/// Class based allocator.
class ClassAllocator : public NonCopyable
{
public:
	ClassAllocator()
	{
	}

	~ClassAllocator();

	void init(GenericMemoryPoolAllocator<U8> alloc, ClassAllocatorInterface* iface);

	/// Allocate memory.
	ANKI_USE_RESULT Error allocate(PtrSize size, U alignment, ClassAllocatorHandle& handle);

	/// Free allocated memory.
	void free(ClassAllocatorHandle& handle);

	void getStats(ClassAllocatorStats& stats) const;

private:
	using Class = ClassAllocatorClass;
	using Chunk = ClassAllocatorChunk;

	GenericMemoryPoolAllocator<U8> m_alloc;
	ClassAllocatorInterface* m_iface = nullptr;

	/// The memory classes.
	DynamicArray<Class> m_classes;

	Class* findClass(PtrSize size, U alignment);

	Chunk* findChunkWithUnusedSlot(Class& cl);

	/// Create a new chunk.
	ANKI_USE_RESULT Error createChunk(Class& cl, Chunk*& chunk);

	/// Destroy a chunk.
	void destroyChunk(Class& cl, Chunk& chunk);
};
/// @}

} // end namespace anki

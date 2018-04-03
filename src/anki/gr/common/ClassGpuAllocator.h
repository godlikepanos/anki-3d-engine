// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

// Forward
class ClassGpuAllocatorChunk;
class ClassGpuAllocatorClass;

/// @addtogroup graphics
/// @{

/// The user defined output of an allocation.
class ClassGpuAllocatorMemory
{
};

/// The user defined methods to allocate memory.
class ClassGpuAllocatorInterface
{
public:
	virtual ~ClassGpuAllocatorInterface()
	{
	}

	/// Allocate memory. Should be thread safe.
	virtual ANKI_USE_RESULT Error allocate(U classIdx, ClassGpuAllocatorMemory*& mem) = 0;

	/// Free memory. Should be thread safe.
	virtual void free(ClassGpuAllocatorMemory* mem) = 0;

	/// Get the number of classes.
	virtual U getClassCount() const = 0;

	/// Get info for a class. Each chunk will be chunkSize size and it can host chunkSize/slotSize sub-allocations in
	/// it.
	virtual void getClassInfo(U classIdx, PtrSize& slotSize, PtrSize& chunkSize) const = 0;
};

/// The output of an allocation.
class ClassGpuAllocatorHandle
{
	friend class ClassGpuAllocator;

public:
	ClassGpuAllocatorMemory* m_memory = nullptr;
	PtrSize m_offset = 0;

	operator Bool() const
	{
		return m_memory != nullptr;
	}

private:
	ClassGpuAllocatorChunk* m_chunk = nullptr;

	Bool valid() const
	{
		return (m_memory && m_chunk) || (m_memory == nullptr && m_chunk == nullptr);
	}
};

/// Class based allocator.
class ClassGpuAllocator : public NonCopyable
{
public:
	ClassGpuAllocator()
	{
	}

	~ClassGpuAllocator();

	void init(GenericMemoryPoolAllocator<U8> alloc, ClassGpuAllocatorInterface* iface);

	/// Allocate memory.
	ANKI_USE_RESULT Error allocate(PtrSize size, U alignment, ClassGpuAllocatorHandle& handle);

	/// Free allocated memory.
	void free(ClassGpuAllocatorHandle& handle);

	PtrSize getAllocatedMemory() const
	{
		return m_allocatedMem;
	}

private:
	using Class = ClassGpuAllocatorClass;
	using Chunk = ClassGpuAllocatorChunk;

	GenericMemoryPoolAllocator<U8> m_alloc;
	ClassGpuAllocatorInterface* m_iface = nullptr;

	/// The memory classes.
	DynamicArray<Class> m_classes;

	PtrSize m_allocatedMem = 0; ///< An estimate.

	Class* findClass(PtrSize size, U alignment);

	Chunk* findChunkWithUnusedSlot(Class& cl);

	/// Create a new chunk.
	ANKI_USE_RESULT Error createChunk(Class& cl, Chunk*& chunk);

	/// Destroy a chunk.
	void destroyChunk(Class& cl, Chunk& chunk);
};
/// @}

} // end namespace anki

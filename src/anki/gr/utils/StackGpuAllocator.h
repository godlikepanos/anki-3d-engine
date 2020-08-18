// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

// Forward
class StackGpuAllocatorChunk;

/// @addtogroup graphics
/// @{

/// The user defined output of an allocation.
class StackGpuAllocatorMemory
{
};

/// The user defined methods to allocate memory.
class StackGpuAllocatorInterface
{
public:
	virtual ~StackGpuAllocatorInterface()
	{
	}

	/// Allocate memory. Should be thread safe.
	virtual ANKI_USE_RESULT Error allocate(PtrSize size, StackGpuAllocatorMemory*& mem) = 0;

	/// Free memory. Should be thread safe.
	virtual void free(StackGpuAllocatorMemory* mem) = 0;

	/// If the current chunk of the linear allocator is of size N then the next chunk will be of size N*scale+bias.
	virtual void getChunkGrowInfo(F32& scale, PtrSize& bias, PtrSize& initialSize) = 0;

	virtual U32 getMaxAlignment() = 0;
};

/// The output of an allocation.
class StackGpuAllocatorHandle
{
	friend class StackGpuAllocator;

public:
	StackGpuAllocatorMemory* m_memory = nullptr;
	PtrSize m_offset = 0;

	explicit operator Bool() const
	{
		return m_memory != nullptr;
	}
};

/// Linear based allocator.
class StackGpuAllocator : public NonCopyable
{
public:
	StackGpuAllocator() = default;

	~StackGpuAllocator();

	void init(GenericMemoryPoolAllocator<U8> alloc, StackGpuAllocatorInterface* iface);

	/// Allocate memory.
	ANKI_USE_RESULT Error allocate(PtrSize size, StackGpuAllocatorHandle& handle);

	/// Reset all the memory chunks. Not thread safe.
	void reset();

	StackGpuAllocatorInterface* getInterface() const
	{
		ANKI_ASSERT(m_iface);
		return m_iface;
	}

private:
	using Chunk = StackGpuAllocatorChunk;

	GenericMemoryPoolAllocator<U8> m_alloc;
	StackGpuAllocatorInterface* m_iface = nullptr;

	Chunk* m_chunkListHead = nullptr;
	Atomic<Chunk*> m_crntChunk = {nullptr};
	Mutex m_lock;

	F32 m_scale = 0.0;
	PtrSize m_bias = 0;
	PtrSize m_initialSize = 0;
	U32 m_alignment = 0;
};
/// @}

} // end namespace anki

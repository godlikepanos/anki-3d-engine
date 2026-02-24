// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/Common.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>
#include <AnKi/Util/BlockArray.h>

namespace anki {

class SegregatedListsGpuMemoryPoolAllocation
{
	friend class SegregatedListsGpuMemoryPool;

public:
	SegregatedListsGpuMemoryPoolAllocation() = default;

	SegregatedListsGpuMemoryPoolAllocation(const SegregatedListsGpuMemoryPoolAllocation&) = delete;

	SegregatedListsGpuMemoryPoolAllocation(SegregatedListsGpuMemoryPoolAllocation&& b)
	{
		*this = std::move(b);
	}

	~SegregatedListsGpuMemoryPoolAllocation()
	{
		free();
	}

	SegregatedListsGpuMemoryPoolAllocation& operator=(const SegregatedListsGpuMemoryPoolAllocation&) = delete;

	SegregatedListsGpuMemoryPoolAllocation& operator=(SegregatedListsGpuMemoryPoolAllocation&& b)
	{
		ANKI_ASSERT(!(*this) && "Need to manually free this");
		m_chunk = b.m_chunk;
		m_offset = b.m_offset;
		m_size = b.m_size;
		b.reset();
		return *this;
	}

	explicit operator Bool() const
	{
		return m_size > 0;
	}

	operator BufferView() const;

	void* getMappedMemory() const;

	void free();

private:
	PtrSize m_offset = kMaxPtrSize;
	PtrSize m_size = 0;

	void* m_chunk = nullptr; // Type is SegregatedListsGpuMemoryPool::SLChunk

	void reset()
	{
		m_chunk = nullptr;
		m_offset = kMaxPtrSize;
		m_size = 0;
	}
};

// It allocates Buffers and sub-allocates out of them.
class SegregatedListsGpuMemoryPool
{
	friend class SegregatedListsGpuMemoryPoolAllocation;

public:
	SegregatedListsGpuMemoryPool();

	~SegregatedListsGpuMemoryPool();

	// bufferSize: The size of the buffer each chunk will allocate
	// maxBuffer: How many buffers of bufferSize to allocate
	void init(PtrSize bufferSize, U32 maxBuffers, CString bufferName, ConstWeakArray<PtrSize> classSizes, BufferUsageBit bufferUsage,
			  BufferMapAccessBit mapAccess = BufferMapAccessBit::kNone);

	// Generic allocation
	// It's thread-safe
	SegregatedListsGpuMemoryPoolAllocation allocate(PtrSize size, U32 alignment);

	// It's thread-safe
	template<typename T>
	SegregatedListsGpuMemoryPoolAllocation allocateStructuredBuffer(U32 count)
	{
		const U32 alignment = (m_structuredBufferBindOffsetAlignment == kMaxU16) ? sizeof(T) : m_structuredBufferBindOffsetAlignment;
		return allocate(count * sizeof(T), alignment);
	}

	// Allocate a vertex buffer.
	// It's thread-safe
	SegregatedListsGpuMemoryPoolAllocation allocateFormat(Format format, U32 count)
	{
		const U32 texelSize = getFormatInfo(format).m_texelSize;
		return allocate(texelSize * count, texelSize);
	}

	// It's thread-safe
	void deferredFree(SegregatedListsGpuMemoryPoolAllocation& alloc);

	// Not thread-safe
	void endFrame(Fence* fence);

	// Not thread-safe
	void getStats(PtrSize& allocatedSize, PtrSize& memoryCapacity) const;

private:
	class SLChunk;
	class SLInterface;
	using SLBuilder = SegregatedListsAllocatorBuilder<SLChunk, SLInterface, DummyMutex, SingletonMemoryPoolWrapper<DefaultMemoryPool>>;

	class Garbage
	{
	public:
		BlockArray<SegregatedListsGpuMemoryPoolAllocation, BlockArrayConfig<16>> m_allocations;
		FencePtr m_fence;
	};

	SLBuilder* m_builder = nullptr;
	Mutex m_mtx;

	String m_bufferName;
	PtrSize m_bufferSize = 0;
	U32 m_maxBufferCount = 0;

	DynamicArray<PtrSize> m_classSizes;

	PtrSize m_allocatedSize = 0;

	BlockArray<Garbage, BlockArrayConfig<8>> m_garbage;
	U32 m_activeGarbage = kMaxU32;

	BufferUsageBit m_bufferUsage = BufferUsageBit::kNone;

	U16 m_chunksCreated = 0;

	U16 m_structuredBufferBindOffsetAlignment = kMaxU16;

	BufferMapAccessBit m_mapAccess = BufferMapAccessBit::kNone;

	Error allocateChunk(SLChunk*& newChunk, PtrSize& chunkSize);
	void deleteChunk(SLChunk* chunk);

	void throwGarbage(Bool all);
};

} // end namespace anki

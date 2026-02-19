// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>
#include <AnKi/Util/BlockArray.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Fence.h>

namespace anki {

// Forward
class SegregatedListsSingleBufferGpuMemoryPool;

// The result of an allocation of SegregatedListsSingleBufferGpuMemoryPool.
class SegregatedListsSingleBufferGpuMemoryPoolAllocation
{
	friend class SegregatedListsSingleBufferGpuMemoryPool;

public:
	SegregatedListsSingleBufferGpuMemoryPoolAllocation() = default;

	SegregatedListsSingleBufferGpuMemoryPoolAllocation(const SegregatedListsSingleBufferGpuMemoryPoolAllocation&) = delete;

	SegregatedListsSingleBufferGpuMemoryPoolAllocation(SegregatedListsSingleBufferGpuMemoryPoolAllocation&& b)
	{
		*this = std::move(b);
	}

	~SegregatedListsSingleBufferGpuMemoryPoolAllocation();

	SegregatedListsSingleBufferGpuMemoryPoolAllocation& operator=(const SegregatedListsSingleBufferGpuMemoryPoolAllocation&) = delete;

	Bool operator==(const SegregatedListsSingleBufferGpuMemoryPoolAllocation& b) const
	{
		return m_offset == b.m_offset && m_parent == b.m_parent && m_size == b.m_size;
	}

	SegregatedListsSingleBufferGpuMemoryPoolAllocation& operator=(SegregatedListsSingleBufferGpuMemoryPoolAllocation&& b)
	{
		ANKI_ASSERT(!(*this) && "Need to deallocate first");
		m_parent = b.m_parent;
		m_offset = b.m_offset;
		m_size = b.m_size;
		b.reset();
		return *this;
	}

	explicit operator Bool() const
	{
		return m_size != 0;
	}

	operator BufferView() const;

	PtrSize getOffset() const
	{
		ANKI_ASSERT(!!(*this));
		return m_offset;
	}

	PtrSize getSize() const
	{
		ANKI_ASSERT(!!(*this));
		return m_size;
	}

	void* getMappedMemory() const;

private:
	SegregatedListsSingleBufferGpuMemoryPool* m_parent = nullptr;
	PtrSize m_offset = kMaxPtrSize;
	PtrSize m_size = 0;

	void reset()
	{
		m_parent = nullptr;
		m_offset = kMaxPtrSize;
		m_size = 0;
	}
};

// GPU memory allocator based on segregated lists. It sub-allocates from a single largs GPU buffer.
class SegregatedListsSingleBufferGpuMemoryPool
{
	friend class SegregatedListsSingleBufferGpuMemoryPoolAllocation;

public:
	SegregatedListsSingleBufferGpuMemoryPool() = default;

	~SegregatedListsSingleBufferGpuMemoryPool()
	{
		destroy();
	}

	SegregatedListsSingleBufferGpuMemoryPool(const SegregatedListsSingleBufferGpuMemoryPool&) = delete;

	SegregatedListsSingleBufferGpuMemoryPool& operator=(const SegregatedListsSingleBufferGpuMemoryPool&) = delete;

	void init(BufferUsageBit gpuBufferUsage, ConstWeakArray<PtrSize> classUpperSizes, PtrSize gpuBuffersize, CString bufferName,
			  BufferMapAccessBit map = BufferMapAccessBit::kNone);

	void destroy();

	// Allocate memory.
	// It's thread-safe.
	SegregatedListsSingleBufferGpuMemoryPoolAllocation allocate(PtrSize size, U32 alignment);

	// Free memory a few frames down the line.
	// It's thread-safe.
	void deferredFree(SegregatedListsSingleBufferGpuMemoryPoolAllocation& token);

	// It's thread-safe.
	void endFrame(Fence* fence);

	// Need to be checking this constantly to get the updated buffer in case of CoWs.
	// It's thread-safe.
	Buffer& getGpuBuffer() const
	{
		ANKI_ASSERT(m_gpuBuffer.isCreated() && "The buffer hasn't been created yet");
		return *m_gpuBuffer;
	}

	// It's thread-safe.
	void* getGpuBufferMappedMemory() const
	{
		ANKI_ASSERT(m_mappedGpuBufferMemory);
		return m_mappedGpuBufferMemory;
	}

	// It's thread-safe.
	void getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const;

private:
	class BuilderInterface;
	class Chunk;
	using Builder = SegregatedListsAllocatorBuilder<Chunk, BuilderInterface, DummyMutex, SingletonMemoryPoolWrapper<GrMemoryPool>>;

	GrDynamicArray<PtrSize> m_classes;
	mutable Mutex m_lock;

	Builder* m_builder = nullptr;
	BufferPtr m_gpuBuffer;
	void* m_mappedGpuBufferMemory = nullptr;
	PtrSize m_allocatedSize = 0;
	Chunk* m_chunk = nullptr; // The one and only chunk

	class Garbage
	{
	public:
		GrBlockArray<SegregatedListsSingleBufferGpuMemoryPoolAllocation, BlockArrayConfig<16>> m_tokens;
		FencePtr m_fence;
	};

	GrBlockArray<Garbage, BlockArrayConfig<8>> m_garbage;
	U32 m_activeGarbage = kMaxU32;

	Error allocateChunk(Chunk*& newChunk, PtrSize& chunkSize);
	void deleteChunk(Chunk* chunk);

	Bool isInitialized() const
	{
		return !!m_gpuBuffer;
	}
};

} // end namespace anki

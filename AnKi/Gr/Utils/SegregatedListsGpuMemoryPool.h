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

// The result of an allocation of SegregatedListsGpuMemoryPool.
class SegregatedListsGpuMemoryPoolToken
{
	friend class SegregatedListsGpuMemoryPool;

public:
	// The offset in the SegregatedListsGpuMemoryPoolToken::getBuffer() buffer.
	PtrSize m_offset = kMaxPtrSize;

	PtrSize m_size = kMaxPtrSize;

	Bool operator==(const SegregatedListsGpuMemoryPoolToken& b) const
	{
		return m_offset == b.m_offset && m_chunk == b.m_chunk && m_chunkOffset == b.m_chunkOffset && m_size == b.m_size;
	}

	Bool isValid() const
	{
		return m_chunk != nullptr;
	}

private:
	void* m_chunk = nullptr;
	PtrSize m_chunkOffset = kMaxPtrSize;
};

// GPU memory allocator based on segregated lists. It allocates a GPU buffer with some initial size. If there is a need to grow it allocates a bigger
// buffer and copies contents of the old one to the new (CoW).
class SegregatedListsGpuMemoryPool
{
public:
	SegregatedListsGpuMemoryPool() = default;

	~SegregatedListsGpuMemoryPool()
	{
		destroy();
	}

	SegregatedListsGpuMemoryPool(const SegregatedListsGpuMemoryPool&) = delete;

	SegregatedListsGpuMemoryPool& operator=(const SegregatedListsGpuMemoryPool&) = delete;

	void init(BufferUsageBit gpuBufferUsage, ConstWeakArray<PtrSize> classUpperSizes, PtrSize initialGpuBufferSize, CString bufferName,
			  Bool allowCoWs, BufferMapAccessBit map = BufferMapAccessBit::kNone);

	void destroy();

	// Allocate memory.
	// It's thread-safe.
	void allocate(PtrSize size, U32 alignment, SegregatedListsGpuMemoryPoolToken& token);

	// Free memory a few frames down the line.
	// It's thread-safe.
	void deferredFree(SegregatedListsGpuMemoryPoolToken& token);

	// It's thread-safe.
	void endFrame(Fence* fence);

	// Need to be checking this constantly to get the updated buffer in case of CoWs.
	// It's not thread-safe.
	Buffer& getGpuBuffer() const
	{
		ANKI_ASSERT(m_gpuBuffer.isCreated() && "The buffer hasn't been created yet");
		return *m_gpuBuffer;
	}

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

	BufferUsageBit m_bufferUsage = BufferUsageBit::kNone;
	GrDynamicArray<PtrSize> m_classes;
	PtrSize m_initialBufferSize = 0;
	GrString m_bufferName;

	mutable Mutex m_lock;

	Builder* m_builder = nullptr;
	BufferPtr m_gpuBuffer;
	void* m_mappedGpuBufferMemory = nullptr;
	PtrSize m_allocatedSize = 0;

	GrDynamicArray<Chunk*> m_deletedChunks;

	class Garbage
	{
	public:
		GrBlockArray<SegregatedListsGpuMemoryPoolToken, BlockArrayConfig<16>> m_tokens;
		FencePtr m_fence;
	};

	GrBlockArray<Garbage, BlockArrayConfig<8>> m_garbage;
	U32 m_activeGarbage = kMaxU32;
	Bool m_allowCoWs = true;

	BufferMapAccessBit m_mapAccess = BufferMapAccessBit::kNone;

	Error allocateChunk(Chunk*& newChunk, PtrSize& chunkSize);
	void deleteChunk(Chunk* chunk);

	Bool isInitialized() const
	{
		return m_bufferUsage != BufferUsageBit::kNone;
	}
};

} // end namespace anki

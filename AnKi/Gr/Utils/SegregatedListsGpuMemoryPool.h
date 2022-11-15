// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// The result of an allocation of SegregatedListsGpuMemoryPool.
/// @memberof SegregatedListsGpuMemoryPool
class SegregatedListsGpuMemoryPoolToken
{
	friend class SegregatedListsGpuMemoryPool;

public:
	/// The offset in the SegregatedListsGpuMemoryPoolToken::getBuffer() buffer.
	PtrSize m_offset = kMaxPtrSize;

	Bool operator==(const SegregatedListsGpuMemoryPoolToken& b) const
	{
		return m_offset == b.m_offset && m_chunk == b.m_chunk && m_chunkOffset == b.m_chunkOffset && m_size == b.m_size;
	}

private:
	void* m_chunk = nullptr;
	PtrSize m_chunkOffset = kMaxPtrSize;
	PtrSize m_size = kMaxPtrSize;
};

/// GPU memory allocator based on segregated lists. It allocates a GPU buffer with some initial size. If there is a need
/// to grow it allocates a bigger buffer and copies contents of the old one to the new (CoW).
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

	void init(GrManager* gr, BaseMemoryPool* pool, BufferUsageBit gpuBufferUsage,
			  ConstWeakArray<PtrSize> classUpperSizes, PtrSize initialGpuBufferSize, CString bufferName,
			  Bool allowCoWs);

	void destroy();

	/// Allocate memory.
	/// @note It's thread-safe.
	void allocate(PtrSize size, U32 alignment, SegregatedListsGpuMemoryPoolToken& token);

	/// Free memory.
	/// @note It's thread-safe.
	void free(SegregatedListsGpuMemoryPoolToken& token);

	/// @note It's thread-safe.
	void endFrame();

	/// Need to be checking this constantly to get the updated buffer in case of CoWs.
	/// @note It's not thread-safe.
	const BufferPtr& getGpuBuffer() const
	{
		ANKI_ASSERT(m_gpuBuffer.isCreated() && "The buffer hasn't been created yet");
		return m_gpuBuffer;
	}

	/// @note It's thread-safe.
	void getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const;

private:
	class BuilderInterface;
	class Chunk;
	using Builder = SegregatedListsAllocatorBuilder<Chunk, BuilderInterface, DummyMutex>;

	GrManager* m_gr = nullptr;
	mutable BaseMemoryPool* m_pool = nullptr;
	BufferUsageBit m_bufferUsage = BufferUsageBit::kNone;
	DynamicArray<PtrSize> m_classes;
	PtrSize m_initialBufferSize = 0;
	String m_bufferName;

	mutable Mutex m_lock;

	Builder* m_builder = nullptr;
	BufferPtr m_gpuBuffer;
	PtrSize m_allocatedSize = 0;

	DynamicArray<Chunk*> m_deletedChunks;

	Array<DynamicArray<SegregatedListsGpuMemoryPoolToken>, kMaxFramesInFlight> m_garbage;
	U8 m_frame = 0;
	Bool m_allowCoWs = true;

	Error allocateChunk(Chunk*& newChunk, PtrSize& chunkSize);
	void deleteChunk(Chunk* chunk);

	Bool isInitialized() const
	{
		return m_gr != nullptr;
	}
};
/// @}

} // end namespace anki

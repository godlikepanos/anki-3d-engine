// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Utils/StackGpuMemoryPool.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

class StackGpuMemoryPool::Chunk
{
public:
	// Builder interface stuff:
	Chunk* m_nextChunk = nullptr;
	Atomic<PtrSize> m_offsetInChunk = {0};
	PtrSize m_chunkSize = 0;

	// Other stuff:
	BufferPtr m_buffer;
	U8* m_mappedMemory = nullptr;
};

class StackGpuMemoryPool::BuilderInterface
{
public:
	PtrSize m_initialSize = 0;
	F64 m_scale = 0.0;
	PtrSize m_bias = 0;
	GrString m_bufferName;
	U32 m_alignment = 0;
	BufferUsageBit m_bufferUsage = BufferUsageBit::kNone;
	BufferMapAccessBit m_bufferMap = BufferMapAccessBit::kNone;
	U8 m_chunkCount = 0;
	Bool m_allowToGrow = false;

	// Builder interface stuff:
	U32 getMaxAlignment() const
	{
		return m_alignment;
	}

	PtrSize getInitialChunkSize() const
	{
		return m_initialSize;
	}

	F64 getNextChunkGrowScale()
	{
		return m_scale;
	}

	PtrSize getNextChunkGrowBias() const
	{
		return m_bias;
	}

	static constexpr Bool ignoreDeallocationErrors()
	{
		return true;
	}

	Error allocateChunk(PtrSize size, Chunk*& out)
	{
		if(!m_allowToGrow && m_chunkCount > 0)
		{
			ANKI_GR_LOGE("Memory pool is not allowed to grow");
			return Error::kOutOfMemory;
		}

		Chunk* chunk = newInstance<Chunk>(GrMemoryPool::getSingleton());

		BufferInitInfo buffInit(m_bufferName);
		buffInit.m_size = size;
		buffInit.m_usage = m_bufferUsage;
		buffInit.m_mapAccess = m_bufferMap;
		chunk->m_buffer = GrManager::getSingleton().newBuffer(buffInit);

		if(!!m_bufferMap)
		{
			chunk->m_mappedMemory = static_cast<U8*>(chunk->m_buffer->map(0, size, m_bufferMap));
		}

		out = chunk;
		++m_chunkCount;

		return Error::kNone;
	}

	void freeChunk(Chunk* chunk)
	{
		if(chunk->m_mappedMemory)
		{
			chunk->m_buffer->unmap();
		}

		deleteInstance(GrMemoryPool::getSingleton(), chunk);
	}

	void recycleChunk([[maybe_unused]] Chunk& out)
	{
		// Do nothing
	}

	Atomic<U32>* getAllocationCount()
	{
		return nullptr;
	};
};

StackGpuMemoryPool::~StackGpuMemoryPool()
{
	if(m_builder)
	{
		deleteInstance(GrMemoryPool::getSingleton(), m_builder);
	}
}

void StackGpuMemoryPool::init(PtrSize initialSize, F64 nextChunkGrowScale, PtrSize nextChunkGrowBias, U32 alignment,
							  BufferUsageBit bufferUsage, BufferMapAccessBit bufferMapping, Bool allowToGrow,
							  CString bufferName)
{
	ANKI_ASSERT(m_builder == nullptr);
	ANKI_ASSERT(initialSize > 0 && alignment > 0);
	ANKI_ASSERT(nextChunkGrowScale >= 1.0 && nextChunkGrowBias > 0);

	m_builder = newInstance<Builder>(GrMemoryPool::getSingleton());
	BuilderInterface& inter = m_builder->getInterface();
	inter.m_initialSize = initialSize;
	inter.m_scale = nextChunkGrowScale;
	inter.m_bias = nextChunkGrowBias;
	inter.m_bufferName.create(bufferName);
	inter.m_alignment = alignment;
	inter.m_bufferUsage = bufferUsage;
	inter.m_bufferMap = bufferMapping;
	inter.m_allowToGrow = allowToGrow;
}

void StackGpuMemoryPool::reset()
{
	m_builder->reset();
}

void StackGpuMemoryPool::allocate(PtrSize size, PtrSize& outOffset, Buffer*& buffer, void*& mappedMemory)
{
	Chunk* chunk;
	PtrSize offset;
	const Error err = m_builder->allocate(size, 1, chunk, offset);
	if(err)
	{
		ANKI_GR_LOGF("Allocation failed");
	}

	outOffset = offset;
	buffer = chunk->m_buffer.get();
	if(chunk->m_mappedMemory)
	{
		mappedMemory = chunk->m_mappedMemory + offset;
	}
}

} // end namespace anki

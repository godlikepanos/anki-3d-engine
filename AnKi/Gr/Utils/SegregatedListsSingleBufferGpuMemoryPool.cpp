// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Utils/SegregatedListsSingleBufferGpuMemoryPool.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/CommandBuffer.h>

namespace anki {

class SegregatedListsSingleBufferGpuMemoryPool::Chunk : public SegregatedListsAllocatorBuilderChunkBase<SingletonMemoryPoolWrapper<GrMemoryPool>>
{
};

class SegregatedListsSingleBufferGpuMemoryPool::BuilderInterface
{
public:
	SegregatedListsSingleBufferGpuMemoryPool* m_parent = nullptr;

	// Interface methods
	U32 getClassCount() const
	{
		return m_parent->m_classes.getSize();
	}

	void getClassInfo(U32 idx, PtrSize& size) const
	{
		size = m_parent->m_classes[idx];
	}

	Error allocateChunk(Chunk*& newChunk, PtrSize& chunkSize)
	{
		return m_parent->allocateChunk(newChunk, chunkSize);
	}

	void deleteChunk(Chunk* chunk)
	{
		m_parent->deleteChunk(chunk);
	}

	static constexpr PtrSize getMinSizeAlignment()
	{
		return 4;
	}
};

void SegregatedListsSingleBufferGpuMemoryPool::init(BufferUsageBit gpuBufferUsage, ConstWeakArray<PtrSize> classUpperSizes, PtrSize bufferSize,
													CString bufferName, BufferMapAccessBit map)
{
	ANKI_ASSERT(!isInitialized());

	BufferInitInfo buffInit(bufferName);
	buffInit.m_size = bufferSize;
	buffInit.m_usage = gpuBufferUsage;
	buffInit.m_mapAccess = map;
	m_gpuBuffer = GrManager::getSingleton().newBuffer(buffInit);

	if(map != BufferMapAccessBit::kNone)
	{
		m_mappedGpuBufferMemory = m_gpuBuffer->map(0, kMaxPtrSize, map);
	}

	ANKI_ASSERT(classUpperSizes.getSize() > 0);
	m_classes.resize(classUpperSizes.getSize());
	for(U32 i = 0; i < m_classes.getSize(); ++i)
	{
		m_classes[i] = classUpperSizes[i];
	}

	m_builder = newInstance<Builder>(GrMemoryPool::getSingleton());
	m_builder->getInterface().m_parent = this;

	m_allocatedSize = 0;
}

void SegregatedListsSingleBufferGpuMemoryPool::destroy()
{
	if(!isInitialized())
	{
		return;
	}

	GrManager::getSingleton().finish();

	if(m_mappedGpuBufferMemory)
	{
		m_gpuBuffer->unmap();
	}

	for(auto it = m_garbage.getBegin(); it != m_garbage.getEnd(); ++it)
	{
		Garbage& garbage = *it;
		if(it.getArrayIndex() != m_activeGarbage)
		{
			ANKI_CHECKF(garbage.m_fence->clientWait(kMaxSecond));
		}
		else
		{
			ANKI_ASSERT(!garbage.m_fence);
		}

		for(SegregatedListsSingleBufferGpuMemoryPoolAllocation& token : garbage.m_tokens)
		{
			m_builder->free(m_chunk, token.m_offset, token.m_size);
			token.reset();
		}
	}

	deleteInstance(GrMemoryPool::getSingleton(), m_builder);
	m_gpuBuffer.reset(nullptr);

	ANKI_ASSERT(m_chunk == nullptr);
}

Error SegregatedListsSingleBufferGpuMemoryPool::allocateChunk(Chunk*& newChunk, PtrSize& chunkSize)
{
	ANKI_ASSERT(isInitialized());

	if(m_chunk == nullptr)
	{
		m_chunk = newInstance<Chunk>(GrMemoryPool::getSingleton());
		newChunk = m_chunk;
		chunkSize = m_gpuBuffer->getSize();
	}
	else
	{
		ANKI_GR_LOGE("Out of memory");
		return Error::kOutOfMemory;
	}

	return Error::kNone;
}

void SegregatedListsSingleBufferGpuMemoryPool::deleteChunk(Chunk* chunk)
{
	ANKI_ASSERT(m_chunk == chunk);
	deleteInstance(GrMemoryPool::getSingleton(), chunk);
	m_chunk = nullptr;
}

SegregatedListsSingleBufferGpuMemoryPoolAllocation SegregatedListsSingleBufferGpuMemoryPool::allocate(PtrSize size, U32 alignment)
{
	ANKI_ASSERT(isInitialized());
	ANKI_ASSERT(size > 0 && alignment > 0);

	LockGuard lock(m_lock);

	Chunk* chunk;
	PtrSize offset;
	const Error err = m_builder->allocate(size, alignment, chunk, offset);
	if(err)
	{
		ANKI_GR_LOGF("Failed to allocate memory");
	}

	SegregatedListsSingleBufferGpuMemoryPoolAllocation alloc;
	alloc.m_parent = this;
	alloc.m_offset = offset;
	alloc.m_size = size;

	m_allocatedSize += size;

	return alloc;
}

void SegregatedListsSingleBufferGpuMemoryPool::deferredFree(SegregatedListsSingleBufferGpuMemoryPoolAllocation& token)
{
	ANKI_ASSERT(isInitialized());

	if(token)
	{
		LockGuard lock(m_lock);

		if(m_activeGarbage == kMaxU32)
		{
			m_activeGarbage = m_garbage.emplace().getArrayIndex();
		}

		m_garbage[m_activeGarbage].m_tokens.emplace(std::move(token));
	}
}

void SegregatedListsSingleBufferGpuMemoryPool::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);
	ANKI_ASSERT(isInitialized());

	LockGuard lock(m_lock);

	// Throw out the garbage
	Array<U32, 8> garbageToDelete;
	U32 garbageToDeleteCount = 0;
	for(auto it = m_garbage.getBegin(); it != m_garbage.getEnd(); ++it)
	{
		Garbage& garbage = *it;

		if(garbage.m_fence && garbage.m_fence->signaled())
		{
			for(SegregatedListsSingleBufferGpuMemoryPoolAllocation& token : garbage.m_tokens)
			{
				m_builder->free(m_chunk, token.m_offset, token.m_size);

				ANKI_ASSERT(m_allocatedSize >= token.m_size);
				m_allocatedSize -= token.m_size;

				token.reset();
			}

			garbageToDelete[garbageToDeleteCount++] = it.getArrayIndex();
		}
	}

	for(U32 i = 0; i < garbageToDeleteCount; ++i)
	{
		m_garbage.erase(garbageToDelete[i]);
	}

	// Set the new fence
	if(m_activeGarbage != kMaxU32)
	{
		ANKI_ASSERT(m_garbage[m_activeGarbage].m_tokens.getSize());
		ANKI_ASSERT(!m_garbage[m_activeGarbage].m_fence);
		m_garbage[m_activeGarbage].m_fence.reset(fence);

		m_activeGarbage = kMaxU32;
	}
}

void SegregatedListsSingleBufferGpuMemoryPool::getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const
{
	ANKI_ASSERT(isInitialized());

	LockGuard lock(m_lock);

	externalFragmentation = m_builder->computeExternalFragmentation();
	userAllocatedSize = m_allocatedSize;
	totalSize = (m_gpuBuffer) ? m_gpuBuffer->getSize() : 0;
}

SegregatedListsSingleBufferGpuMemoryPoolAllocation::operator BufferView() const
{
	ANKI_ASSERT(!!(*this));

	return BufferView(m_parent->m_gpuBuffer.get(), m_offset, m_size);
}

SegregatedListsSingleBufferGpuMemoryPoolAllocation::~SegregatedListsSingleBufferGpuMemoryPoolAllocation()
{
	if(*this)
	{
		m_parent->deferredFree(*this);
	}
}

void* SegregatedListsSingleBufferGpuMemoryPoolAllocation::getMappedMemory() const
{
	ANKI_ASSERT(!!(*this));
	ANKI_ASSERT(m_parent->m_mappedGpuBufferMemory != nullptr);

	U8* mem = static_cast<U8*>(m_parent->m_mappedGpuBufferMemory);
	mem += m_offset;

	return mem;
}

} // end namespace anki

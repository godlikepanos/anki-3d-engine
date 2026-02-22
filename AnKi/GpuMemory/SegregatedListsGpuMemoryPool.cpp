// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/SegregatedListsGpuMemoryPool.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

class SegregatedListsGpuMemoryPool::SLChunk : public SegregatedListsAllocatorBuilderChunkBase<SingletonMemoryPoolWrapper<DefaultMemoryPool>>
{
public:
	BufferPtr m_buffer;
	void* m_mappedMemory = nullptr;
	SegregatedListsGpuMemoryPool* m_parent = nullptr;
};

class SegregatedListsGpuMemoryPool::SLInterface
{
public:
	SegregatedListsGpuMemoryPool* m_parent = nullptr;

	// Interface methods
	U32 getClassCount() const
	{
		return m_parent->m_classSizes.getSize();
	}

	void getClassInfo(U32 idx, PtrSize& size) const
	{
		size = m_parent->m_classSizes[idx];
	}

	Error allocateChunk(SLChunk*& newChunk, PtrSize& chunkSize)
	{
		return m_parent->allocateChunk(newChunk, chunkSize);
	}

	void deleteChunk(SLChunk* chunk)
	{
		m_parent->deleteChunk(chunk);
	}

	static constexpr PtrSize getMinSizeAlignment()
	{
		return 4;
	}
};

SegregatedListsGpuMemoryPool::SegregatedListsGpuMemoryPool()
{
}

SegregatedListsGpuMemoryPool::~SegregatedListsGpuMemoryPool()
{
	GrManager::getSingleton().finish();

	throwGarbage(true);

	deleteInstance(DefaultMemoryPool::getSingleton(), m_builder);
}

void SegregatedListsGpuMemoryPool::init(PtrSize bufferSize, U32 maxBuffers, CString bufferName, ConstWeakArray<PtrSize> classSizes,
										BufferUsageBit bufferUsage, BufferMapAccessBit mapAccess)
{
	m_builder = newInstance<SLBuilder>(DefaultMemoryPool::getSingleton());
	m_builder->getInterface().m_parent = this;

	if(!GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
	{
		m_structuredBufferBindOffsetAlignment = U16(GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment);
	}

	ANKI_ASSERT(bufferSize > 0);
	m_bufferSize = bufferSize;

	ANKI_ASSERT(maxBuffers > 0);
	m_maxBufferCount = maxBuffers;

	ANKI_ASSERT(bufferName);
	m_bufferName = bufferName;

	ANKI_ASSERT(classSizes.getSize() > 0 && classSizes.getBack() == bufferSize);
	m_classSizes.resize(classSizes.getSize());
	for(U32 i = 0; i < classSizes.getSize(); ++i)
	{
		m_classSizes[i] = classSizes[i];
	}

	m_bufferUsage = bufferUsage;
	m_mapAccess = mapAccess;
}

Error SegregatedListsGpuMemoryPool::allocateChunk(SLChunk*& newChunk, PtrSize& chunkSize)
{
	if(m_chunksCreated == m_maxBufferCount)
	{
		ANKI_GPUMEM_LOGE("Reached the max limit of memory chunks. Incrase %u", m_maxBufferCount);
		return Error::kOutOfMemory;
	}

	BufferInitInfo buffInit("TexPoolChunk");
	buffInit.m_size = m_bufferSize;
	buffInit.m_usage = m_bufferUsage;
	buffInit.m_mapAccess = m_mapAccess;

	BufferPtr buff = GrManager::getSingleton().newBuffer(buffInit);

	newChunk = newInstance<SLChunk>(DefaultMemoryPool::getSingleton());
	newChunk->m_buffer = buff;
	newChunk->m_parent = this;

	if(!!m_mapAccess)
	{
		newChunk->m_mappedMemory = newChunk->m_buffer->map(0, kMaxPtrSize, m_mapAccess);
	}

	chunkSize = m_bufferSize;

	++m_chunksCreated;

	return Error::kNone;
}

void SegregatedListsGpuMemoryPool::deleteChunk(SLChunk* chunk)
{
	if(chunk)
	{
		ANKI_GPUMEM_LOGW("Will delete the mem pool chunk and will serialize CPU and GPU");
		GrManager::getSingleton().finish();

		if(chunk->m_mappedMemory)
		{
			chunk->m_buffer->unmap();
		}

		deleteInstance(DefaultMemoryPool::getSingleton(), chunk);

		// Wait again to force delete the memory
		GrManager::getSingleton().finish();

		ANKI_ASSERT(m_chunksCreated > 0);
		--m_chunksCreated;
	}
}

SegregatedListsGpuMemoryPoolAllocation SegregatedListsGpuMemoryPool::allocate(PtrSize size, U32 alignment)
{
	LockGuard lock(m_mtx);

	SLChunk* chunk;
	PtrSize offset;
	if(m_builder->allocate(size, alignment, chunk, offset))
	{
		ANKI_GPUMEM_LOGF("Failed to allocate tex memory");
	}

	SegregatedListsGpuMemoryPoolAllocation alloc;
	alloc.m_chunk = chunk;
	alloc.m_offset = offset;
	alloc.m_size = size;

	m_allocatedSize += size;

	return alloc;
}

void SegregatedListsGpuMemoryPool::deferredFree(SegregatedListsGpuMemoryPoolAllocation& alloc)
{
	if(!alloc)
	{
		return;
	}

	LockGuard lock(m_mtx);

	if(m_activeGarbage == kMaxU32)
	{
		m_activeGarbage = m_garbage.emplace().getArrayIndex();
	}

	m_garbage[m_activeGarbage].m_allocations.emplace(std::move(alloc));
	ANKI_ASSERT(!alloc);
}

void SegregatedListsGpuMemoryPool::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);

	LockGuard lock(m_mtx);

	throwGarbage(false);

	// Set the new fence
	if(m_activeGarbage != kMaxU32)
	{
		ANKI_ASSERT(m_garbage[m_activeGarbage].m_allocations.getSize());
		ANKI_ASSERT(!m_garbage[m_activeGarbage].m_fence);
		m_garbage[m_activeGarbage].m_fence.reset(fence);

		m_activeGarbage = kMaxU32;
	}
}

void SegregatedListsGpuMemoryPool::throwGarbage(Bool all)
{
	Array<U32, 8> garbageToDelete;
	U32 garbageToDeleteCount = 0;
	for(auto it = m_garbage.getBegin(); it != m_garbage.getEnd(); ++it)
	{
		Garbage& garbage = *it;

		if(all || (garbage.m_fence && garbage.m_fence->signaled()))
		{
			for(SegregatedListsGpuMemoryPoolAllocation& alloc : garbage.m_allocations)
			{
				m_builder->free(static_cast<SLChunk*>(alloc.m_chunk), alloc.m_offset, alloc.m_size);

				ANKI_ASSERT(m_allocatedSize >= alloc.m_size);
				m_allocatedSize -= alloc.m_size;

				alloc.reset();
			}

			garbageToDelete[garbageToDeleteCount++] = it.getArrayIndex();
		}
	}

	for(U32 i = 0; i < garbageToDeleteCount; ++i)
	{
		m_garbage.erase(garbageToDelete[i]);
	}
}

void SegregatedListsGpuMemoryPool::getStats(PtrSize& allocatedSize, PtrSize& memoryCapacity) const
{
	allocatedSize = m_allocatedSize;
	memoryCapacity = PtrSize(m_chunksCreated) * m_bufferSize;
}

SegregatedListsGpuMemoryPoolAllocation::~SegregatedListsGpuMemoryPoolAllocation()
{
	if(*this)
	{
		SegregatedListsGpuMemoryPool::SLChunk& chunk = *static_cast<SegregatedListsGpuMemoryPool::SLChunk*>(m_chunk);
		chunk.m_parent->deferredFree(*this);
	}
}

SegregatedListsGpuMemoryPoolAllocation::operator BufferView() const
{
	ANKI_ASSERT(!!(*this));
	SegregatedListsGpuMemoryPool::SLChunk& chunk = *static_cast<SegregatedListsGpuMemoryPool::SLChunk*>(m_chunk);
	return BufferView(chunk.m_buffer.get(), m_offset, m_size);
}

void* SegregatedListsGpuMemoryPoolAllocation::getMappedMemory() const
{
	ANKI_ASSERT(!!(*this));
	SegregatedListsGpuMemoryPool::SLChunk& chunk = *static_cast<SegregatedListsGpuMemoryPool::SLChunk*>(m_chunk);
	ANKI_ASSERT(chunk.m_mappedMemory);

	return static_cast<U8*>(chunk.m_mappedMemory) + m_offset;
}

} // end namespace anki

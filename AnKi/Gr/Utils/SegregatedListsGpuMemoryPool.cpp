// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/CommandBuffer.h>

namespace anki {

class SegregatedListsGpuMemoryPool::Chunk : public SegregatedListsAllocatorBuilderChunkBase<SingletonMemoryPoolWrapper<GrMemoryPool>>
{
public:
	PtrSize m_offsetInGpuBuffer;
};

class SegregatedListsGpuMemoryPool::BuilderInterface
{
public:
	SegregatedListsGpuMemoryPool* m_parent = nullptr;

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

void SegregatedListsGpuMemoryPool::init(BufferUsageBit gpuBufferUsage, ConstWeakArray<PtrSize> classUpperSizes, PtrSize initialGpuBufferSize,
										CString bufferName, Bool allowCoWs, BufferMapAccessBit map)
{
	ANKI_ASSERT(!isInitialized());

	ANKI_ASSERT(gpuBufferUsage != BufferUsageBit::kNone);
	m_bufferUsage = gpuBufferUsage;

	ANKI_ASSERT(classUpperSizes.getSize() > 0);
	m_classes.resize(classUpperSizes.getSize());
	for(U32 i = 0; i < m_classes.getSize(); ++i)
	{
		m_classes[i] = classUpperSizes[i];
	}

	ANKI_ASSERT(initialGpuBufferSize > 0);
	m_initialBufferSize = initialGpuBufferSize;

	m_bufferName = bufferName;

	m_builder = newInstance<Builder>(GrMemoryPool::getSingleton());
	m_builder->getInterface().m_parent = this;

	m_allocatedSize = 0;
	m_allowCoWs = allowCoWs;
	m_mapAccess = map;
}

void SegregatedListsGpuMemoryPool::destroy()
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

		for(const SegregatedListsGpuMemoryPoolToken& token : garbage.m_tokens)
		{
			m_builder->free(static_cast<Chunk*>(token.m_chunk), token.m_chunkOffset, token.m_size);
		}
	}

	deleteInstance(GrMemoryPool::getSingleton(), m_builder);
	m_gpuBuffer.reset(nullptr);

	for(Chunk* chunk : m_deletedChunks)
	{
		deleteInstance(GrMemoryPool::getSingleton(), chunk);
	}
}

Error SegregatedListsGpuMemoryPool::allocateChunk(Chunk*& newChunk, PtrSize& chunkSize)
{
	ANKI_ASSERT(isInitialized());

	if(!m_gpuBuffer.isCreated())
	{
		// First chunk, our job is easy, create the buffer

		BufferInitInfo buffInit(m_bufferName);
		buffInit.m_size = m_initialBufferSize;
		buffInit.m_usage = m_bufferUsage | BufferUsageBit::kAllCopy;
		buffInit.m_mapAccess = m_mapAccess;
		m_gpuBuffer = GrManager::getSingleton().newBuffer(buffInit);

		if(!!m_mapAccess)
		{
			m_mappedGpuBufferMemory = m_gpuBuffer->map(0, kMaxPtrSize, m_mapAccess);
		}

		newChunk = newInstance<Chunk>(GrMemoryPool::getSingleton());
		newChunk->m_offsetInGpuBuffer = 0;
	}
	else if(m_deletedChunks.getSize() > 0)
	{
		// We already have a deleted chunk, use that

		newChunk = m_deletedChunks.getBack();
		m_deletedChunks.popBack();
	}
	else if(m_allowCoWs)
	{
		// Current buffer is not enough. Need to grow it which we can't grow GPU buffers do a CoW

		ANKI_GR_LOGV("Will grow the %s buffer and perform a copy-on-write", m_bufferName.cstr());

		// Create the new buffer
		BufferInitInfo buffInit(m_bufferName);
		buffInit.m_size = m_gpuBuffer->getSize() * 2;
		buffInit.m_usage = m_bufferUsage | BufferUsageBit::kAllCopy;
		buffInit.m_mapAccess = m_mapAccess;
		BufferPtr newBuffer = GrManager::getSingleton().newBuffer(buffInit);

		// Do the copy
		CommandBufferInitInfo cmdbInit("SegregatedListsGpuMemoryPool CoW");
		cmdbInit.m_flags = CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

		Array<BufferBarrierInfo, 2> barriers;
		barriers[0].m_bufferView = BufferView(m_gpuBuffer.get());
		barriers[0].m_previousUsage = m_bufferUsage;
		barriers[0].m_nextUsage = BufferUsageBit::kCopySource;
		barriers[1].m_bufferView = BufferView(newBuffer.get());
		barriers[1].m_previousUsage = BufferUsageBit::kNone;
		barriers[1].m_nextUsage = BufferUsageBit::kCopyDestination;
		cmdb->setPipelineBarrier({}, barriers, {});

		cmdb->copyBufferToBuffer(BufferView(m_gpuBuffer.get()), BufferView(newBuffer.get(), 0, m_gpuBuffer->getSize()));

		barriers[1].m_previousUsage = BufferUsageBit::kCopyDestination;
		barriers[1].m_nextUsage = m_bufferUsage;
		cmdb->setPipelineBarrier({}, ConstWeakArray<BufferBarrierInfo>{&barriers[1], 1}, {});

		cmdb->endRecording();
		GrManager::getSingleton().submit(cmdb.get());

		// Create the new chunk
		newChunk = newInstance<Chunk>(GrMemoryPool::getSingleton());
		newChunk->m_offsetInGpuBuffer = m_gpuBuffer->getSize();

		// Switch the buffers
		m_gpuBuffer = newBuffer;

		if(!!m_mapAccess)
		{
			m_mappedGpuBufferMemory = m_gpuBuffer->map(0, kMaxPtrSize, m_mapAccess);
		}
	}
	else
	{
		ANKI_GR_LOGE("Out of memory and can't copy-on-write");
		return Error::kOutOfMemory;
	}

	chunkSize = m_initialBufferSize;

	return Error::kNone;
}

void SegregatedListsGpuMemoryPool::deleteChunk(Chunk* chunk)
{
	m_deletedChunks.emplaceBack(chunk);
}

void SegregatedListsGpuMemoryPool::allocate(PtrSize size, U32 alignment, SegregatedListsGpuMemoryPoolToken& token)
{
	ANKI_ASSERT(isInitialized());
	ANKI_ASSERT(size > 0 && alignment > 0);
	ANKI_ASSERT(token == SegregatedListsGpuMemoryPoolToken());

	LockGuard lock(m_lock);

	Chunk* chunk;
	PtrSize offset;
	const Error err = m_builder->allocate(size, alignment, chunk, offset);
	if(err)
	{
		ANKI_GR_LOGF("Failed to allocate memory");
	}

	token.m_chunk = chunk;
	token.m_chunkOffset = offset;
	token.m_offset = offset + chunk->m_offsetInGpuBuffer;
	token.m_size = size;

	m_allocatedSize += size;
}

void SegregatedListsGpuMemoryPool::deferredFree(SegregatedListsGpuMemoryPoolToken& token)
{
	ANKI_ASSERT(isInitialized());

	if(token.m_chunk == nullptr)
	{
		return;
	}

	{
		LockGuard lock(m_lock);

		if(m_activeGarbage == kMaxU32)
		{
			m_activeGarbage = m_garbage.emplace().getArrayIndex();
		}

		m_garbage[m_activeGarbage].m_tokens.emplace(token);
	}

	token = {};
}

void SegregatedListsGpuMemoryPool::endFrame(Fence* fence)
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
			for(SegregatedListsGpuMemoryPoolToken token : garbage.m_tokens)
			{
				m_builder->free(static_cast<Chunk*>(token.m_chunk), token.m_chunkOffset, token.m_size);

				ANKI_ASSERT(m_allocatedSize >= token.m_size);
				m_allocatedSize -= token.m_size;
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

void SegregatedListsGpuMemoryPool::getStats(F32& externalFragmentation, PtrSize& userAllocatedSize, PtrSize& totalSize) const
{
	ANKI_ASSERT(isInitialized());

	LockGuard lock(m_lock);

	externalFragmentation = m_builder->computeExternalFragmentation();
	userAllocatedSize = m_allocatedSize;
	totalSize = (m_gpuBuffer) ? m_gpuBuffer->getSize() : 0;
}

} // end namespace anki

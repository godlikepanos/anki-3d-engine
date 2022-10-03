// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/TransferGpuAllocator.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error TransferGpuAllocator::StackAllocatorBuilderInterface::allocateChunk(PtrSize size, Chunk*& out)
{
	out = newInstance<Chunk>(*m_parent->m_pool);

	BufferInitInfo bufferInit(size, BufferUsageBit::kTransferSource, BufferMapAccessBit::kWrite, "Transfer");
	out->m_buffer = m_parent->m_gr->newBuffer(bufferInit);

	out->m_mappedBuffer = out->m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite);

	return Error::kNone;
}

void TransferGpuAllocator::StackAllocatorBuilderInterface::freeChunk(Chunk* chunk)
{
	ANKI_ASSERT(chunk);

	chunk->m_buffer->unmap();

	deleteInstance(*m_parent->m_pool, chunk);
}

TransferGpuAllocator::TransferGpuAllocator()
{
}

TransferGpuAllocator::~TransferGpuAllocator()
{
	for(Pool& pool : m_pools)
	{
		ANKI_ASSERT(pool.m_pendingReleases == 0);
		pool.m_fences.destroy(*m_pool);
	}
}

Error TransferGpuAllocator::init(PtrSize maxSize, GrManager* gr, HeapMemoryPool* pool)
{
	ANKI_ASSERT(pool);
	m_pool = pool;
	m_gr = gr;

	m_maxAllocSize = getAlignedRoundUp(kChunkInitialSize * kPoolCount, maxSize);
	ANKI_RESOURCE_LOGI("Will use %zuMB of memory for transfer scratch", m_maxAllocSize / PtrSize(1_MB));

	for(Pool& pool : m_pools)
	{
		pool.m_stackAlloc.getInterface().m_parent = this;
	}

	return Error::kNone;
}

Error TransferGpuAllocator::allocate(PtrSize size, TransferGpuAllocatorHandle& handle)
{
	ANKI_TRACE_SCOPED_EVENT(RSRC_ALLOCATE_TRANSFER);

	const PtrSize poolSize = m_maxAllocSize / kPoolCount;

	LockGuard<Mutex> lock(m_mtx);

	Pool* pool;
	if(m_crntPoolAllocatedSize + size <= poolSize)
	{
		// Have enough space in the pool

		pool = &m_pools[m_crntPool];
	}
	else
	{
		// Don't have enough space. Wait for one pool used in the past

		m_crntPool = U8((m_crntPool + 1) % kPoolCount);
		pool = &m_pools[m_crntPool];

		{
			ANKI_TRACE_SCOPED_EVENT(RSRC_WAIT_TRANSFER);

			// Wait for all memory to be released
			while(pool->m_pendingReleases != 0)
			{
				m_condVar.wait(m_mtx);
			}

			// All memory is released, loop until all fences are triggered
			while(!pool->m_fences.isEmpty())
			{
				FencePtr fence = pool->m_fences.getFront();

				const Bool done = fence->clientWait(kMaxFenceWaitTime);
				if(done)
				{
					pool->m_fences.popFront(*m_pool);
				}
			}
		}

		pool->m_stackAlloc.reset();
		m_crntPoolAllocatedSize = 0;
	}

	Chunk* chunk;
	PtrSize offset;
	[[maybe_unused]] const Error err = pool->m_stackAlloc.allocate(size, kGpuBufferAlignment, chunk, offset);
	ANKI_ASSERT(!err);

	handle.m_buffer = chunk->m_buffer;
	handle.m_mappedMemory = static_cast<U8*>(chunk->m_mappedBuffer) + offset;
	handle.m_offsetInBuffer = offset;
	handle.m_range = size;
	handle.m_pool = U8(pool - &m_pools[0]);

	m_crntPoolAllocatedSize += size;
	++pool->m_pendingReleases;

	// Do a cleanup of done fences. Do that to avoid having too many fences alive. Fences are implemented with file
	// decriptors in Linux and we don't want to exceed the process' limit of max open file descriptors
	for(Pool& p : m_pools)
	{
		List<FencePtr>::Iterator it = p.m_fences.getBegin();
		while(it != p.m_fences.getEnd())
		{
			const Bool fenceDone = (*it)->clientWait(0.0);
			if(fenceDone)
			{
				auto nextIt = it + 1;
				p.m_fences.erase(*m_pool, it);
				it = nextIt;
			}
			else
			{
				++it;
			}
		}
	}

	return Error::kNone;
}

void TransferGpuAllocator::release(TransferGpuAllocatorHandle& handle, FencePtr fence)
{
	ANKI_ASSERT(fence);
	ANKI_ASSERT(handle.valid());

	Pool& pool = m_pools[handle.m_pool];

	{
		LockGuard<Mutex> lock(m_mtx);

		pool.m_fences.pushBack(*m_pool, fence);

		ANKI_ASSERT(pool.m_pendingReleases > 0);
		--pool.m_pendingReleases;

		m_condVar.notifyOne();
	}

	handle.invalidate();
}

} // end namespace anki

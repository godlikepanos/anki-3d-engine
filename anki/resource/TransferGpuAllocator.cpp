// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/TransferGpuAllocator.h>
#include <anki/gr/Fence.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/GrManager.h>
#include <anki/util/Tracer.h>

namespace anki
{

class TransferGpuAllocator::Memory : public StackGpuAllocatorMemory
{
public:
	BufferPtr m_buffer;
	void* m_mappedMemory;
};

class TransferGpuAllocator::Interface : public StackGpuAllocatorInterface
{
public:
	GrManager* m_gr;
	ResourceAllocator<U8> m_alloc;

	ResourceAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	ANKI_USE_RESULT Error allocate(PtrSize size, StackGpuAllocatorMemory*& mem) final
	{
		TransferGpuAllocator::Memory* mm = m_alloc.newInstance<TransferGpuAllocator::Memory>();

		mm->m_buffer = m_gr->newBuffer(
			BufferInitInfo(size, BufferUsageBit::TRANSFER_SOURCE, BufferMapAccessBit::WRITE, "Transfer"));
		mm->m_mappedMemory = mm->m_buffer->map(0, size, BufferMapAccessBit::WRITE);

		mem = mm;
		return Error::NONE;
	}

	void free(StackGpuAllocatorMemory* mem) final
	{
		ANKI_ASSERT(mem);

		TransferGpuAllocator::Memory* mm = static_cast<TransferGpuAllocator::Memory*>(mem);
		if(mm->m_mappedMemory)
		{
			mm->m_buffer->unmap();
		}
		m_alloc.deleteInstance(mm);
	}

	void getChunkGrowInfo(F32& scale, PtrSize& bias, PtrSize& initialSize) final
	{
		scale = 1.5;
		bias = 0;
		initialSize = TransferGpuAllocator::CHUNK_INITIAL_SIZE;
	}

	U32 getMaxAlignment() final
	{
		return 16;
	}
};

BufferPtr TransferGpuAllocatorHandle::getBuffer() const
{
	ANKI_ASSERT(m_handle.m_memory);
	const TransferGpuAllocator::Memory* mm = static_cast<const TransferGpuAllocator::Memory*>(m_handle.m_memory);
	ANKI_ASSERT(mm->m_buffer);
	return mm->m_buffer;
}

void* TransferGpuAllocatorHandle::getMappedMemory() const
{
	ANKI_ASSERT(m_handle.m_memory);
	const TransferGpuAllocator::Memory* mm = static_cast<const TransferGpuAllocator::Memory*>(m_handle.m_memory);
	ANKI_ASSERT(mm->m_mappedMemory);
	return static_cast<U8*>(mm->m_mappedMemory) + m_handle.m_offset;
}

TransferGpuAllocator::TransferGpuAllocator()
{
}

TransferGpuAllocator::~TransferGpuAllocator()
{
	for(Frame& frame : m_frames)
	{
		ANKI_ASSERT(frame.m_pendingReleases == 0);
		frame.m_fences.destroy(m_alloc);
	}
}

Error TransferGpuAllocator::init(PtrSize maxSize, GrManager* gr, ResourceAllocator<U8> alloc)
{
	m_alloc = alloc;
	m_gr = gr;

	m_maxAllocSize = getAlignedRoundUp(CHUNK_INITIAL_SIZE * FRAME_COUNT, maxSize);
	ANKI_RESOURCE_LOGI("Will use %luMB of memory for transfer scratch", m_maxAllocSize / 1024 / 1024);

	m_interface.reset(m_alloc.newInstance<Interface>());
	m_interface->m_gr = gr;
	m_interface->m_alloc = alloc;

	for(Frame& frame : m_frames)
	{
		frame.m_stackAlloc.init(m_alloc, m_interface.get());
	}

	return Error::NONE;
}

Error TransferGpuAllocator::allocate(PtrSize size, TransferGpuAllocatorHandle& handle)
{
	ANKI_TRACE_SCOPED_EVENT(RSRC_ALLOCATE_TRANSFER);

	const PtrSize frameSize = m_maxAllocSize / FRAME_COUNT;

	LockGuard<Mutex> lock(m_mtx);

	Frame* frame;
	if(m_crntFrameAllocatedSize + size <= frameSize)
	{
		// Have enough space in the frame

		frame = &m_frames[m_frameCount];
	}
	else
	{
		// Don't have enough space. Wait for next frame

		m_frameCount = U8((m_frameCount + 1) % FRAME_COUNT);
		Frame& nextFrame = m_frames[m_frameCount];

		// Wait for all memory to be released
		while(nextFrame.m_pendingReleases != 0)
		{
			m_condVar.wait(m_mtx);
		}

		// Wait all fences
		while(!nextFrame.m_fences.isEmpty())
		{
			FencePtr fence = nextFrame.m_fences.getFront();

			const Bool done = fence->clientWait(MAX_FENCE_WAIT_TIME);
			if(done)
			{
				nextFrame.m_fences.popFront(m_alloc);
			}
		}

		nextFrame.m_stackAlloc.reset();
		m_crntFrameAllocatedSize = 0;
		frame = &nextFrame;
	}

	ANKI_CHECK(frame->m_stackAlloc.allocate(size, handle.m_handle));
	handle.m_range = size;
	handle.m_frame = U8(frame - &m_frames[0]);
	m_crntFrameAllocatedSize += size;
	++frame->m_pendingReleases;

	return Error::NONE;
}

void TransferGpuAllocator::release(TransferGpuAllocatorHandle& handle, FencePtr fence)
{
	ANKI_ASSERT(fence);
	ANKI_ASSERT(handle.valid());

	Frame& frame = m_frames[handle.m_frame];

	{
		LockGuard<Mutex> lock(m_mtx);

		frame.m_fences.pushBack(m_alloc, fence);

		ANKI_ASSERT(frame.m_pendingReleases > 0);
		--frame.m_pendingReleases;

		m_condVar.notifyOne();
	}

	handle.invalidate();
}

} // end namespace anki

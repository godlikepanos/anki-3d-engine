// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/Readback.h>

namespace anki {

void ReadbackManager::allocateData(MultiframeReadbackToken& token, PtrSize size, Buffer*& buffer, PtrSize& bufferOffset) const
{
	for([[maybe_unused]] U64 frame : token.m_frameIds)
	{
		ANKI_ASSERT(frame != m_frameId && "Can't allocate multiple times in a frame");
	}

	GpuReadbackMemoryAllocation& allocation = token.m_allocations[token.m_slot];

	if(allocation.isValid() && allocation.getAllocatedSize() != size)
	{
		GpuReadbackMemoryPool::getSingleton().deferredFree(allocation);
	}

	if(!allocation.isValid())
	{
		allocation = GpuReadbackMemoryPool::getSingleton().allocate(size);
	}
	token.m_frameIds[token.m_slot] = m_frameId;

	buffer = &allocation.getBuffer();
	bufferOffset = allocation.getOffset();

	token.m_slot = (token.m_slot + 1) % kMaxFramesInFlight;
}

U32 ReadbackManager::findBestSlot(const MultiframeReadbackToken& token) const
{
	const U64 earliestFrame = m_frameId - (kMaxFramesInFlight - 1);
	U32 bestSlot = kMaxU32;
	U32 secondBestSlot = kMaxU32;

	for(U32 i = 0; i < kMaxFramesInFlight; ++i)
	{
		if(token.m_frameIds[i] == earliestFrame && token.m_allocations[i].isValid())
		{
			bestSlot = i;
			break;
		}
		else if(token.m_frameIds[i] < earliestFrame && token.m_allocations[i].isValid())
		{
			secondBestSlot = i;
		}
	}

	const U32 slot = (bestSlot != kMaxU32) ? bestSlot : secondBestSlot;
	return slot;
}

void ReadbackManager::readMostRecentData(const MultiframeReadbackToken& token, void* data, PtrSize dataSize, PtrSize& dataOut) const
{
	ANKI_ASSERT(data && dataSize > 0);
	dataOut = 0;

	const U32 slot = findBestSlot(token);
	if(slot == kMaxU32)
	{
		return;
	}

	const GpuReadbackMemoryAllocation& allocation = token.m_allocations[slot];
	dataOut = min(dataSize, PtrSize(allocation.getAllocatedSize()));

	memcpy(data, static_cast<const U8*>(allocation.getMappedMemory()), dataOut);
}

void ReadbackManager::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);

	// Release fences
	for(Frame& frame : m_frames)
	{
		if(frame.m_fence.isCreated())
		{
			if(frame.m_fence->clientWait(0.0))
			{
				frame.m_fence.reset(nullptr);
			}
		}
	}

	Frame& frame = m_frames[m_frameId % m_frames.getSize()];
	if(frame.m_fence.isCreated()) [[unlikely]]
	{
		ANKI_R_LOGW("Readback fence is not signaled. Need to wait it");
		const Bool signaled = frame.m_fence->clientWait(10.0_sec);
		if(!signaled)
		{
			ANKI_R_LOGF("Fence won't signal. Can't recover");
		}
	}

	frame.m_fence.reset(fence);

	++m_frameId;
}

} // end namespace anki

// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Utils/Readback.h>

namespace anki {

const GpuReadbackMemoryAllocation* ReadbackManager::findBestAllocation(const MultiframeReadbackToken& token) const
{
	const MultiframeReadbackToken::Allocation* candidate = nullptr;
	for(auto it = token.m_allocations.getBegin(); it != token.m_allocations.getEnd(); ++it)
	{
		const Bool frameHasFinished = it->m_frameIndex <= m_lastFinishedFrame;
		if(frameHasFinished)
		{
			if(candidate == nullptr || it->m_frameIndex > candidate->m_frameIndex)
			{
				// No candidate or candidate that is later frame
				candidate = &(*it);
			}
		}
	}

	return (candidate) ? &candidate->m_alloc : nullptr;
}

void ReadbackManager::readMostRecentData(const MultiframeReadbackToken& token, void* data, PtrSize dataSize, PtrSize& dataOut) const
{
	ANKI_ASSERT(data && dataSize > 0);
	dataOut = 0;

	const GpuReadbackMemoryAllocation* alloc = findBestAllocation(token);
	if(alloc == nullptr)
	{
		return;
	}

	dataOut = min(dataSize, alloc->getSize());
	memcpy(data, alloc->getMappedMemory(), dataOut);
}

void ReadbackManager::endFrame(Fence* fence)
{
	ANKI_ASSERT(fence);

	// Delete frames
	Array<U32, 8> framesToDelete;
	ANKI_ASSERT(m_frames.getSize() <= framesToDelete.getSize() && "Can't buffer that many frames. Something is wrong");
	U32 framesToDeleteCount = 0;
	for(Frame& frame : m_frames)
	{
		ANKI_ASSERT(frame.m_fence);

		const Bool signaled = frame.m_fence->signaled();

		if(frame.m_frameIndex <= m_lastFinishedFrame)
		{
			ANKI_ASSERT(signaled);
		}

		if(signaled)
		{
			m_lastFinishedFrame = max(m_lastFinishedFrame, frame.m_frameIndex);

			framesToDelete[framesToDeleteCount++] = frame.m_blockArrayIndex;
		}
	}

	for(U32 i = 0; i < framesToDeleteCount; ++i)
	{
		m_frames.erase(framesToDelete[i]);
	}

	// Create new frame
	auto it = m_frames.emplace();
	Frame& frame = *it;
	frame.m_fence.reset(fence);
	frame.m_frameIndex = m_crntFrame;
	frame.m_blockArrayIndex = it.getArrayIndex();

	++m_crntFrame;
}

} // end namespace anki

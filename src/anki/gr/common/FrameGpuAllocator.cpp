// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/FrameGpuAllocator.h>

namespace anki
{

void FrameGpuAllocator::init(PtrSize size, U32 alignment, PtrSize maxAllocationSize)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(size > 0 && alignment > 0 && maxAllocationSize > 0);

	PtrSize perFrameSize = size / MAX_FRAMES_IN_FLIGHT;
	alignRoundDown(alignment, perFrameSize);
	m_size = perFrameSize * MAX_FRAMES_IN_FLIGHT;

	m_alignment = alignment;
	m_maxAllocationSize = maxAllocationSize;
}

PtrSize FrameGpuAllocator::endFrame()
{
	ANKI_ASSERT(isCreated());

	PtrSize perFrameSize = m_size / MAX_FRAMES_IN_FLIGHT;

	PtrSize crntFrameStartOffset = perFrameSize * (m_frame % MAX_FRAMES_IN_FLIGHT);

	PtrSize nextFrameStartOffset = perFrameSize * ((m_frame + 1) % MAX_FRAMES_IN_FLIGHT);

	PtrSize crntOffset = m_offset.exchange(nextFrameStartOffset);
	ANKI_ASSERT(crntOffset >= crntFrameStartOffset);

	PtrSize bytesUsed = crntOffset - crntFrameStartOffset;
	PtrSize bytesNotUsed = (bytesUsed > perFrameSize) ? 0 : perFrameSize - bytesUsed;

	++m_frame;
	return bytesNotUsed;
}

Error FrameGpuAllocator::allocate(PtrSize originalSize, PtrSize& outOffset)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(originalSize > 0);
	Error err = ErrorCode::NONE;

	// Align size
	PtrSize size = getAlignedRoundUp(m_alignment, originalSize);
	ANKI_ASSERT(size <= m_maxAllocationSize && "Too high!");

	PtrSize offset = m_offset.fetchAdd(size);
	PtrSize perFrameSize = m_size / MAX_FRAMES_IN_FLIGHT;
	PtrSize crntFrameStartOffset = perFrameSize * (m_frame % MAX_FRAMES_IN_FLIGHT);

	if(offset - crntFrameStartOffset + size <= perFrameSize)
	{
		ANKI_ASSERT(isAligned(m_alignment, offset));
		ANKI_ASSERT((offset + size) <= m_size);

#if ANKI_ENABLE_TRACE
		m_lastAllocatedSize.store(size);
#endif

		// Encode token
		outOffset = offset;

		ANKI_ASSERT(outOffset + originalSize <= m_size);
	}
	else
	{
		outOffset = MAX_PTR_SIZE;
		err = ErrorCode::OUT_OF_MEMORY;
	}

	return err;
}

#if ANKI_ENABLE_TRACE
PtrSize FrameGpuAllocator::getUnallocatedMemorySize() const
{
	PtrSize perFrameSize = m_size / MAX_FRAMES_IN_FLIGHT;
	PtrSize crntFrameStartOffset = perFrameSize * (m_frame % MAX_FRAMES_IN_FLIGHT);
	PtrSize usedSize = m_offset.get() - crntFrameStartOffset + m_lastAllocatedSize.get();

	PtrSize remaining = (perFrameSize >= usedSize) ? (perFrameSize - usedSize) : 0;
	return remaining;
}
#endif

} // end namespace anki
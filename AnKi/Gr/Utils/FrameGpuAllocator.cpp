// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Utils/FrameGpuAllocator.h>

namespace anki {

void FrameGpuAllocator::init(PtrSize size, U32 alignment, PtrSize maxAllocationSize)
{
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(size > 0 && alignment > 0 && maxAllocationSize > 0);

	PtrSize perFrameSize = size / kMaxFramesInFlight;
	alignRoundDown(alignment, perFrameSize);
	m_size = perFrameSize * kMaxFramesInFlight;

	m_alignment = alignment;
	m_maxAllocationSize = maxAllocationSize;
}

PtrSize FrameGpuAllocator::endFrame()
{
	ANKI_ASSERT(isCreated());

	PtrSize perFrameSize = m_size / kMaxFramesInFlight;

	PtrSize crntFrameStartOffset = perFrameSize * (m_frame % kMaxFramesInFlight);

	PtrSize nextFrameStartOffset = perFrameSize * ((m_frame + 1) % kMaxFramesInFlight);

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
	Error err = Error::kNone;

	// Align size
	PtrSize size = getAlignedRoundUp(m_alignment, originalSize);
	ANKI_ASSERT(size <= m_maxAllocationSize && "Too high!");

	const PtrSize offset = m_offset.fetchAdd(size);
	const PtrSize perFrameSize = m_size / kMaxFramesInFlight;
	const PtrSize crntFrameStartOffset = perFrameSize * (m_frame % kMaxFramesInFlight);

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
		outOffset = kMaxPtrSize;
		err = Error::kOutOfMemory;
	}

	return err;
}

#if ANKI_ENABLE_TRACE
PtrSize FrameGpuAllocator::getUnallocatedMemorySize() const
{
	PtrSize perFrameSize = m_size / kMaxFramesInFlight;
	PtrSize crntFrameStartOffset = perFrameSize * (m_frame % kMaxFramesInFlight);
	PtrSize usedSize = m_offset.getNonAtomically() - crntFrameStartOffset + m_lastAllocatedSize.getNonAtomically();

	PtrSize remaining = (perFrameSize >= usedSize) ? (perFrameSize - usedSize) : 0;
	return remaining;
}
#endif

} // end namespace anki

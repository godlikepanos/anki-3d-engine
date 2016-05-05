// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/GpuFrameRingAllocator.h>

namespace anki
{

//==============================================================================
void GpuFrameRingAllocator::init(
	void* cpuMappedMem, PtrSize size, U32 alignment, PtrSize maxAllocationSize)
{
	ANKI_ASSERT(
		cpuMappedMem && size > 0 && alignment > 0 && maxAllocationSize > 0);

	PtrSize perFrameSize = size / MAX_FRAMES_IN_FLIGHT;
	alignRoundDown(alignment, perFrameSize);
	m_size = perFrameSize * MAX_FRAMES_IN_FLIGHT;

	m_cpuAddress = static_cast<U8*>(cpuMappedMem);
	m_alignment = alignment;
	m_maxAllocationSize = maxAllocationSize;
}

//==============================================================================
PtrSize GpuFrameRingAllocator::endFrame()
{
	PtrSize perFrameSize = m_size / MAX_FRAMES_IN_FLIGHT;

	PtrSize crntFrameStartOffset =
		perFrameSize * (m_frame % MAX_FRAMES_IN_FLIGHT);

	PtrSize nextFrameStartOffset =
		perFrameSize * ((m_frame + 1) % MAX_FRAMES_IN_FLIGHT);

	PtrSize crntOffset = m_offset.exchange(nextFrameStartOffset);
	ANKI_ASSERT(crntOffset >= crntFrameStartOffset);

	PtrSize bytesUsed = crntOffset - crntFrameStartOffset;
	PtrSize bytesNotUsed =
		(bytesUsed > perFrameSize) ? 0 : perFrameSize - bytesUsed;

	++m_frame;
	return bytesNotUsed;
}

//==============================================================================
void* GpuFrameRingAllocator::allocate(
	PtrSize originalSize, DynamicBufferToken& token)
{
	ANKI_ASSERT(originalSize > 0);
	ANKI_ASSERT(m_cpuAddress);

	// Align size
	PtrSize size = getAlignedRoundUp(m_alignment, originalSize);
	ANKI_ASSERT(size <= m_maxAllocationSize && "Too high!");

	PtrSize offset = m_offset.fetchAdd(size);
	PtrSize perFrameSize = m_size / MAX_FRAMES_IN_FLIGHT;
	PtrSize crntFrameStartOffset =
		perFrameSize * (m_frame % MAX_FRAMES_IN_FLIGHT);

	if(offset - crntFrameStartOffset + size <= perFrameSize)
	{
		ANKI_ASSERT(isAligned(m_alignment, m_cpuAddress + offset));
		ANKI_ASSERT((offset + size) <= m_size);

		// Encode token
		token.m_offset = offset;
		token.m_range = originalSize;

		return static_cast<void*>(m_cpuAddress + offset);
	}
	else
	{
		ANKI_LOGF("Out of GPU dynamic memory");
	}

	return nullptr;
}

} // end namespace anki
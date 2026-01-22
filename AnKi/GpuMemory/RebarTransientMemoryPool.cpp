// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Fence.h>

namespace anki {

ANKI_SVAR(RebarUserMemory, StatCategory::kGpuMem, "ReBAR used mem", StatFlag::kBytes | StatFlag::kMainThreadUpdates)

RebarTransientMemoryPool::~RebarTransientMemoryPool()
{
	GrManager::getSingleton().finish();

	m_buffer->unmap();
	m_buffer.reset(nullptr);
}

void RebarTransientMemoryPool::init()
{
	BufferInitInfo buffInit("ReBar");
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	buffInit.m_size = g_cvarCoreRebarGpuMemorySize;
	buffInit.m_usage = BufferUsageBit::kAllConstant | BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv | BufferUsageBit::kVertexOrIndex
					   | BufferUsageBit::kShaderBindingTable | BufferUsageBit::kAllIndirect | BufferUsageBit::kCopySource
					   | BufferUsageBit::kAccelerationStructureBuild;
	m_buffer = GrManager::getSingleton().newBuffer(buffInit);

	m_bufferSize = buffInit.m_size;

	if(!GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
	{
		m_structuredBufferAlignment = GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
	}

	m_mappedMem = static_cast<U8*>(m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

	// Create the slice of the 1st frame
	m_activeSliceMask.set(0);
	m_slices[0].m_offset = 0;
	m_crntActiveSlice = 0;
}

BufferView RebarTransientMemoryPool::allocateInternal(PtrSize origSize, U32 alignment, void*& mappedMem)
{
	ANKI_ASSERT(origSize > 0);
	ANKI_ASSERT(alignment > 0);
	const PtrSize size = origSize + alignment;

	// Try in a loop because we may end up with an allocation its offset crosses the buffer's end
	PtrSize offset;
	Bool done = false;
	do
	{
		offset = m_offset.fetchAdd(size) % m_bufferSize;
		const PtrSize end = (offset + size) % m_bufferSize;

		done = offset < end;
	} while(!done);

	// Wait for the range that contains the offset
	m_activeSliceMask.iterateSetBitsFromLeastSignificant([&](U32 sliceIdx) {
		if(sliceIdx == m_crntActiveSlice)
		{
			return FunctorContinue::kContinue;
		}

		const FrameSlice& slice = m_slices[sliceIdx];

		Bool overlaps;
		if(offset <= slice.m_offset)
		{
			overlaps = offset + size > slice.m_offset;
		}
		else
		{
			overlaps = slice.m_offset + slice.m_range > offset;
		}

		if(overlaps)
		{
			ANKI_CORE_LOGW("ReBAR has to wait for a fence. This means that the ReBAR buffer is not big enough. Increase the %s CVAR",
						   g_cvarCoreRebarGpuMemorySize.getName().cstr());

			if(!m_sliceFences[sliceIdx]->clientWait(kMaxSecond))
			{
				ANKI_CORE_LOGF("Timeout detected");
			}
		}

		return FunctorContinue::kContinue;
	});

	const PtrSize alignedOffset = getAlignedRoundUp(alignment, offset);
	ANKI_ASSERT(alignedOffset + origSize <= offset + size);
	ANKI_ASSERT(offset + size <= m_bufferSize);

	mappedMem = m_mappedMem + alignedOffset;
	return BufferView(m_buffer.get(), alignedOffset, origSize);
}

void RebarTransientMemoryPool::endFrame(Fence* fence)
{
	// Free up previous slices
	m_activeSliceMask.iterateSetBitsFromLeastSignificant([&](U32 sliceIdx) {
		if(sliceIdx != m_crntActiveSlice)
		{
			if(m_sliceFences[sliceIdx]->signaled())
			{
				m_sliceFences[sliceIdx].reset(nullptr);
				m_slices[sliceIdx] = {};
				m_activeSliceMask.unset(sliceIdx);
			}
		}

		return FunctorContinue::kContinue;
	});

	// Finalize the active slice
	const PtrSize crntOffset = m_offset.getNonAtomically();
	const PtrSize crntNormalizedOffset = crntOffset % m_bufferSize;

	FrameSlice& slice = m_slices[m_crntActiveSlice];
	ANKI_ASSERT(slice.m_offset < kMaxPtrSize && slice.m_range == 0 && !m_sliceFences[m_crntActiveSlice]);

	ANKI_ASSERT(crntOffset >= slice.m_offset);
	const PtrSize range = crntOffset - slice.m_offset;

	if(range == 0)
	{
		// No allocations this frame, remove the slice

		slice = {};
		m_activeSliceMask.unset(m_crntActiveSlice);
	}
	else if((slice.m_offset % m_bufferSize) + range > m_bufferSize)
	{
		// The frame we are ending wrapped arround the ReBAR buffer, create two slices

		slice.m_offset = slice.m_offset % m_bufferSize;
		slice.m_range = m_bufferSize - slice.m_offset;
		m_sliceFences[m_crntActiveSlice].reset(fence);

		const U32 secondSliceIdx = (~m_activeSliceMask).getLeastSignificantBit();
		m_slices[secondSliceIdx].m_offset = 0;
		m_slices[secondSliceIdx].m_range = range - slice.m_range;
		ANKI_ASSERT(crntNormalizedOffset == m_slices[secondSliceIdx].m_range);
		m_sliceFences[secondSliceIdx].reset(fence);
		m_activeSliceMask.set(secondSliceIdx);
	}
	else
	{
		// No wrapping, just finalize the active slice

		slice.m_offset = slice.m_offset % m_bufferSize;
		slice.m_range = range;
		m_sliceFences[m_crntActiveSlice].reset(fence);
	}

	// Create a new active slice
	const U32 newSliceIdx = (~m_activeSliceMask).getLeastSignificantBit();
	m_activeSliceMask.set(newSliceIdx);
	m_slices[newSliceIdx].m_offset = crntOffset;
	m_crntActiveSlice = newSliceIdx;

	validateSlices();

	// Stats
	const PtrSize usedMemory = range;
	ANKI_TRACE_INC_COUNTER(ReBarUsedMemory, usedMemory);
	g_svarRebarUserMemory.set(usedMemory);
}

void RebarTransientMemoryPool::validateSlices() const
{
	for(U32 sliceIdxA = 0; sliceIdxA < kSliceCount; ++sliceIdxA)
	{
		if(sliceIdxA == m_crntActiveSlice)
		{
			ANKI_ASSERT(m_activeSliceMask.get(sliceIdxA));
			ANKI_ASSERT(m_slices[sliceIdxA].m_offset < kMaxPtrSize && m_slices[sliceIdxA].m_range == 0 && !m_sliceFences[sliceIdxA]);
		}
		else if(m_activeSliceMask.get(sliceIdxA))
		{
			const FrameSlice& a = m_slices[sliceIdxA];
			ANKI_ASSERT(a.m_offset < kMaxPtrSize && a.m_range > 0 && a.m_offset + a.m_range <= m_bufferSize);
			ANKI_ASSERT(!!m_sliceFences[sliceIdxA]);

			m_activeSliceMask.iterateSetBitsFromLeastSignificant([&](U32 sliceIdxB) {
				if(sliceIdxA == sliceIdxB || sliceIdxB == m_crntActiveSlice)
				{
					return FunctorContinue::kContinue;
				}

				const FrameSlice& b = m_slices[sliceIdxB];

				if(a.m_offset < b.m_offset)
				{
					ANKI_ASSERT(a.m_offset + a.m_range <= b.m_offset);
				}
				else if(b.m_offset < a.m_offset)
				{
					ANKI_ASSERT(b.m_offset + b.m_range <= a.m_offset);
				}
				else
				{
					ANKI_ASSERT(0 && "Offsets can't be equal");
				}

				ANKI_ASSERT(m_sliceFences[sliceIdxA] && m_sliceFences[sliceIdxB]);

				return FunctorContinue::kContinue;
			});
		}
		else
		{
			const FrameSlice& a = m_slices[sliceIdxA];
			ANKI_ASSERT(a.m_offset == kMaxPtrSize && a.m_range == 0 && !m_sliceFences[sliceIdxA]);
		}
	}
}

} // end namespace anki

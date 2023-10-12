// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

static StatCounter g_rebarUserMemoryStatVar(StatCategory::kGpuMem, "ReBAR used mem", StatFlag::kBytes | StatFlag::kMainThreadUpdates);

static NumericCVar<PtrSize> g_rebarGpuMemorySizeCvar(CVarSubsystem::kCore, "RebarGpuMemorySize", 24_MB, 1_MB, 1_GB,
													 "ReBAR: always mapped GPU memory");

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
	buffInit.m_size = g_rebarGpuMemorySizeCvar.get();
	buffInit.m_usage = BufferUsageBit::kAllConstant | BufferUsageBit::kAllUav | BufferUsageBit::kVertex | BufferUsageBit::kIndex
					   | BufferUsageBit::kShaderBindingTable | BufferUsageBit::kAllIndirect | BufferUsageBit::kTransferSource;
	m_buffer = GrManager::getSingleton().newBuffer(buffInit);

	m_bufferSize = buffInit.m_size;

	m_alignment = GrManager::getSingleton().getDeviceCapabilities().m_constantBufferBindOffsetAlignment;
	m_alignment = max(m_alignment, GrManager::getSingleton().getDeviceCapabilities().m_uavBufferBindOffsetAlignment);
	m_alignment = max(m_alignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);

	m_mappedMem = static_cast<U8*>(m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
}

RebarAllocation RebarTransientMemoryPool::allocateFrame(PtrSize size, void*& mappedMem)
{
	RebarAllocation out = tryAllocateFrame(size, mappedMem);
	if(!out.isValid()) [[unlikely]]
	{
		ANKI_CORE_LOGF("Out of ReBAR GPU memory");
	}

	return out;
}

RebarAllocation RebarTransientMemoryPool::tryAllocateFrame(PtrSize origSize, void*& mappedMem)
{
	ANKI_ASSERT(origSize > 0);
	const PtrSize size = getAlignedRoundUp(m_alignment, origSize);

	// Try in a loop because we may end up with an allocation its offset crosses the buffer's end
	PtrSize offset;
	Bool done = false;
	do
	{
		offset = m_offset.fetchAdd(size) % m_bufferSize;
		const PtrSize end = (offset + origSize) % (m_bufferSize + 1);

		done = offset < end;
	} while(!done);

	mappedMem = m_mappedMem + offset;
	RebarAllocation out;
	out.m_offset = offset;
	out.m_range = origSize;

	return out;
}

void RebarTransientMemoryPool::endFrame()
{
	const PtrSize crntOffset = m_offset.getNonAtomically();

	const PtrSize usedMemory = crntOffset - m_previousFrameEndOffset;
	m_previousFrameEndOffset = crntOffset;

	if(usedMemory >= PtrSize(0.8 * F64(m_bufferSize / kMaxFramesInFlight)))
	{
		ANKI_CORE_LOGW("Frame used more that 80%% of its safe limit of ReBAR memory");
	}

	ANKI_TRACE_INC_COUNTER(ReBarUsedMemory, usedMemory);
	g_rebarUserMemoryStatVar.set(usedMemory);
}

} // end namespace anki

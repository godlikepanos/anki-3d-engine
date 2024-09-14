// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

inline StatCounter g_rebarUserMemoryStatVar(StatCategory::kGpuMem, "ReBAR used mem", StatFlag::kBytes | StatFlag::kMainThreadUpdates);

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
	buffInit.m_size = g_rebarGpuMemorySizeCvar;
	buffInit.m_usage = BufferUsageBit::kAllConstant | BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv | BufferUsageBit::kVertex
					   | BufferUsageBit::kIndex | BufferUsageBit::kShaderBindingTable | BufferUsageBit::kAllIndirect | BufferUsageBit::kCopySource;
	m_buffer = GrManager::getSingleton().newBuffer(buffInit);

	m_bufferSize = buffInit.m_size;

	if(!GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
	{
		m_structuredBufferAlignment = GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
	}

	m_mappedMem = static_cast<U8*>(m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
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
		const PtrSize end = (offset + size) % (m_bufferSize + 1);

		done = offset < end;
	} while(!done);

	const PtrSize alignedOffset = getAlignedRoundUp(alignment, offset);
	ANKI_ASSERT(alignedOffset + origSize <= offset + size);

	mappedMem = m_mappedMem + alignedOffset;
	return BufferView(m_buffer.get(), alignedOffset, origSize);
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

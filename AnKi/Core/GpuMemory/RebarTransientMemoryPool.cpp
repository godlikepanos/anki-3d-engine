// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

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
	buffInit.m_size = ConfigSet::getSingleton().getCoreRebarGpuMemorySize();
	buffInit.m_usage = BufferUsageBit::kAllUniform | BufferUsageBit::kAllStorage | BufferUsageBit::kVertex | BufferUsageBit::kIndex
					   | BufferUsageBit::kShaderBindingTable | BufferUsageBit::kAllIndirect;
	m_buffer = GrManager::getSingleton().newBuffer(buffInit);

	m_bufferSize = buffInit.m_size;

	m_alignment = GrManager::getSingleton().getDeviceCapabilities().m_uniformBufferBindOffsetAlignment;
	m_alignment = max(m_alignment, GrManager::getSingleton().getDeviceCapabilities().m_storageBufferBindOffsetAlignment);
	m_alignment = max(m_alignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);

	m_mappedMem = static_cast<U8*>(m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
}

void* RebarTransientMemoryPool::allocateFrame(PtrSize size, RebarAllocation& token)
{
	void* address = tryAllocateFrame(size, token);
	if(address == nullptr) [[unlikely]]
	{
		ANKI_CORE_LOGF("Out of ReBAR GPU memory");
	}

	return address;
}

void* RebarTransientMemoryPool::tryAllocateFrame(PtrSize origSize, RebarAllocation& token)
{
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

	void* address = m_mappedMem + offset;
	token.m_offset = offset;
	token.m_range = origSize;

	return address;
}

PtrSize RebarTransientMemoryPool::endFrame()
{
	const PtrSize crntOffset = m_offset.getNonAtomically();

	const PtrSize usedMemory = crntOffset - m_previousFrameEndOffset;
	m_previousFrameEndOffset = crntOffset;

	if(usedMemory >= PtrSize(0.8 * F64(m_bufferSize / kMaxFramesInFlight)))
	{
		ANKI_CORE_LOGW("Frame used more that 80%% of its safe limit of ReBAR memory");
	}

	ANKI_TRACE_INC_COUNTER(ReBarUsedMemory, usedMemory);
	return usedMemory;
}

} // end namespace anki

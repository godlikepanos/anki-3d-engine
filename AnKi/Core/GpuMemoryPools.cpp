// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

void UnifiedGeometryMemoryPool::init(HeapMemoryPool* pool, GrManager* gr, const ConfigSet& cfg)
{
	ANKI_ASSERT(pool && gr);

	const PtrSize poolSize = cfg.getCoreGlobalVertexMemorySize();

	const Array classes = {1_KB, 8_KB, 32_KB, 128_KB, 512_KB, 4_MB, 8_MB, 16_MB, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kVertex | BufferUsageBit::kIndex | BufferUsageBit::kTransferDestination;

	if(gr->getDeviceCapabilities().m_rayTracingEnabled)
	{
		buffUsage |= BufferUsageBit::kAccelerationStructureBuild;
	}

	m_alloc.init(gr, pool, buffUsage, classes, poolSize, "UnifiedGeometry", false);
}

void GpuSceneMemoryPool::init(HeapMemoryPool* pool, GrManager* gr, const ConfigSet& cfg)
{
	ANKI_ASSERT(pool && gr);

	const PtrSize poolSize = cfg.getCoreGpuSceneInitialSize();

	const Array classes = {32_B, 64_B, 128_B, 256_B, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kAllStorage | BufferUsageBit::kTransferDestination;

	m_alloc.init(gr, pool, buffUsage, classes, poolSize, "GpuScene", true);
}

RebarStagingGpuMemoryPool::~RebarStagingGpuMemoryPool()
{
	GrManager& gr = m_buffer->getManager();

	gr.finish();

	m_buffer->unmap();
	m_buffer.reset(nullptr);
}

Error RebarStagingGpuMemoryPool::init(GrManager* gr, const ConfigSet& cfg)
{
	BufferInitInfo buffInit("ReBar");
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	buffInit.m_size = cfg.getCoreRebarGpuMemorySize();
	buffInit.m_usage =
		BufferUsageBit::kAllUniform | BufferUsageBit::kAllStorage | BufferUsageBit::kVertex | BufferUsageBit::kIndex;
	m_buffer = gr->newBuffer(buffInit);

	m_bufferSize = buffInit.m_size;

	m_alignment = gr->getDeviceCapabilities().m_uniformBufferBindOffsetAlignment;
	m_alignment = max(m_alignment, gr->getDeviceCapabilities().m_storageBufferBindOffsetAlignment);
	m_alignment = max(m_alignment, gr->getDeviceCapabilities().m_sbtRecordAlignment);

	m_mappedMem = static_cast<U8*>(m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

	return Error::kNone;
}

void* RebarStagingGpuMemoryPool::allocateFrame(PtrSize size, RebarGpuMemoryToken& token)
{
	void* address = tryAllocateFrame(size, token);
	if(ANKI_UNLIKELY(address == nullptr))
	{
		ANKI_CORE_LOGF("Out of ReBAR GPU memory");
	}

	return address;
}

void* RebarStagingGpuMemoryPool::tryAllocateFrame(PtrSize origSize, RebarGpuMemoryToken& token)
{
	const PtrSize size = getAlignedRoundUp(m_alignment, origSize);
	const PtrSize offset = m_offset.fetchAdd(size);

	void* address;
	if(ANKI_UNLIKELY(offset + origSize > m_bufferSize))
	{
		token = {};
		address = nullptr;
	}
	else
	{
		address = m_mappedMem + offset;

		token.m_offset = offset;
		token.m_range = origSize;
	}

	return address;
}

PtrSize RebarStagingGpuMemoryPool::endFrame()
{
	const PtrSize crntOffset = m_offset.getNonAtomically();

	PtrSize usedMemory;
	if(crntOffset >= m_previousFrameEndOffset)
	{
		usedMemory = crntOffset - m_previousFrameEndOffset;
	}
	else
	{
		usedMemory = crntOffset;
	}

	m_previousFrameEndOffset = crntOffset;

	m_frameCount = (m_frameCount + 1) % kMaxFramesInFlight;
	if(m_frameCount == 0)
	{
		m_offset.setNonAtomically(0);
	}

	ANKI_TRACE_INC_COUNTER(ReBarUsedMemory, usedMemory);
	return usedMemory;
}

} // end namespace anki

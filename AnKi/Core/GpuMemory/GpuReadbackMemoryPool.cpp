// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemory/GpuReadbackMemoryPool.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

GpuReadbackMemoryPool::GpuReadbackMemoryPool()
{
	const Array classes = {64_B, 256_B, 1_MB, 5_MB};

	const BufferUsageBit buffUsage = BufferUsageBit::kAllUav;
	const BufferMapAccessBit mapAccess = BufferMapAccessBit::kRead;

	m_pool.init(buffUsage, classes, classes.getBack(), "GpuReadback", false, mapAccess);

	m_alignment = GrManager::getSingleton().getDeviceCapabilities().m_uavBufferBindOffsetAlignment;
}

GpuReadbackMemoryPool ::~GpuReadbackMemoryPool()
{
}

GpuReadbackMemoryAllocation GpuReadbackMemoryPool::allocate(PtrSize size)
{
	GpuReadbackMemoryAllocation out;
	m_pool.allocate(size, m_alignment, out.m_token);
	out.m_buffer = &m_pool.getGpuBuffer();
	out.m_mappedMemory = static_cast<U8*>(m_pool.getGpuBufferMappedMemory()) + out.m_token.m_offset;
	return out;
}

void GpuReadbackMemoryPool::deferredFree(GpuReadbackMemoryAllocation& allocation)
{
	m_pool.deferredFree(allocation.m_token);
	::new(&allocation) GpuReadbackMemoryAllocation();
}

void GpuReadbackMemoryPool::endFrame()
{
	m_pool.endFrame();
}

} // end namespace anki

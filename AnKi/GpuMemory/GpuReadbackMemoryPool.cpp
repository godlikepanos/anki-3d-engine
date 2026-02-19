// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/GpuReadbackMemoryPool.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

GpuReadbackMemoryPool::GpuReadbackMemoryPool()
{
	const Array classes = {64_B, 256_B, 1_MB, 5_MB};

	const BufferUsageBit buffUsage = BufferUsageBit::kAllUav;
	const BufferMapAccessBit mapAccess = BufferMapAccessBit::kRead;

	m_pool.init(buffUsage, classes, classes.getBack(), "GpuReadback", mapAccess);

	if(!GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
	{
		m_structuredBufferAlignment = GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
	}
}

} // end namespace anki

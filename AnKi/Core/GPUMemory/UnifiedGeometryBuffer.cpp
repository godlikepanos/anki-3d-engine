// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GPUMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

void UnifiedGeometryBuffer::init()
{
	const PtrSize poolSize = ConfigSet::getSingleton().getCoreGlobalVertexMemorySize();

	const Array classes = {1_KB, 8_KB, 32_KB, 128_KB, 512_KB, 4_MB, 8_MB, 16_MB, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kVertex | BufferUsageBit::kIndex | BufferUsageBit::kTransferDestination
							   | (BufferUsageBit::kAllTexture & BufferUsageBit::kAllRead);

	if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled)
	{
		buffUsage |= BufferUsageBit::kAccelerationStructureBuild;
	}

	m_pool.init(buffUsage, classes, poolSize, "UnifiedGeometry", false);

	// Allocate something dummy to force creating the GPU buffer
	UnifiedGeometryBufferAllocation alloc;
	allocate(16, 4, alloc);
	deferredFree(alloc);
}

} // end namespace anki

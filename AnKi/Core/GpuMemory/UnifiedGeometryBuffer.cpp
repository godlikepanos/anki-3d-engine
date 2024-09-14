// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemory/UnifiedGeometryBuffer.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

inline StatCounter g_unifiedGeomBufferAllocatedSizeStatVar(StatCategory::kGpuMem, "UGB allocated", StatFlag::kBytes | StatFlag::kMainThreadUpdates);
inline StatCounter g_unifiedGeomBufferTotalStatVar(StatCategory::kGpuMem, "UGB total", StatFlag::kBytes | StatFlag::kMainThreadUpdates);
inline StatCounter g_unifiedGeomBufferFragmentationStatVar(StatCategory::kGpuMem, "UGB fragmentation",
														   StatFlag::kFloat | StatFlag::kMainThreadUpdates);

void UnifiedGeometryBuffer::init()
{
	const PtrSize poolSize = g_unifiedGometryBufferSizeCvar;

	const Array classes = {1_KB, 8_KB, 32_KB, 128_KB, 512_KB, 4_MB, 8_MB, 16_MB, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kVertex | BufferUsageBit::kIndex | BufferUsageBit::kCopyDestination | BufferUsageBit::kAllSrv;

	if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled)
	{
		buffUsage |= BufferUsageBit::kAccelerationStructureBuild;
	}

	m_pool.init(buffUsage, classes, poolSize, "UnifiedGeometry", false);

	// Allocate something dummy to force creating the GPU buffer
	UnifiedGeometryBufferAllocation alloc = allocate(16, 4);
	deferredFree(alloc);
}

void UnifiedGeometryBuffer::updateStats() const
{
	F32 externalFragmentation;
	PtrSize userAllocatedSize, totalSize;
	m_pool.getStats(externalFragmentation, userAllocatedSize, totalSize);

	g_unifiedGeomBufferAllocatedSizeStatVar.set(userAllocatedSize);
	g_unifiedGeomBufferTotalStatVar.set(totalSize);
	g_unifiedGeomBufferFragmentationStatVar.set(externalFragmentation);
}

} // end namespace anki

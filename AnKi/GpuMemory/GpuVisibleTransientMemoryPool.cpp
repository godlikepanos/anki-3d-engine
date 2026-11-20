// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

ANKI_SVAR(GpuMem, GpuVisibleTransientMemory, StatCategory::kGpuMem, "GPU visible transient mem", StatFlag::kBytes | StatFlag::kMainThreadUpdates)

void GpuVisibleTransientMemoryPool::endFrame()
{
	g_svarGpuMemGpuVisibleTransientMemory.set(m_pool.getAllocatedMemory());

	// This is GPU only memory so next frame can start re-using immediately
	m_pool.reset();
}

} // end namespace anki

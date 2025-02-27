// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Core/StatsSet.h>

namespace anki {

static StatCounter g_gpuVisibleTransientMemoryStatVar(StatCategory::kGpuMem, "GPU visible transient mem",
													  StatFlag::kBytes | StatFlag::kMainThreadUpdates);

void GpuVisibleTransientMemoryPool::endFrame()
{
	g_gpuVisibleTransientMemoryStatVar.set(m_pool.getAllocatedMemory());

	if(m_frame == 0)
	{
		m_pool.reset();
	}

	m_frame = (m_frame + 1) % kMaxFramesInFlight;
}

} // end namespace anki

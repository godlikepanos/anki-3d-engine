// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Utils/StackGpuMemoryPool.h>
#include <AnKi/Gr/GrManager.h>

namespace anki {

/// @addtogroup core
/// @{

/// @memberof GpuVisibleTransientMemoryPool
class GpuVisibleTransientMemoryAllocation
{
public:
	Buffer* m_buffer = nullptr;
	PtrSize m_offset = kMaxPtrSize;
};

/// GPU only transient memory. Used for temporary allocations. Allocations will get reset after each frame.
class GpuVisibleTransientMemoryPool : public MakeSingleton<GpuVisibleTransientMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuVisibleTransientMemoryAllocation allocate(PtrSize size)
	{
		GpuVisibleTransientMemoryAllocation out;
		m_pool.allocate(size, out.m_offset, out.m_buffer);
		return out;
	}

	void endFrame()
	{
		if(m_frame == 0)
		{
			m_pool.reset();
		}

		m_frame = (m_frame + 1) % kMaxFramesInFlight;
	}

private:
	StackGpuMemoryPool m_pool;
	U32 m_frame = 0;

	GpuVisibleTransientMemoryPool()
	{
		U32 alignment = GrManager::getSingleton().getDeviceCapabilities().m_uniformBufferBindOffsetAlignment;
		alignment =
			max(alignment, GrManager::getSingleton().getDeviceCapabilities().m_storageBufferBindOffsetAlignment);
		alignment = max(alignment, GrManager::getSingleton().getDeviceCapabilities().m_sbtRecordAlignment);

		const BufferUsageBit buffUsage = BufferUsageBit::kAllUniform | BufferUsageBit::kAllStorage
										 | BufferUsageBit::kIndirectDraw | BufferUsageBit::kVertex;
		m_pool.init(10_MB, 2.0, 0, alignment, buffUsage, BufferMapAccessBit::kNone, true,
					"GpuVisibleTransientMemoryPool");
	}

	~GpuVisibleTransientMemoryPool() = default;
};
/// @}

} // end namespace anki

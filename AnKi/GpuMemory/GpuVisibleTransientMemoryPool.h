// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/GpuMemory/Common.h>
#include <AnKi/GpuMemory/StackGpuMemoryPool.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

// GPU only transient memory. Used for temporary allocations. Allocations will get reset after each frame.
class GpuVisibleTransientMemoryPool : public MakeSingleton<GpuVisibleTransientMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	BufferView allocate(PtrSize size, PtrSize alignment)
	{
		return m_pool.allocate(size, alignment);
	}

	template<typename T>
	BufferView allocateStructuredBuffer(U32 count)
	{
		return m_pool.allocateStructuredBuffer<T>(count);
	}

	BufferView allocateStructuredBuffer(U32 count, U32 structureSize)
	{
		return m_pool.allocateStructuredBuffer(count, structureSize);
	}

	void endFrame();

private:
	StackGpuMemoryPool m_pool;

	GpuVisibleTransientMemoryPool()
	{
		BufferUsageBit buffUsage = BufferUsageBit::kAllConstant | BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv | BufferUsageBit::kIndirectDraw
								   | BufferUsageBit::kIndirectCompute | BufferUsageBit::kVertexOrIndex | BufferUsageBit::kAllCopy
								   | BufferUsageBit::kIndirectDispatchRays | BufferUsageBit::kShaderBindingTable;
		if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracing)
		{
			buffUsage |= (BufferUsageBit::kAccelerationStructureBuildScratch | BufferUsageBit::kAccelerationStructureBuild
						  | BufferUsageBit::kAccelerationStructure);
		}
		m_pool.init(10_MB, 2.0, 0, buffUsage, BufferMapAccessBit::kNone, true, "GpuVisibleTransientMemoryPool");
	}

	~GpuVisibleTransientMemoryPool() = default;
};
/// @}

} // end namespace anki

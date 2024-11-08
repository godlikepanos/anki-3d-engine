// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Utils/StackGpuMemoryPool.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>

namespace anki {

/// @addtogroup core
/// @{

/// GPU only transient memory. Used for temporary allocations. Allocations will get reset after each frame.
class GpuVisibleTransientMemoryPool : public MakeSingleton<GpuVisibleTransientMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	BufferView allocate(PtrSize size, PtrSize alignment)
	{
		PtrSize offset;
		Buffer* buffer;
		m_pool.allocate(size, alignment, offset, buffer);
		return BufferView(buffer, offset, size);
	}

	template<typename T>
	BufferView allocateStructuredBuffer(U32 count)
	{
		return allocateStructuredBuffer(count, sizeof(T));
	}

	BufferView allocateStructuredBuffer(U32 count, U32 structureSize)
	{
		return allocate(PtrSize(structureSize * count), (m_structuredBufferAlignment == kMaxU32) ? structureSize : m_structuredBufferAlignment);
	}

	void endFrame();

private:
	StackGpuMemoryPool m_pool;
	U32 m_frame = 0;
	U32 m_structuredBufferAlignment = kMaxU32;

	GpuVisibleTransientMemoryPool()
	{
		if(!GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferNaturalAlignment)
		{
			m_structuredBufferAlignment = GrManager::getSingleton().getDeviceCapabilities().m_structuredBufferBindOffsetAlignment;
		}

		BufferUsageBit buffUsage = BufferUsageBit::kAllConstant | BufferUsageBit::kAllUav | BufferUsageBit::kAllSrv | BufferUsageBit::kIndirectDraw
								   | BufferUsageBit::kIndirectCompute | BufferUsageBit::kVertex | BufferUsageBit::kAllCopy
								   | BufferUsageBit::kIndirectTraceRays;
		if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled)
		{
			buffUsage |= (BufferUsageBit::kAccelerationStructureBuildScratch | BufferUsageBit::kAccelerationStructureBuild);
		}
		m_pool.init(10_MB, 2.0, 0, buffUsage, BufferMapAccessBit::kNone, true, "GpuVisibleTransientMemoryPool");
	}

	~GpuVisibleTransientMemoryPool() = default;
};
/// @}

} // end namespace anki

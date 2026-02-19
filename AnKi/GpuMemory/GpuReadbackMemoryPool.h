// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Utils/SegregatedListsSingleBufferGpuMemoryPool.h>

namespace anki {

using GpuReadbackMemoryAllocation = SegregatedListsSingleBufferGpuMemoryPoolAllocation;

class GpuReadbackMemoryPool : public MakeSingleton<GpuReadbackMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	// Thread-safe
	GpuReadbackMemoryAllocation allocate(PtrSize size, U32 alignment)
	{
		return m_pool.allocate(size, alignment);
	}

	// Thread-safe
	template<typename T>
	GpuReadbackMemoryAllocation allocateStructuredBuffer(U32 count)
	{
		const U32 alignment = (m_structuredBufferAlignment == kMaxU32) ? sizeof(T) : m_structuredBufferAlignment;
		return allocate(sizeof(T) * count, alignment);
	}

	// Thread-safe
	void deferredFree(GpuReadbackMemoryAllocation& allocation)
	{
		m_pool.deferredFree(allocation);
	}

	void endFrame(Fence* fence)
	{
		m_pool.endFrame(fence);
	}

private:
	SegregatedListsSingleBufferGpuMemoryPool m_pool;
	U32 m_structuredBufferAlignment = kMaxU32;

	GpuReadbackMemoryPool();
};

} // end namespace anki

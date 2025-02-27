// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>

namespace anki {

/// @addtogroup gpu_memory
/// @{

/// @memberof GpuReadbackMemoryPool
class GpuReadbackMemoryAllocation
{
	friend class GpuReadbackMemoryPool;

public:
	GpuReadbackMemoryAllocation() = default;

	GpuReadbackMemoryAllocation(const GpuReadbackMemoryAllocation&) = delete;

	GpuReadbackMemoryAllocation(GpuReadbackMemoryAllocation&& b)
	{
		*this = std::move(b);
	}

	~GpuReadbackMemoryAllocation();

	GpuReadbackMemoryAllocation& operator=(const GpuReadbackMemoryAllocation&) = delete;

	GpuReadbackMemoryAllocation& operator=(GpuReadbackMemoryAllocation&& b)
	{
		ANKI_ASSERT(!isValid() && "Forgot to delete");
		m_token = b.m_token;
		b.m_token = {};
		m_buffer = b.m_buffer;
		m_mappedMemory = b.m_mappedMemory;
		return *this;
	}

	Bool isValid() const
	{
		return m_token.m_offset != kMaxPtrSize;
	}

	/// Get offset in the Unified Geometry Buffer buffer.
	U32 getOffset() const
	{
		ANKI_ASSERT(isValid());
		return U32(m_token.m_offset);
	}

	U32 getAllocatedSize() const
	{
		ANKI_ASSERT(isValid());
		return U32(m_token.m_size);
	}

	Buffer& getBuffer() const
	{
		ANKI_ASSERT(isValid());
		return *m_buffer;
	}

	const void* getMappedMemory() const
	{
		ANKI_ASSERT(isValid());
		return m_mappedMemory;
	}

private:
	SegregatedListsGpuMemoryPoolToken m_token;
	Buffer* m_buffer = nullptr;
	void* m_mappedMemory = nullptr;
};

class GpuReadbackMemoryPool : public MakeSingleton<GpuReadbackMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

public:
	GpuReadbackMemoryAllocation allocate(PtrSize size, U32 alignment);

	template<typename T>
	GpuReadbackMemoryAllocation allocateStructuredBuffer(U32 count)
	{
		const U32 alignment = (m_structuredBufferAlignment == kMaxU32) ? sizeof(T) : m_structuredBufferAlignment;
		return allocate(sizeof(T) * count, alignment);
	}

	void deferredFree(GpuReadbackMemoryAllocation& allocation);

	void endFrame();

private:
	SegregatedListsGpuMemoryPool m_pool;
	U32 m_structuredBufferAlignment = kMaxU32;

	GpuReadbackMemoryPool();

	~GpuReadbackMemoryPool();
};

inline GpuReadbackMemoryAllocation::~GpuReadbackMemoryAllocation()
{
	GpuReadbackMemoryPool::getSingleton().deferredFree(*this);
}
/// @}

} // end namespace anki

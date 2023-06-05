// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>

namespace anki {

/// @addtogroup core
/// @{

/// @memberof UnifiedGeometryBuffer
class UnifiedGeometryBufferAllocation
{
	friend class UnifiedGeometryBuffer;

public:
	UnifiedGeometryBufferAllocation() = default;

	UnifiedGeometryBufferAllocation(const UnifiedGeometryBufferAllocation&) = delete;

	UnifiedGeometryBufferAllocation(UnifiedGeometryBufferAllocation&& b)
	{
		*this = std::move(b);
	}

	~UnifiedGeometryBufferAllocation();

	UnifiedGeometryBufferAllocation& operator=(const UnifiedGeometryBufferAllocation&) = delete;

	UnifiedGeometryBufferAllocation& operator=(UnifiedGeometryBufferAllocation&& b)
	{
		ANKI_ASSERT(!isValid() && "Forgot to delete");
		m_token = b.m_token;
		b.m_token = {};
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

private:
	SegregatedListsGpuMemoryPoolToken m_token;
};

/// Manages vertex and index memory for the WHOLE application.
class UnifiedGeometryBuffer : public MakeSingleton<UnifiedGeometryBuffer>
{
	template<typename>
	friend class MakeSingleton;

public:
	UnifiedGeometryBuffer(const UnifiedGeometryBuffer&) = delete; // Non-copyable

	UnifiedGeometryBuffer& operator=(const UnifiedGeometryBuffer&) = delete; // Non-copyable

	void init();

	void allocate(PtrSize size, U32 alignment, UnifiedGeometryBufferAllocation& alloc)
	{
		m_pool.allocate(size, alignment, alloc.m_token);
	}

	void deferredFree(UnifiedGeometryBufferAllocation& alloc)
	{
		m_pool.deferredFree(alloc.m_token);
	}

	void endFrame()
	{
		m_pool.endFrame();
#if ANKI_STATS_ENABLED
		updateStats();
#endif
	}

	Buffer& getBuffer() const
	{
		return m_pool.getGpuBuffer();
	}

private:
	SegregatedListsGpuMemoryPool m_pool;

	UnifiedGeometryBuffer() = default;

	~UnifiedGeometryBuffer() = default;

	void updateStats() const;
};

inline UnifiedGeometryBufferAllocation::~UnifiedGeometryBufferAllocation()
{
	UnifiedGeometryBuffer::getSingleton().deferredFree(*this);
}
/// @}

} // end namespace anki

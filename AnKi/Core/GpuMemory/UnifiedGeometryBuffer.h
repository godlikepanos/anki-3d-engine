// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Gr/Utils/SegregatedListsGpuMemoryPool.h>

namespace anki {

/// @addtogroup core
/// @{

inline NumericCVar<PtrSize> g_unifiedGometryBufferSizeCvar("Core", "UnifiedGeometryBufferSize", 128_MB, 16_MB, 2_GB,
														   "Global index and vertex buffer size");

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
		m_fakeOffset = b.m_fakeOffset;
		m_fakeAllocatedSize = b.m_fakeAllocatedSize;
		b.m_token = {};
		b.m_fakeAllocatedSize = 0;
		b.m_fakeOffset = kMaxU32;
		return *this;
	}

	operator BufferView() const;

	/// This will return an exaggerated view compared to the above that it's properly aligned.
	BufferView getCompleteBufferView() const;

	Bool isValid() const
	{
		return m_token.m_offset != kMaxPtrSize;
	}

	/// Get offset in the Unified Geometry Buffer buffer.
	U32 getOffset() const
	{
		ANKI_ASSERT(isValid());
		return m_fakeOffset;
	}

	U32 getAllocatedSize() const
	{
		ANKI_ASSERT(isValid());
		return m_fakeAllocatedSize;
	}

private:
	SegregatedListsGpuMemoryPoolToken m_token;
	U32 m_fakeOffset = kMaxU32; ///< In some allocations with weird alignments we need a different offset.
	U32 m_fakeAllocatedSize = 0;
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

	/// The alignment doesn't need to be power of 2 unlike other allocators.
	UnifiedGeometryBufferAllocation allocate(PtrSize size, U32 alignment)
	{
		ANKI_ASSERT(size && alignment);

		const U32 fixedAlignment = max(4u, nextPowerOfTwo(alignment)); // Fix the alignment and make sure it's at least 4
		const U32 fixedSize = getAlignedRoundUp(4u, U32(size) + alignment); // Over-allocate and align to 4 because some cmd buffer ops need it
		ANKI_ASSERT(fixedSize >= size);

		UnifiedGeometryBufferAllocation out;
		m_pool.allocate(fixedSize, fixedAlignment, out.m_token);

		const U32 remainder = out.m_token.m_offset % alignment;
		out.m_fakeOffset = U32(out.m_token.m_offset + (alignment - remainder));
		ANKI_ASSERT(isAligned(alignment, out.m_fakeOffset));

		out.m_fakeAllocatedSize = U32(size);
		ANKI_ASSERT(PtrSize(out.m_fakeOffset) + out.m_fakeAllocatedSize <= out.m_token.m_offset + out.m_token.m_size);

		return out;
	}

	/// Allocate a vertex buffer.
	UnifiedGeometryBufferAllocation allocateFormat(Format format, U32 count)
	{
		const U32 texelSize = getFormatInfo(format).m_texelSize;
		return allocate(texelSize * count, texelSize);
	}

	void deferredFree(UnifiedGeometryBufferAllocation& alloc)
	{
		m_pool.deferredFree(alloc.m_token);
		alloc.m_fakeAllocatedSize = 0;
		alloc.m_fakeOffset = kMaxU32;
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

	BufferView getBufferView() const
	{
		return BufferView(&m_pool.getGpuBuffer());
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

inline UnifiedGeometryBufferAllocation::operator BufferView() const
{
	return {&UnifiedGeometryBuffer::getSingleton().getBuffer(), getOffset(), getAllocatedSize()};
}

inline BufferView UnifiedGeometryBufferAllocation::getCompleteBufferView() const
{
	return {&UnifiedGeometryBuffer::getSingleton().getBuffer(), m_token.m_offset, m_token.m_size};
}
/// @}

} // end namespace anki

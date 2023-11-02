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
		m_realOffset = b.m_realOffset;
		m_realAllocatedSize = b.m_realAllocatedSize;
		b.m_token = {};
		b.m_realAllocatedSize = 0;
		b.m_realOffset = kMaxU32;
		return *this;
	}

	operator BufferOffsetRange() const;

	Bool isValid() const
	{
		return m_token.m_offset != kMaxPtrSize;
	}

	/// Get offset in the Unified Geometry Buffer buffer.
	U32 getOffset() const
	{
		ANKI_ASSERT(isValid());
		return m_realOffset;
	}

	U32 getAllocatedSize() const
	{
		ANKI_ASSERT(isValid());
		return m_realAllocatedSize;
	}

private:
	SegregatedListsGpuMemoryPoolToken m_token;
	U32 m_realOffset = kMaxU32; ///< In some allocations with weird alignments we need a different offset.
	U32 m_realAllocatedSize = 0;
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

	UnifiedGeometryBufferAllocation allocate(PtrSize size, U32 alignment)
	{
		UnifiedGeometryBufferAllocation out;
		m_pool.allocate(size, alignment, out.m_token);
		out.m_realOffset = U32(out.m_token.m_offset);
		out.m_realAllocatedSize = U32(size);
		return out;
	}

	/// Allocate a vertex buffer.
	UnifiedGeometryBufferAllocation allocateFormat(Format format, U32 count)
	{
		const U32 texelSize = getFormatInfo(format).m_texelSize;
		const U32 alignment = max(4u, nextPowerOfTwo(texelSize));
		const U32 size = count * texelSize + alignment; // Over-allocate

		UnifiedGeometryBufferAllocation out;
		m_pool.allocate(size, alignment, out.m_token);

		const U32 remainder = out.m_token.m_offset % texelSize;
		out.m_realOffset = U32(out.m_token.m_offset + (texelSize - remainder));
		out.m_realAllocatedSize = count * texelSize;
		ANKI_ASSERT(isAligned(texelSize, out.m_realOffset));
		ANKI_ASSERT(out.m_realOffset + out.m_realAllocatedSize <= out.m_token.m_offset + out.m_token.m_size);
		return out;
	}

	void deferredFree(UnifiedGeometryBufferAllocation& alloc)
	{
		m_pool.deferredFree(alloc.m_token);
		alloc.m_realAllocatedSize = 0;
		alloc.m_realOffset = kMaxU32;
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

	BufferOffsetRange getBufferOffsetRange() const
	{
		return {&m_pool.getGpuBuffer(), 0, kMaxPtrSize};
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

inline UnifiedGeometryBufferAllocation::operator BufferOffsetRange() const
{
	return {&UnifiedGeometryBuffer::getSingleton().getBuffer(), getOffset(), getAllocatedSize()};
}
/// @}

} // end namespace anki

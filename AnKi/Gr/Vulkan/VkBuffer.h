// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Vulkan/VkGpuMemoryManager.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

// Buffer implementation
class BufferImpl final : public Buffer
{
	friend class Buffer;

public:
	BufferImpl(CString name)
		: Buffer(name)
	{
	}

	~BufferImpl();

	Error init(const BufferInitInfo& inf);

	VkBuffer getHandle() const
	{
		ANKI_ASSERT(isCreated());
		return m_handle;
	}

	Bool usageValid(BufferUsageBit usage) const
	{
		return (m_usage & usage) == usage;
	}

	PtrSize getActualSize() const
	{
		ANKI_ASSERT(m_actualSize > 0);
		return m_actualSize;
	}

	VkBufferMemoryBarrier computeBarrierInfo(BufferUsageBit before, BufferUsageBit after, VkPipelineStageFlags& srcStages,
											 VkPipelineStageFlags& dstStages) const;

	// Only for texture buffers.
	// It's thread-safe
	VkBufferView getOrCreateBufferView(Format fmt, PtrSize offset, PtrSize range) const;

	const GpuMemoryHandle& getGpuMemoryHandle() const
	{
		return m_memHandle;
	}

private:
	VkBuffer m_handle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memHandle;
	VkMemoryPropertyFlags m_memoryFlags = 0;
	PtrSize m_actualSize = 0;
	PtrSize m_mappedMemoryRangeAlignment = 0; ///< Cache this value.
	Bool m_needsFlush : 1 = false;
	Bool m_needsInvalidate : 1 = false;

	mutable GrHashMap<U64, VkBufferView> m_views; ///< Only for texture buffers.
	mutable RWMutex m_viewsMtx;

#if ANKI_ASSERTIONS_ENABLED
	Bool m_mapped = false;
	mutable Atomic<U32> m_flushCount = {0};
	mutable Atomic<U32> m_invalidateCount = {0};
#endif

	Bool isCreated() const
	{
		return m_handle != VK_NULL_HANDLE;
	}

	static VkPipelineStageFlags computePplineStage(BufferUsageBit usage);
	static VkAccessFlags computeAccessMask(BufferUsageBit usage);

	ANKI_FORCE_INLINE VkMappedMemoryRange setVkMappedMemoryRange(PtrSize offset, PtrSize range) const
	{
		// First the offset
		ANKI_ASSERT(offset < m_size);
		offset += m_memHandle.m_offset; // Move from buffer offset to memory offset
		alignRoundDown(m_mappedMemoryRangeAlignment, offset);

		// And the range
		range = (range == kMaxPtrSize) ? m_actualSize : range;
		alignRoundUp(m_mappedMemoryRangeAlignment, range);
		ANKI_ASSERT(offset + range <= m_memHandle.m_offset + m_actualSize);

		VkMappedMemoryRange vkrange = {};
		vkrange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		vkrange.memory = m_memHandle.m_memory;
		vkrange.offset = offset;
		vkrange.size = range;

		return vkrange;
	}
};

} // end namespace anki

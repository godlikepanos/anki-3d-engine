// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/GpuMemoryManager.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class BufferImpl final : public Buffer, public VulkanObject<Buffer, BufferImpl>
{
public:
	BufferImpl(GrManager* manager, CString name)
		: Buffer(manager, name)
		, m_needsFlush(false)
		, m_needsInvalidate(false)
	{
	}

	~BufferImpl();

	Error init(const BufferInitInfo& inf);

	[[nodiscard]] void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access);

	void unmap()
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(m_mapped);

#if ANKI_EXTRA_CHECKS
		m_mapped = false;
#endif
	}

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

	void computeBarrierInfo(BufferUsageBit before, BufferUsageBit after, VkPipelineStageFlags& srcStages,
							VkAccessFlags& srcAccesses, VkPipelineStageFlags& dstStages,
							VkAccessFlags& dstAccesses) const;

	ANKI_FORCE_INLINE void flush(PtrSize offset, PtrSize range) const
	{
		ANKI_ASSERT(!!(m_access & BufferMapAccessBit::kWrite) && "No need to flush when the CPU doesn't write");
		if(m_needsFlush)
		{
			VkMappedMemoryRange vkrange = setVkMappedMemoryRange(offset, range);
			ANKI_VK_CHECKF(vkFlushMappedMemoryRanges(getDevice(), 1, &vkrange));
#if ANKI_EXTRA_CHECKS
			m_flushCount.fetchAdd(1);
#endif
		}
	}

	ANKI_FORCE_INLINE void invalidate(PtrSize offset, PtrSize range) const
	{
		ANKI_ASSERT(!!(m_access & BufferMapAccessBit::kRead) && "No need to invalidate when the CPU doesn't read");
		if(m_needsInvalidate)
		{
			VkMappedMemoryRange vkrange = setVkMappedMemoryRange(offset, range);
			ANKI_VK_CHECKF(vkInvalidateMappedMemoryRanges(getDevice(), 1, &vkrange));
#if ANKI_EXTRA_CHECKS
			m_invalidateCount.fetchAdd(1);
#endif
		}
	}

	/// Only for texture buffers.
	/// @note It's thread-safe
	VkBufferView getOrCreateBufferView(Format fmt, PtrSize offset, PtrSize range) const;

private:
	VkBuffer m_handle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memHandle;
	VkMemoryPropertyFlags m_memoryFlags = 0;
	PtrSize m_actualSize = 0;
	PtrSize m_mappedMemoryRangeAlignment = 0; ///< Cache this value.
	Bool m_needsFlush : 1;
	Bool m_needsInvalidate : 1;

	mutable HashMap<U64, VkBufferView> m_views; ///< Only for texture buffers.
	mutable RWMutex m_viewsMtx;

#if ANKI_EXTRA_CHECKS
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
/// @}

} // end namespace anki

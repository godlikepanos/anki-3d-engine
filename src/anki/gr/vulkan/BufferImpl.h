// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class BufferImpl : public VulkanObject
{
public:
	BufferImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~BufferImpl();

	ANKI_USE_RESULT Error init(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);

	ANKI_USE_RESULT void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access);

	void unmap()
	{
		ANKI_ASSERT(isCreated());
		ANKI_ASSERT(m_mapped);

#if ANKI_EXTRA_CHECKS
		m_mapped = false;
#endif
		// TODO Flush or invalidate caches
	}

	VkBuffer getHandle() const
	{
		ANKI_ASSERT(isCreated());
		return m_handle;
	}

	PtrSize getSize() const
	{
		ANKI_ASSERT(m_size);
		return m_size;
	}

	Bool usageValid(BufferUsageBit usage) const
	{
		return (m_usage & usage) == usage;
	}

	void computeBarrierInfo(BufferUsageBit before,
		BufferUsageBit after,
		VkPipelineStageFlags& srcStages,
		VkAccessFlags& srcAccesses,
		VkPipelineStageFlags& dstStages,
		VkAccessFlags& dstAccesses) const;

private:
	VkBuffer m_handle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memHandle;
	BufferMapAccessBit m_access = BufferMapAccessBit::NONE;
	U32 m_size = 0;
	VkMemoryPropertyFlags m_memoryFlags = 0;
	BufferUsageBit m_usage = BufferUsageBit::NONE;

#if ANKI_EXTRA_CHECKS
	Bool8 m_mapped = false;
#endif

	Bool isCreated() const
	{
		return m_handle != VK_NULL_HANDLE;
	}

	static VkPipelineStageFlags computePplineStage(BufferUsageBit usage);
	static VkAccessFlags computeAccessMask(BufferUsageBit usage);
};
/// @}

} // end namespace anki

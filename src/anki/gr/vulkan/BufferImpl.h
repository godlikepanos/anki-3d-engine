// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Buffer.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Buffer implementation
class BufferImpl final : public Buffer, public VulkanObject<Buffer, BufferImpl>
{
public:
	BufferImpl(GrManager* manager, CString name)
		: Buffer(manager, name)
	{
	}

	~BufferImpl();

	ANKI_USE_RESULT Error init(const BufferInitInfo& inf);

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

	Bool usageValid(BufferUsageBit usage) const
	{
		return (m_usage & usage) == usage;
	}

	PtrSize getActualSize() const
	{
		ANKI_ASSERT(m_actualSize > 0);
		return m_actualSize;
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
	VkMemoryPropertyFlags m_memoryFlags = 0;
	PtrSize m_actualSize = 0;

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

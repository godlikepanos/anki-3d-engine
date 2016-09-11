// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Allocator of descriptor sets.
class DescriptorSetAllocator : public NonCopyable
{
public:
	DescriptorSetAllocator()
	{
	}

	~DescriptorSetAllocator()
	{
		ANKI_ASSERT(m_globalDPool == VK_NULL_HANDLE);
		ANKI_ASSERT(m_globalDsetLayout == VK_NULL_HANDLE);
	}

	ANKI_USE_RESULT Error init(VkDevice dev);

	void destroy();

	ANKI_USE_RESULT Error allocate(VkDescriptorSet& out)
	{
		out = VK_NULL_HANDLE;
		VkDescriptorSetAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ci.descriptorPool = m_globalDPool;
		ci.descriptorSetCount = 1;
		ci.pSetLayouts = &m_globalDsetLayout;

		LockGuard<Mutex> lock(m_mtx);
		if(++m_descriptorSetAllocationCount > MAX_RESOURCE_GROUPS)
		{
			ANKI_LOGE("Exceeded the MAX_RESOURCE_GROUPS");
			return ErrorCode::OUT_OF_MEMORY;
		}
		ANKI_VK_CHECK(vkAllocateDescriptorSets(m_dev, &ci, &out));
		return ErrorCode::NONE;
	}

	void free(VkDescriptorSet ds)
	{
		ANKI_ASSERT(ds);
		LockGuard<Mutex> lock(m_mtx);
		ANKI_ASSERT(m_descriptorSetAllocationCount > 0);
		--m_descriptorSetAllocationCount;
		ANKI_VK_CHECKF(vkFreeDescriptorSets(m_dev, m_globalDPool, 1, &ds));
	}

	VkDescriptorSetLayout getGlobalDescriptorSetLayout() const
	{
		ANKI_ASSERT(m_globalDsetLayout);
		return m_globalDsetLayout;
	}

private:
	VkDevice m_dev = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_globalDsetLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_globalDPool = VK_NULL_HANDLE;
	Mutex m_mtx;
	U32 m_descriptorSetAllocationCount = 0;

	Error initGlobalDsetLayout();
	Error initGlobalDsetPool();
};
/// @}

} // end namespace anki

// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineLayout.h>

namespace anki
{

void PipelineLayoutFactory::destroy()
{
	while(!m_layouts.isEmpty())
	{
		auto it = m_layouts.getBegin();
		VkPipelineLayout handle = *it;
		m_layouts.erase(m_alloc, it);

		vkDestroyPipelineLayout(m_dev, handle, nullptr);
	}
}

Error PipelineLayoutFactory::newPipelineLayout(const WeakArray<DescriptorSetLayout>& dsetLayouts, U32 pushConstantsSize,
											   PipelineLayout& layout)
{
	U64 hash = computeHash(&pushConstantsSize, sizeof(pushConstantsSize));
	Array<VkDescriptorSetLayout, MAX_DESCRIPTOR_SETS> vkDsetLayouts;
	U32 dsetLayoutCount = 0;
	for(const DescriptorSetLayout& dl : dsetLayouts)
	{
		vkDsetLayouts[dsetLayoutCount++] = dl.getHandle();
	}

	if(dsetLayoutCount > 0)
	{
		hash = appendHash(&vkDsetLayouts[0], sizeof(vkDsetLayouts[0]) * dsetLayoutCount, hash);
	}

	LockGuard<Mutex> lock(m_layoutsMtx);

	auto it = m_layouts.find(hash);
	if(it != m_layouts.getEnd())
	{
		// Found it

		layout.m_handle = *it;
	}
	else
	{
		// Not found, create new

		VkPipelineLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ci.pSetLayouts = &vkDsetLayouts[0];
		ci.setLayoutCount = dsetLayoutCount;

		VkPushConstantRange pushConstantRange;
		if(pushConstantsSize > 0)
		{
			pushConstantRange.offset = 0;
			pushConstantRange.size = pushConstantsSize;
			pushConstantRange.stageFlags = VK_SHADER_STAGE_ALL;
			ci.pushConstantRangeCount = 1;
			ci.pPushConstantRanges = &pushConstantRange;
		}

		VkPipelineLayout pplineLayHandle;
		ANKI_VK_CHECK(vkCreatePipelineLayout(m_dev, &ci, nullptr, &pplineLayHandle));

		m_layouts.emplace(m_alloc, hash, pplineLayHandle);

		layout.m_handle = pplineLayHandle;
	}

	return Error::NONE;
}

} // end namespace anki

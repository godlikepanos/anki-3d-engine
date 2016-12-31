// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineLayout.h>

namespace anki
{

class PipelineLayoutFactory::Layout
{
public:
	U64 m_hash;
	VkPipelineLayout m_handle;
};

class PipelineLayoutFactory::Hasher
{
public:
	U64 operator()(U64 k) const
	{
		return k;
	}
};

void PipelineLayoutFactory::destroy()
{
	for(auto it : m_layouts)
	{
		Layout* l = it;
		vkDestroyPipelineLayout(m_dev, l->m_handle, nullptr);
		m_alloc.deleteInstance(l);
	}

	m_layouts.destroy(m_alloc);
}

void PipelineLayoutFactory::newPipelineLayout(const WeakArray<DescriptorSetLayout>& dsetLayouts, PipelineLayout& layout)
{
	U64 hash = 1;
	Array<VkDescriptorSetLayout, MAX_DESCRIPTOR_SETS> vkDsetLayouts;
	U dsetLayoutCount = 0;
	for(const DescriptorSetLayout& dl : dsetLayouts)
	{
		vkDsetLayouts[dsetLayoutCount++] = dl.getHandle();
	}

	if(dsetLayoutCount > 0)
	{
		hash = computeHash(&vkDsetLayouts[0], sizeof(vkDsetLayouts[0]) * dsetLayoutCount);
	}

	LockGuard<Mutex> lock(m_layoutsMtx);

	auto it = m_layouts.find(hash);
	if(it != m_layouts.getEnd())
	{
		// Found it

		layout.m_handle = (*it)->m_handle;
	}
	else
	{
		// Not found, create new

		Layout* lay = m_alloc.newInstance<Layout>();
		lay->m_hash = hash;

		VkPipelineLayoutCreateInfo ci = {
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		};
		ci.pSetLayouts = &vkDsetLayouts[0];
		ci.setLayoutCount = dsetLayoutCount;

		ANKI_VK_CHECKF(vkCreatePipelineLayout(m_dev, &ci, nullptr, &lay->m_handle));

		layout.m_handle = lay->m_handle;
	}
}

} // end namespace anki

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineLayout.h>

namespace anki
{

class PipelineLayoutFactory::Layout : public IntrusiveHashMapEnabled<PipelineLayoutFactory::Layout>
{
public:
	VkPipelineLayout m_handle;
};

void PipelineLayoutFactory::destroy()
{
	while(!m_layouts.isEmpty())
	{
		Layout* l = &(*m_layouts.getBegin());
		m_layouts.erase(l);

		vkDestroyPipelineLayout(m_dev, l->m_handle, nullptr);
		m_alloc.deleteInstance(l);
	}
}

Error PipelineLayoutFactory::newPipelineLayout(
	const WeakArray<DescriptorSetLayout>& dsetLayouts, PipelineLayout& layout)
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

		layout.m_handle = (*it).m_handle;
	}
	else
	{
		// Not found, create new

		Layout* lay = m_alloc.newInstance<Layout>();

		VkPipelineLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		ci.pSetLayouts = &vkDsetLayouts[0];
		ci.setLayoutCount = dsetLayoutCount;

		ANKI_VK_CHECK(vkCreatePipelineLayout(m_dev, &ci, nullptr, &lay->m_handle));
		m_layouts.pushBack(hash, lay);

		layout.m_handle = lay->m_handle;
	}

	return ErrorCode::NONE;
}

} // end namespace anki

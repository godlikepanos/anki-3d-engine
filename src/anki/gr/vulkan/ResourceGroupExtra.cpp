// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ResourceGroupExtra.h>

namespace anki
{

void DescriptorSetLayoutFactory::destroy()
{
	for(auto it : m_map)
	{
		VkDescriptorSetLayout dset = it;
		vkDestroyDescriptorSetLayout(m_dev, dset, nullptr);
	}

	m_map.destroy(m_alloc);
}

Error DescriptorSetLayoutFactory::getOrCreateLayout(const DescriptorSetLayoutInfo& dsinf, VkDescriptorSetLayout& out)
{
	out = VK_NULL_HANDLE;
	LockGuard<Mutex> lock(m_mtx);

	auto it = m_map.find(dsinf);

	if(it != m_map.getEnd())
	{
		out = *it;
	}
	else
	{
		// Create the layout

		VkDescriptorSetLayoutCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		const U BINDING_COUNT =
			MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS + MAX_IMAGE_BINDINGS;

		Array<VkDescriptorSetLayoutBinding, BINDING_COUNT> bindings;
		memset(&bindings[0], 0, sizeof(bindings));
		ci.pBindings = &bindings[0];

		U count = 0;
		U bindingIdx = 0;

		// Combined image samplers
		for(U i = 0; i < dsinf.m_texCount; ++i)
		{
			VkDescriptorSetLayoutBinding& binding = bindings[count++];
			binding.binding = bindingIdx++;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
		}

		// Uniform buffers
		bindingIdx = MAX_TEXTURE_BINDINGS;
		for(U i = 0; i < dsinf.m_uniCount; ++i)
		{
			VkDescriptorSetLayoutBinding& binding = bindings[count++];
			binding.binding = bindingIdx++;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
		}

		// Storage buffers
		bindingIdx = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS;
		for(U i = 0; i < dsinf.m_storageCount; ++i)
		{
			VkDescriptorSetLayoutBinding& binding = bindings[count++];
			binding.binding = bindingIdx++;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
		}

		// Images
		bindingIdx = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS;
		for(U i = 0; i < dsinf.m_imgCount; ++i)
		{
			VkDescriptorSetLayoutBinding& binding = bindings[count++];
			binding.binding = bindingIdx++;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		}

		ANKI_ASSERT(count <= BINDING_COUNT);
		ci.bindingCount = count;

		ANKI_VK_CHECK(vkCreateDescriptorSetLayout(m_dev, &ci, nullptr, &out));

		m_map.pushBack(m_alloc, dsinf, out);
	}

	return ErrorCode::NONE;
}

Error DescriptorSetAllocator::init(GrAllocator<U8> alloc, VkDevice dev)
{
	ANKI_ASSERT(dev);
	m_dev = dev;
	ANKI_CHECK(initGlobalDsetPool());
	m_layoutFactory.init(alloc, dev);

	return ErrorCode::NONE;
}

void DescriptorSetAllocator::destroy()
{
	if(m_globalDPool)
	{
		vkDestroyDescriptorPool(m_dev, m_globalDPool, nullptr);
		m_globalDPool = VK_NULL_HANDLE;
	}

	m_layoutFactory.destroy();
}

Error DescriptorSetAllocator::initGlobalDsetPool()
{
	Array<VkDescriptorPoolSize, 4> pools = {{}};
	pools[0] =
		VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURE_BINDINGS * MAX_RESOURCE_GROUPS};
	pools[1] = VkDescriptorPoolSize{
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_UNIFORM_BUFFER_BINDINGS * MAX_RESOURCE_GROUPS};
	pools[2] = VkDescriptorPoolSize{
		VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_STORAGE_BUFFER_BINDINGS * MAX_RESOURCE_GROUPS};
	pools[3] = VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_IMAGE_BINDINGS * MAX_RESOURCE_GROUPS};

	VkDescriptorPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	ci.maxSets = MAX_RESOURCE_GROUPS;
	ci.poolSizeCount = pools.getSize();
	ci.pPoolSizes = &pools[0];

	ANKI_VK_CHECK(vkCreateDescriptorPool(m_dev, &ci, nullptr, &m_globalDPool));

	return ErrorCode::NONE;
}

Error DescriptorSetAllocator::allocate(const DescriptorSetLayoutInfo& dsinf, VkDescriptorSet& out)
{
	VkDescriptorSetLayout layout;
	ANKI_CHECK(m_layoutFactory.getOrCreateLayout(dsinf, layout));

	out = VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ci.descriptorPool = m_globalDPool;
	ci.descriptorSetCount = 1;
	ci.pSetLayouts = &layout;

	LockGuard<Mutex> lock(m_mtx);
	if(++m_descriptorSetAllocationCount > MAX_RESOURCE_GROUPS)
	{
		ANKI_LOGE("Exceeded the MAX_RESOURCE_GROUPS");
		return ErrorCode::OUT_OF_MEMORY;
	}
	ANKI_VK_CHECK(vkAllocateDescriptorSets(m_dev, &ci, &out));
	return ErrorCode::NONE;
}

} // end namespace anki

// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ResourceGroupExtra.h>

namespace anki
{

Error DescriptorSetAllocator::init(VkDevice dev)
{
	ANKI_ASSERT(dev);
	m_dev = dev;
	ANKI_CHECK(initGlobalDsetLayout());
	ANKI_CHECK(initGlobalDsetPool());
	return ErrorCode::NONE;
}

void DescriptorSetAllocator::destroy()
{
	if(m_globalDPool)
	{
		vkDestroyDescriptorPool(m_dev, m_globalDPool, nullptr);
		m_globalDPool = VK_NULL_HANDLE;
	}

	if(m_globalDsetLayout)
	{
		vkDestroyDescriptorSetLayout(m_dev, m_globalDsetLayout, nullptr);
		m_globalDsetLayout = VK_NULL_HANDLE;
	}
}

Error DescriptorSetAllocator::initGlobalDsetLayout()
{
	VkDescriptorSetLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

	const U BINDING_COUNT =
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS + MAX_IMAGE_BINDINGS;
	ci.bindingCount = BINDING_COUNT;

	Array<VkDescriptorSetLayoutBinding, BINDING_COUNT> bindings;
	memset(&bindings[0], 0, sizeof(bindings));
	ci.pBindings = &bindings[0];

	U count = 0;

	// Combined image samplers
	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL;

		++count;
	}

	// Uniform buffers
	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL;

		++count;
	}

	// Storage buffers
	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL;

		++count;
	}

	// Images
	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		VkDescriptorSetLayoutBinding& binding = bindings[count];
		binding.binding = count;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		++count;
	}

	ANKI_ASSERT(count == BINDING_COUNT);

	ANKI_VK_CHECK(vkCreateDescriptorSetLayout(m_dev, &ci, nullptr, &m_globalDsetLayout));

	return ErrorCode::NONE;
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

} // end namespace anki

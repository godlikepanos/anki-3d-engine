// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/DescriptorSet.h>
#include <algorithm>

namespace anki
{

/// Cache entry. It's built around a specific descriptor set layout.
class DescriptorSetFactory::Cache
{
public:
	U64 m_hash;
	VkDescriptorSetLayout m_layoutHandle;
	VkDescriptorPool m_poolHandle;
	Mutex m_mtx;
};

DescriptorSetFactory::~DescriptorSetFactory()
{
}

void DescriptorSetFactory::init(const GrAllocator<U8>& alloc, VkDevice dev)
{
	m_alloc = alloc;
	m_dev = dev;
}

void DescriptorSetFactory::newDescriptorSetLayout(const DescriptorSetLayoutInitInfo& init, DescriptorSetLayout& layout)
{
	// Compute the hash for the layout
	Array<DescriptorBinding, MAX_BINDINGS_PER_DESCRIPTOR_SET> bindings;
	U bindingCount = init.m_bindings.getSize();
	U64 hash;

	if(init.m_bindings.getSize() > 0)
	{
		memcpy(&bindings[0], &init.m_bindings[0], init.m_bindings.getSizeInBytes());
		std::sort(&bindings[0],
			&bindings[0] + bindingCount,
			[](const DescriptorBinding& a, const DescriptorBinding& b) { return a.m_binding < b.m_binding; });

		hash = computeHash(&bindings[0], init.m_bindings.getSizeInBytes());
		ANKI_ASSERT(hash != 1);
	}
	else
	{
		hash = 1;
	}

	// Find or create the cache entry
	LockGuard<Mutex> lock(m_mtx);

	Cache* cache = nullptr;
	U cacheIdx = MAX_U;
	U count = 0;
	for(Cache* it : m_caches)
	{
		if(it->m_hash == hash)
		{
			cache = it;
			cacheIdx = count;
			break;
		}
		++count;
	}

	if(cache == nullptr)
	{
		cache = newCacheEntry(&bindings[0], bindingCount, hash);
		m_caches.resize(m_alloc, m_caches.getSize() + 1);
		m_caches[m_caches.getSize() - 1] = cache;
		cacheIdx = m_caches.getSize() - 1;
	}

	// Set the layout
	layout.m_handle = cache->m_layoutHandle;
	layout.m_cacheEntryIdx = cacheIdx;
}

DescriptorSetFactory::Cache* DescriptorSetFactory::newCacheEntry(
	const DescriptorBinding* bindings, U bindingCount, U64 hash)
{
	ANKI_ASSERT(bindings);

	// Create
	Cache* cache = m_alloc.newInstance<Cache>();
	cache->m_hash = hash;

	// Create the VK layout
	Array<VkDescriptorSetLayoutBinding, MAX_BINDINGS_PER_DESCRIPTOR_SET> vkBindings;
	VkDescriptorSetLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

	for(U i = 0; i < bindingCount; ++i)
	{
		VkDescriptorSetLayoutBinding& vk = vkBindings[i];
		const DescriptorBinding& ak = bindings[i];

		vk.binding = ak.m_binding;
		vk.descriptorCount = 1;
		vk.descriptorType = static_cast<VkDescriptorType>(ak.m_type);
		vk.pImmutableSamplers = nullptr;
		vk.stageFlags = convertShaderTypeBit(ak.m_stageMask);
	}

	ci.bindingCount = bindingCount;
	ci.pBindings = &vkBindings[0];

	ANKI_VK_CHECKF(vkCreateDescriptorSetLayout(m_dev, &ci, nullptr, &cache->m_layoutHandle));

	// Create the pool
	Array<VkDescriptorPoolSize, DESCRIPTOR_TYPE_COUNT> poolSizes;
	U poolSizeCount = 0;
	for(U i = 0; i < bindingCount; ++i)
	{
		U j;
		for(j = 0; j < poolSizeCount; ++j)
		{
			if(poolSizes[j].type == bindings[i].m_type)
			{
				++poolSizes[j].descriptorCount;
				break;
			}
		}

		if(j == poolSizeCount)
		{
			poolSizes[poolSizeCount].type = static_cast<VkDescriptorType>(bindings[i].m_type);
			poolSizes[poolSizeCount].descriptorCount = 1;
			++poolSizeCount;
		}
	}

	ANKI_ASSERT(poolSizeCount > 0);

	for(U i = 0; i < poolSizeCount; ++i)
	{
		poolSizes[i].descriptorCount *= MAX_DESCRIPTOR_SETS_PER_POOL;
	}

	VkDescriptorPoolCreateInfo poolci = {};
	poolci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolci.maxSets = MAX_DESCRIPTOR_SETS_PER_POOL;
	poolci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolci.poolSizeCount = poolSizeCount;
	poolci.pPoolSizes = &poolSizes[0];

	ANKI_VK_CHECKF(vkCreateDescriptorPool(m_dev, &poolci, nullptr, &cache->m_poolHandle));

	return cache;
}

} // end namespace anki

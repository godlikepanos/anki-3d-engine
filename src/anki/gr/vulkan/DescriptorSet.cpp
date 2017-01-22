// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/DescriptorSet.h>
#include <anki/util/List.h>
#include <anki/util/HashMap.h>
#include <algorithm>

namespace anki
{

/// Descriptor set internal class.
class DS : public IntrusiveListEnabled<DS>, public IntrusiveHashMapEnabled<DS>
{
public:
	VkDescriptorSet m_handle = {};
	U64 m_lastFrameUsed = MAX_U64;
};

class DSThreadAllocator
{
public:
	const DSLayoutCacheEntry* m_layoutEntry; ///< Know your father.

	ThreadId m_tid;
	DynamicArray<VkDescriptorPool> m_pools;
	U32 m_lastPoolDSCount = 0;
	U32 m_lastPoolFreeDSCount = 0;

	IntrusiveList<DS> m_list; ///< At the left of the list are the least used sets.
	IntrusiveHashMap<U64, DS> m_hashmap;

	DSThreadAllocator(const DSLayoutCacheEntry* layout, ThreadId tid)
		: m_layoutEntry(layout)
		, m_tid(tid)
	{
		ANKI_ASSERT(m_layoutEntry);
	}

	~DSThreadAllocator();

	ANKI_USE_RESULT Error init();
	ANKI_USE_RESULT Error createNewPool();

	ANKI_USE_RESULT Error getOrCreateSet(U64 hash, const DSWriteInfo& inf, DS*& out)
	{
		out = tryFindSet(hash);
		if(out == nullptr)
		{
			ANKI_CHECK(newSet(hash, inf, out));
		}

		return ErrorCode::NONE;
	}

private:
	ANKI_USE_RESULT DS* tryFindSet(U64 hash);
	ANKI_USE_RESULT Error newSet(U64 hash, const DSWriteInfo& inf, DS*& out);
	void writeSet(const DSWriteInfo& inf, DS& set);
};

/// Cache entry. It's built around a specific descriptor set layout.
class DSLayoutCacheEntry
{
public:
	DescriptorSetFactory* m_factory;

	U64 m_hash = 0; ///< Layout hash.
	VkDescriptorSetLayout m_layoutHandle = {};

	// Cache the create info
	Array<VkDescriptorPoolSize, DESCRIPTOR_TYPE_COUNT> m_poolSizesCreateInf = {};
	VkDescriptorPoolCreateInfo m_poolCreateInf = {};

	DynamicArray<DSThreadAllocator*> m_threadCaches;
	Mutex m_threadCachesMtx;

	DSLayoutCacheEntry(DescriptorSetFactory* factory)
		: m_factory(factory)
	{
	}

	~DSLayoutCacheEntry();

	ANKI_USE_RESULT Error init(const DescriptorBinding* bindings, U bindingCount, U64 hash);
};

Error DSThreadAllocator::init()
{
	ANKI_CHECK(createNewPool());
	return ErrorCode::NONE;
}

Error DSThreadAllocator::createNewPool()
{
	m_lastPoolDSCount =
		(m_lastPoolDSCount != 0) ? (m_lastPoolDSCount * DESCRIPTOR_POOL_SIZE_SCALE) : DESCRIPTOR_POOL_INITIAL_SIZE;
	m_lastPoolFreeDSCount = m_lastPoolDSCount;

	// Set the create info
	Array<VkDescriptorPoolSize, DESCRIPTOR_TYPE_COUNT> poolSizes;
	memcpy(&poolSizes[0],
		&m_layoutEntry->m_poolSizesCreateInf[0],
		sizeof(poolSizes[0]) * m_layoutEntry->m_poolCreateInf.poolSizeCount);

	for(U i = 0; i < m_layoutEntry->m_poolCreateInf.poolSizeCount; ++i)
	{
		poolSizes[i].descriptorCount *= m_lastPoolDSCount;
		ANKI_ASSERT(poolSizes[i].descriptorCount > 0);
	}

	VkDescriptorPoolCreateInfo ci = m_layoutEntry->m_poolCreateInf;
	ci.pPoolSizes = &poolSizes[0];
	ci.maxSets = m_lastPoolDSCount;

	// Create
	VkDescriptorPool pool;
	ANKI_VK_CHECK(vkCreateDescriptorPool(m_layoutEntry->m_factory->m_dev, &ci, nullptr, &pool));

	// Push back
	m_pools.resize(m_layoutEntry->m_factory->m_alloc, m_pools.getSize() + 1);
	m_pools[m_pools.getSize() - 1] = pool;

	return ErrorCode::NONE;
}

DS* DSThreadAllocator::tryFindSet(U64 hash)
{
	ANKI_ASSERT(hash > 0);

	auto it = m_hashmap.find(hash);
	if(it != m_hashmap.getEnd())
	{
		return nullptr;
	}
	else
	{
		DS* ds = &(*it);

		// Remove from the list and place at the end of the list
		m_list.erase(ds);
		m_list.pushBack(ds);
		ds->m_lastFrameUsed = m_layoutEntry->m_factory->m_frameCount;

		return ds;
	}
}

Error DSThreadAllocator::newSet(U64 hash, const DSWriteInfo& inf, DS*& out)
{
	out = nullptr;

	// First try to see if there are unused to recycle
	const U64 crntFrame = m_layoutEntry->m_factory->m_frameCount;
	auto it = m_list.getBegin();
	const auto end = m_list.getEnd();
	while(it != end)
	{
		DS* set = &(*it);
		U64 frameDiff = crntFrame - set->m_lastFrameUsed;
		if(frameDiff > DESCRIPTOR_FRAME_BUFFERING)
		{
			// Found something, recycle
			m_hashmap.erase(set);
			m_list.erase(set);
			m_list.pushBack(set);
			m_hashmap.pushBack(hash, set);

			out = set;
			break;
		}
	}

	if(out == nullptr)
	{
		// Need to allocate one

		if(m_lastPoolFreeDSCount == 0)
		{
			// Can't allocate one from the current pool, create new
			ANKI_CHECK(createNewPool());
		}

		VkDescriptorSetAllocateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		ci.descriptorPool = m_pools.getBack();
		ci.pSetLayouts = &m_layoutEntry->m_layoutHandle;
		ci.descriptorSetCount = 1;

		VkDescriptorSet handle;
		VkResult rez = vkAllocateDescriptorSets(m_layoutEntry->m_factory->m_dev, &ci, &handle);
		(void)rez;
		ANKI_ASSERT(rez == VK_SUCCESS && "That allocation can't fail");

		out = m_layoutEntry->m_factory->m_alloc.newInstance<DS>();
		out->m_handle = handle;
	}

	ANKI_ASSERT(out);
	out->m_lastFrameUsed = m_layoutEntry->m_factory->m_frameCount;

	// Finally, write it
	writeSet(inf, *out);

	return ErrorCode::NONE;
}

void DSThreadAllocator::writeSet(const DSWriteInfo& inf, DS& set)
{
	ANKI_ASSERT(!"TODO");
}

Error DSLayoutCacheEntry::init(const DescriptorBinding* bindings, U bindingCount, U64 hash)
{
	ANKI_ASSERT(bindings);
	ANKI_ASSERT(hash > 0);

	m_hash = hash;

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

	ANKI_VK_CHECK(vkCreateDescriptorSetLayout(m_factory->m_dev, &ci, nullptr, &m_layoutHandle));

	// Create the pool info
	U poolSizeCount = 0;
	for(U i = 0; i < bindingCount; ++i)
	{
		U j;
		for(j = 0; j < poolSizeCount; ++j)
		{
			if(m_poolSizesCreateInf[j].type == bindings[i].m_type)
			{
				++m_poolSizesCreateInf[j].descriptorCount;
				break;
			}
		}

		if(j == poolSizeCount)
		{
			m_poolSizesCreateInf[poolSizeCount].type = static_cast<VkDescriptorType>(bindings[i].m_type);

			switch(m_poolSizesCreateInf[poolSizeCount].type)
			{
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				m_poolSizesCreateInf[poolSizeCount].descriptorCount = MAX_TEXTURE_BINDINGS;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				m_poolSizesCreateInf[poolSizeCount].descriptorCount = MAX_UNIFORM_BUFFER_BINDINGS;
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				m_poolSizesCreateInf[poolSizeCount].descriptorCount = MAX_STORAGE_BUFFER_BINDINGS;
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				m_poolSizesCreateInf[poolSizeCount].descriptorCount = MAX_IMAGE_BINDINGS;
				break;
			default:
				ANKI_ASSERT(0);
			}

			m_poolSizesCreateInf[poolSizeCount].descriptorCount = 1;
			++poolSizeCount;
		}
	}

	ANKI_ASSERT(poolSizeCount > 0);

	m_poolCreateInf.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	m_poolCreateInf.poolSizeCount = poolSizeCount;

	return ErrorCode::NONE;
}

DescriptorSetFactory::~DescriptorSetFactory()
{
}

void DescriptorSetFactory::init(const GrAllocator<U8>& alloc, VkDevice dev)
{
	m_alloc = alloc;
	m_dev = dev;
}

Error DescriptorSetFactory::newDescriptorSetLayout(const DescriptorSetLayoutInitInfo& init, DescriptorSetLayout& layout)
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
	LockGuard<Mutex> lock(m_cachesMtx);

	DSLayoutCacheEntry* cache = nullptr;
	U cacheIdx = MAX_U;
	U count = 0;
	for(DSLayoutCacheEntry* it : m_caches)
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
		cache = m_alloc.newInstance<DSLayoutCacheEntry>(this);
		ANKI_CHECK(cache->init(&bindings[0], bindingCount, hash));

		m_caches.resize(m_alloc, m_caches.getSize() + 1);
		m_caches[m_caches.getSize() - 1] = cache;
		cacheIdx = m_caches.getSize() - 1;
	}

	// Set the layout
	layout.m_handle = cache->m_layoutHandle;
	layout.m_cacheEntryIdx = cacheIdx;

	return ErrorCode::NONE;
}

} // end namespace anki

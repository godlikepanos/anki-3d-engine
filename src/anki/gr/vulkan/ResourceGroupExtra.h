// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/util/HashMap.h>

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

/// Creates descriptor set layouts.
class DescriptorSetLayoutFactory
{
public:
	DescriptorSetLayoutFactory()
	{
	}

	~DescriptorSetLayoutFactory()
	{
	}

	void init(GrAllocator<U8> alloc, VkDevice dev)
	{
		m_alloc = alloc;
		m_dev = dev;
	}

	void destroy();

	/// If there is no bindings for a specific type then pass zero.
	VkDescriptorSetLayout createLayout(
		U texBindingCount, U uniBindingCount, U storageBindingCount, U imageBindingCount);

private:
	class Key
	{
	public:
		U64 m_hash;

		Key(U a, U b, U c, U d)
		{
			ANKI_ASSERT(a < 0xFF && b < 0xFF && c < 0xFF && d < 0xFF);
			m_hash = (a << 24) | (b << 16) | (c << 8) | d;
		}
	};

	/// Hash the hash.
	class Hasher
	{
	public:
		U64 operator()(const Key& b) const
		{
			return b.m_hash;
		}
	};

	/// Hash compare.
	class Compare
	{
	public:
		Bool operator()(const Key& a, const Key& b) const
		{
			return a.m_hash == b.m_hash;
		}
	};

	HashMap<Key, VkDescriptorSetLayout, Hasher, Compare> m_map;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;

	Mutex m_mtx;
};
/// @}

} // end namespace anki

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

class DescriptorSetLayoutInfo
{
public:
	U8 m_texCount = MAX_U8;
	U8 m_uniCount = MAX_U8;
	U8 m_storageCount = MAX_U8;
	U8 m_imgCount = MAX_U8;

	DescriptorSetLayoutInfo() = default;

	DescriptorSetLayoutInfo(U texBindingCount, U uniBindingCount, U storageBindingCount, U imageBindingCount)
		: m_texCount(texBindingCount)
		, m_uniCount(uniBindingCount)
		, m_storageCount(storageBindingCount)
		, m_imgCount(imageBindingCount)
	{
		ANKI_ASSERT(m_texCount < MAX_U8 && m_uniCount < MAX_U8 && m_storageCount < MAX_U8 && m_imgCount < MAX_U8);
	}

	Bool operator==(const DescriptorSetLayoutInfo& b) const
	{
		return m_texCount == b.m_texCount && m_uniCount == b.m_uniCount && m_storageCount == b.m_storageCount
			&& m_imgCount == b.m_imgCount;
	}

	U64 computeHash() const
	{
		ANKI_ASSERT(m_texCount < MAX_U8 && m_uniCount < MAX_U8 && m_storageCount < MAX_U8 && m_imgCount < MAX_U8);
		return (m_texCount << 24) | (m_uniCount << 16) | (m_storageCount << 8) | m_imgCount;
	}
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
	ANKI_USE_RESULT Error getOrCreateLayout(const DescriptorSetLayoutInfo& dsinf, VkDescriptorSetLayout& out);

private:
	using Key = DescriptorSetLayoutInfo;

	/// Hash the hash.
	class Hasher
	{
	public:
		U64 operator()(const Key& b) const
		{
			return b.computeHash();
		}
	};

	/// Hash compare.
	class Compare
	{
	public:
		Bool operator()(const Key& a, const Key& b) const
		{
			return a.computeHash() == b.computeHash();
		}
	};

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;

	HashMap<Key, VkDescriptorSetLayout, Hasher, Compare> m_map;

	Mutex m_mtx;
};

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
	}

	ANKI_USE_RESULT Error init(GrAllocator<U8> alloc, VkDevice dev);

	void destroy();

	ANKI_USE_RESULT Error allocate(const DescriptorSetLayoutInfo& dsinf, VkDescriptorSet& out);

	void free(VkDescriptorSet ds)
	{
		ANKI_ASSERT(ds);
		LockGuard<Mutex> lock(m_mtx);
		ANKI_ASSERT(m_descriptorSetAllocationCount > 0);
		--m_descriptorSetAllocationCount;
		ANKI_VK_CHECKF(vkFreeDescriptorSets(m_dev, m_globalDPool, 1, &ds));
	}

	DescriptorSetLayoutFactory& getDescriptorSetLayoutFactory()
	{
		return m_layoutFactory;
	}

private:
	VkDevice m_dev = VK_NULL_HANDLE;
	VkDescriptorPool m_globalDPool = VK_NULL_HANDLE;
	Mutex m_mtx;
	U32 m_descriptorSetAllocationCount = 0;

	DescriptorSetLayoutFactory m_layoutFactory;

	Error initGlobalDsetPool();
};
/// @}

} // end namespace anki

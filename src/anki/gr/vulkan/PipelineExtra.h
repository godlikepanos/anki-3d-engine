// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/ResourceGroupExtra.h>
#include <anki/util/HashMap.h>
#include <anki/util/Hash.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Creator of compatible render passes.
class CompatibleRenderPassCreator : public NonCopyable
{
public:
	CompatibleRenderPassCreator(GrManagerImpl* manager)
		: m_manager(manager)
	{
		ANKI_ASSERT(manager);
	}

	~CompatibleRenderPassCreator()
	{
	}

	void destroy();

	ANKI_USE_RESULT VkRenderPass getOrCreateCompatibleRenderPass(const PipelineInitInfo& init);

private:
	class RenderPassKey
	{
	public:
		Array<PixelFormat, MAX_COLOR_ATTACHMENTS> m_colorAttachments;
		PixelFormat m_depthStencilAttachment;

		RenderPassKey()
		{
			// Zero because we compute hashes
			memset(this, 0, sizeof(*this));
		}

		RenderPassKey& operator=(const RenderPassKey& b) = default;
	};

	class RenderPassHasher
	{
	public:
		U64 operator()(const RenderPassKey& b) const
		{
			return computeHash(&b, sizeof(b));
		}
	};

	class RenderPassCompare
	{
	public:
		Bool operator()(const RenderPassKey& a, const RenderPassKey& b) const
		{
			for(U i = 0; i < a.m_colorAttachments.getSize(); ++i)
			{
				if(a.m_colorAttachments[i] != b.m_colorAttachments[i])
				{
					return false;
				}
			}

			return a.m_depthStencilAttachment == b.m_depthStencilAttachment;
		}
	};

	GrManagerImpl* m_manager = nullptr;

	Mutex m_mtx;
	HashMap<RenderPassKey, VkRenderPass, RenderPassHasher, RenderPassCompare> m_hashmap;

	VkRenderPass createNewRenderPass(const PipelineInitInfo& init);
};

/// Creator of pipeline layouts.
class PipelineLayoutFactory
{
public:
	PipelineLayoutFactory() = default;

	void init(GrAllocator<U8> alloc, VkDevice dev, DescriptorSetLayoutFactory* dsetLayFactory)
	{
		ANKI_ASSERT(dev);
		ANKI_ASSERT(dsetLayFactory);
		m_alloc = alloc;
		m_dev = dev;
		m_dsetLayFactory = dsetLayFactory;
	}

	void destroy();

	ANKI_USE_RESULT Error getOrCreateLayout(
		U8 setMask, const Array<DescriptorSetLayoutInfo, MAX_BOUND_RESOURCE_GROUPS>& dsinf, VkPipelineLayout& out);

private:
	class Key
	{
	public:
		U64 m_hash = 0;

		Key(U8 setMask, const Array<DescriptorSetLayoutInfo, MAX_BOUND_RESOURCE_GROUPS>& dsinf)
		{
			ANKI_ASSERT(MAX_BOUND_RESOURCE_GROUPS == 2 && "The following lines make that assumption");
			for(U i = 0; i < MAX_BOUND_RESOURCE_GROUPS; ++i)
			{
				if(setMask & (1 << i))
				{
					U64 hash = dsinf[i].computeHash();
					ANKI_ASSERT((hash >> 32) == 0);
					m_hash = (m_hash << 32) | hash;
				}
			}
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

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	DescriptorSetLayoutFactory* m_dsetLayFactory = nullptr;

	HashMap<Key, VkPipelineLayout, Hasher, Compare> m_map;
	Mutex m_mtx;
};
/// @}

} // end namespace anki

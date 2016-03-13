// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/Pipeline.h>
#include <anki/util/HashMap.h>
#include <anki/util/Hash.h>

namespace anki
{

//==============================================================================
// GrManagerImpl::CompatibleRenderPassHashMap                                  =
//==============================================================================

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

class GrManagerImpl::CompatibleRenderPassHashMap
{
public:
	Mutex m_mtx;
	HashMap<RenderPassKey, VkRenderPass, RenderPassHasher, RenderPassCompare>
		m_hashmap;
};

//==============================================================================
// GrManagerImpl                                                               =
//==============================================================================

//==============================================================================
VkRenderPass GrManagerImpl::getOrCreateCompatibleRenderPass(
	const PipelineInitInfo& init)
{
	VkRenderPass out;
	// Create the key
	RenderPassKey key;
	for(U i = 0; i < init.m_color.m_attachmentCount; ++i)
	{
		key.m_colorAttachments[i] = init.m_color.m_attachments[i].m_format;
	}
	key.m_depthStencilAttachment = init.m_depthStencil.m_format;

	// Lock
	LockGuard<Mutex> lock(m_renderPasses->m_mtx);

	auto it = m_renderPasses->m_hashmap.find(key);
	if(it != m_renderPasses->m_hashmap.getEnd())
	{
		// Found the key
		out = *it;
	}
	else
	{
		// Not found, create one
		VkRenderPassCreateInfo ci;
		ANKI_VK_MEMSET_DBG(ci);
		ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;

		Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1>
			attachmentDescriptions;
		Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS> references;
		for(U i = 0; i < init.m_color.m_attachmentCount; ++i)
		{
			// We only care about samples and format
			VkAttachmentDescription& desc = attachmentDescriptions[i];
			ANKI_VK_MEMSET_DBG(desc);
			desc.flags = 0;
			desc.format = convertFormat(init.m_color.m_attachments[i].m_format);
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			references[i].attachment = i;
			references[i].layout = VK_IMAGE_LAYOUT_UNDEFINED;
		}

		ci.attachmentCount = init.m_color.m_attachmentCount;

		Bool hasDepthStencil =
			init.m_depthStencil.m_format.m_components != ComponentFormat::NONE;
		VkAttachmentReference dsReference = {0, VK_IMAGE_LAYOUT_UNDEFINED};
		if(hasDepthStencil)
		{
			VkAttachmentDescription& desc =
				attachmentDescriptions[ci.attachmentCount];
			ANKI_VK_MEMSET_DBG(desc);
			desc.flags = 0;
			desc.format = convertFormat(init.m_depthStencil.m_format);
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			desc.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			dsReference.attachment = ci.attachmentCount;
			dsReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;

			++ci.attachmentCount;
		}

		VkSubpassDescription desc;
		ANKI_VK_MEMSET_DBG(desc);
		desc.flags = 0;
		desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		desc.inputAttachmentCount = 0;
		desc.pInputAttachments = nullptr;
		desc.colorAttachmentCount = init.m_color.m_attachmentCount;
		desc.pColorAttachments =
			(init.m_color.m_attachmentCount) ? &references[0] : nullptr;
		desc.pResolveAttachments = nullptr;
		desc.pDepthStencilAttachment =
			(hasDepthStencil) ? &dsReference : nullptr;
		desc.preserveAttachmentCount = 0;
		desc.pPreserveAttachments = nullptr;

		ANKI_ASSERT(ci.attachmentCount);
		ci.pAttachments = &attachmentDescriptions[0];
		ci.subpassCount = 1;
		ci.pSubpasses = &desc;
		ci.dependencyCount = 0;
		ci.pDependencies = nullptr;

		VkRenderPass rpass;
		ANKI_VK_CHECK(vkCreateRenderPass(m_device, &ci, nullptr, &rpass));

		m_renderPasses->m_hashmap.pushBack(m_alloc, key, rpass);
	}

	return out;
}

} // end namespace anki

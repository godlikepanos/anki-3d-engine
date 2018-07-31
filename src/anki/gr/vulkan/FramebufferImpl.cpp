// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>

namespace anki
{

FramebufferImpl::~FramebufferImpl()
{
	if(m_fb)
	{
		vkDestroyFramebuffer(getDevice(), m_fb, nullptr);
	}

	for(auto it : m_rpasses)
	{
		VkRenderPass rpass = it;
		ANKI_ASSERT(rpass);
		vkDestroyRenderPass(getDevice(), rpass, nullptr);
	}

	m_rpasses.destroy(getAllocator());

	if(m_compatibleRpass)
	{
		vkDestroyRenderPass(getDevice(), m_compatibleRpass, nullptr);
	}
}

Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	ANKI_ASSERT(init.isValid());

	// Init common
	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		m_colorAttachmentMask.set(i);
		m_colorAttCount = i + 1;
	}

	if(init.m_depthStencilAttachment.m_textureView)
	{
		m_aspect = init.m_depthStencilAttachment.m_textureView->getSubresource().m_depthStencilAspect;
	}

	initClearValues(init);

	// Create a renderpass.
	initRpassCreateInfo(init);
	ANKI_VK_CHECK(vkCreateRenderPass(getDevice(), &m_rpassCi, nullptr, &m_compatibleRpass));
	getGrManagerImpl().trySetVulkanHandleName(
		init.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, m_compatibleRpass);

	// Create the FB
	ANKI_CHECK(initFbs(init));

	return Error::NONE;
}

void FramebufferImpl::initClearValues(const FramebufferInitInfo& init)
{
	for(U i = 0; i < m_colorAttCount; ++i)
	{
		if(init.m_colorAttachments[i].m_loadOperation == AttachmentLoadOperation::CLEAR)
		{
			F32* col = &m_clearVals[i].color.float32[0];
			col[0] = init.m_colorAttachments[i].m_clearValue.m_colorf[0];
			col[1] = init.m_colorAttachments[i].m_clearValue.m_colorf[1];
			col[2] = init.m_colorAttachments[i].m_clearValue.m_colorf[2];
			col[3] = init.m_colorAttachments[i].m_clearValue.m_colorf[3];
		}
		else
		{
			m_clearVals[i] = {};
		}
	}

	if(hasDepthStencil())
	{
		if(init.m_depthStencilAttachment.m_loadOperation == AttachmentLoadOperation::CLEAR
			|| init.m_depthStencilAttachment.m_stencilLoadOperation == AttachmentLoadOperation::CLEAR)
		{
			m_clearVals[m_colorAttCount].depthStencil.depth =
				init.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth;

			m_clearVals[m_colorAttCount].depthStencil.stencil =
				init.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_stencil;
		}
		else
		{
			m_clearVals[m_colorAttCount] = {};
		}
	}
}

Error FramebufferImpl::initFbs(const FramebufferInitInfo& init)
{
	VkFramebufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_compatibleRpass;
	ci.attachmentCount = init.m_colorAttachmentCount + ((hasDepthStencil()) ? 1 : 0);
	ci.layers = 1;

	Array<VkImageView, MAX_COLOR_ATTACHMENTS + 1> imgViews;
	U count = 0;

	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		const FramebufferAttachmentInfo& att = init.m_colorAttachments[i];
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*att.m_textureView);
		const TextureImpl& tex = static_cast<const TextureImpl&>(*view.m_tex);
		ANKI_ASSERT(tex.isSubresourceGoodForFramebufferAttachment(view.getSubresource()));

		imgViews[count++] = view.m_handle;

		if(m_width == 0)
		{
			m_width = tex.getWidth() >> view.getSubresource().m_firstMipmap;
			m_height = tex.getHeight() >> view.getSubresource().m_firstMipmap;
		}

		m_refs[i] = att.m_textureView;

		if(!!(tex.getTextureUsage() & TextureUsageBit::PRESENT))
		{
			m_presentableTex = true;
		}
	}

	if(hasDepthStencil())
	{
		const FramebufferAttachmentInfo& att = init.m_depthStencilAttachment;
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*att.m_textureView);
		const TextureImpl& tex = static_cast<const TextureImpl&>(*view.m_tex);
		ANKI_ASSERT(tex.isSubresourceGoodForFramebufferAttachment(view.getSubresource()));

		imgViews[count++] = view.m_handle;

		if(m_width == 0)
		{
			m_width = tex.getWidth() >> view.getSubresource().m_firstMipmap;
			m_height = tex.getHeight() >> view.getSubresource().m_firstMipmap;
		}

		m_refs[MAX_COLOR_ATTACHMENTS] = att.m_textureView;
	}

	ci.width = m_width;
	ci.height = m_height;

	ci.pAttachments = &imgViews[0];
	ANKI_ASSERT(count == ci.attachmentCount);

	ANKI_VK_CHECK(vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_fb));
	getGrManagerImpl().trySetVulkanHandleName(init.getName(), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, m_fb);

	return Error::NONE;
}

void FramebufferImpl::setupAttachmentDescriptor(
	const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc, VkImageLayout layout) const
{
	desc = {};
	desc.format = convertFormat(static_cast<const TextureViewImpl&>(*att.m_textureView).m_tex->getFormat());
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = convertLoadOp(att.m_loadOperation);
	desc.storeOp = convertStoreOp(att.m_storeOperation);
	desc.stencilLoadOp = convertLoadOp(att.m_stencilLoadOperation);
	desc.stencilStoreOp = convertStoreOp(att.m_stencilStoreOperation);
	desc.initialLayout = layout;
	desc.finalLayout = layout;
}

void FramebufferImpl::initRpassCreateInfo(const FramebufferInitInfo& init)
{
	// Setup attachments and references
	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		setupAttachmentDescriptor(
			init.m_colorAttachments[i], m_attachmentDescriptions[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_references[i].attachment = i;
		m_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if(hasDepthStencil())
	{
		setupAttachmentDescriptor(init.m_depthStencilAttachment,
			m_attachmentDescriptions[init.m_colorAttachmentCount],
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkAttachmentReference& dsReference = m_references[init.m_colorAttachmentCount];
		dsReference.attachment = init.m_colorAttachmentCount;
		dsReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Setup the render pass
	m_rpassCi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	m_rpassCi.pAttachments = &m_attachmentDescriptions[0];
	m_rpassCi.attachmentCount = init.m_colorAttachmentCount + ((hasDepthStencil()) ? 1 : 0);

	// Subpass
	m_subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	m_subpassDescr.colorAttachmentCount = init.m_colorAttachmentCount;
	m_subpassDescr.pColorAttachments = (init.m_colorAttachmentCount) ? &m_references[0] : nullptr;
	m_subpassDescr.pDepthStencilAttachment = (hasDepthStencil()) ? &m_references[init.m_colorAttachmentCount] : nullptr;

	m_rpassCi.subpassCount = 1;
	m_rpassCi.pSubpasses = &m_subpassDescr;
}

VkRenderPass FramebufferImpl::getRenderPassHandle(
	const Array<VkImageLayout, MAX_COLOR_ATTACHMENTS>& colorLayouts, VkImageLayout dsLayout)
{
	VkRenderPass out = {};

	// Create hash
	Array<VkImageLayout, MAX_COLOR_ATTACHMENTS + 1> allLayouts;
	U allLayoutCount = 0;
	for(U i = 0; i < m_colorAttCount; ++i)
	{
		ANKI_ASSERT(colorLayouts[i] != VK_IMAGE_LAYOUT_UNDEFINED);
		allLayouts[allLayoutCount++] = colorLayouts[i];
	}

	if(hasDepthStencil())
	{
		ANKI_ASSERT(dsLayout != VK_IMAGE_LAYOUT_UNDEFINED);
		allLayouts[allLayoutCount++] = dsLayout;
	}

	U64 hash = computeHash(&allLayouts[0], allLayoutCount * sizeof(allLayouts[0]));

	// Get or create
	LockGuard<Mutex> lock(m_rpassesMtx);
	auto it = m_rpasses.find(hash);
	if(it != m_rpasses.getEnd())
	{
		out = *it;
	}
	else
	{
		// Create

		VkRenderPassCreateInfo ci = m_rpassCi;
		Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> attachmentDescriptions = m_attachmentDescriptions;
		Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS + 1> references = m_references;
		VkSubpassDescription subpassDescr = m_subpassDescr;

		// Fix pointers
		subpassDescr.pColorAttachments = &references[0];
		ci.pAttachments = &attachmentDescriptions[0];
		ci.pSubpasses = &subpassDescr;

		for(U i = 0; i < subpassDescr.colorAttachmentCount; ++i)
		{
			const VkImageLayout lay = colorLayouts[i];
			ANKI_ASSERT(lay != VK_IMAGE_LAYOUT_UNDEFINED);

			attachmentDescriptions[i].initialLayout = lay;
			attachmentDescriptions[i].finalLayout = lay;

			references[i].layout = lay;
		}

		if(hasDepthStencil())
		{
			const U i = subpassDescr.colorAttachmentCount;
			const VkImageLayout lay = dsLayout;
			ANKI_ASSERT(lay != VK_IMAGE_LAYOUT_UNDEFINED);

			attachmentDescriptions[i].initialLayout = lay;
			attachmentDescriptions[i].finalLayout = lay;

			references[subpassDescr.colorAttachmentCount].layout = lay;
			subpassDescr.pDepthStencilAttachment = &references[subpassDescr.colorAttachmentCount];
		}

		ANKI_VK_CHECKF(vkCreateRenderPass(getDevice(), &ci, nullptr, &out));
		getGrManagerImpl().trySetVulkanHandleName(getName(), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, out);

		m_rpasses.emplace(getAllocator(), hash, out);
	}

	ANKI_ASSERT(out);
	return out;
}

} // end namespace anki

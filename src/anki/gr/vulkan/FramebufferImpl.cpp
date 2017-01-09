// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	// TODO
}

Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	m_defaultFb = init.refersToDefaultFramebuffer();

	// Create a renderpass.
	initRpassCreateInfo(init);
	ANKI_VK_CHECK(vkCreateRenderPass(getDevice(), &m_rpassCi, nullptr, &m_rpass));

	// Create the FBs
	ANKI_CHECK(initFbs(init));

	return ErrorCode::NONE;
}

Error FramebufferImpl::initFbs(const FramebufferInitInfo& init)
{
	const Bool hasDepthStencil = init.m_depthStencilAttachment.m_texture == true;

	VkFramebufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_rpass;
	ci.attachmentCount = init.m_colorAttachmentCount + ((hasDepthStencil) ? 1 : 0);

	ci.layers = 1;

	if(m_defaultFb)
	{
		for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkImageView view = getGrManagerImpl().getDefaultSurfaceImageView(i);
			ci.pAttachments = &view;

			m_width = getGrManagerImpl().getDefaultSurfaceWidth();
			m_height = getGrManagerImpl().getDefaultSurfaceHeight();
			ci.width = m_width;
			ci.height = m_height;

			ANKI_VK_CHECK(vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_fbs[i]));
		}

		m_colorAttachmentMask.set(0);
	}
	else
	{
		Array<VkImageView, MAX_COLOR_ATTACHMENTS + 1> attachments;
		U count = 0;

		for(U i = 0; i < init.m_colorAttachmentCount; ++i)
		{
			const FramebufferAttachmentInfo& att = init.m_colorAttachments[i];
			TextureImpl& tex = *att.m_texture->m_impl;

			attachments[count++] = tex.getOrCreateSingleSurfaceView(att.m_surface, att.m_aspect);

			if(m_width == 0)
			{
				m_width = tex.m_width >> att.m_surface.m_level;
				m_height = tex.m_height >> att.m_surface.m_level;
			}

			m_refs[i] = att.m_texture;
			m_colorAttachmentMask.set(i);
		}

		if(hasDepthStencil)
		{
			const FramebufferAttachmentInfo& att = init.m_depthStencilAttachment;
			TextureImpl& tex = *att.m_texture->m_impl;

			DepthStencilAspectBit aspect;
			if(!!(tex.m_workarounds & TextureImplWorkaround::S8_TO_D24S8))
			{
				aspect = DepthStencilAspectBit::STENCIL;
			}
			else
			{
				aspect = att.m_aspect;
			}

			m_depthAttachment = !!(aspect & DepthStencilAspectBit::DEPTH);
			m_stencilAttachment = !!(aspect & DepthStencilAspectBit::STENCIL);

			attachments[count++] = tex.getOrCreateSingleSurfaceView(att.m_surface, aspect);

			if(m_width == 0)
			{
				m_width = tex.m_width >> att.m_surface.m_level;
				m_height = tex.m_height >> att.m_surface.m_level;
			}

			m_refs[init.m_colorAttachmentCount] = att.m_texture;
		}

		ci.width = m_width;
		ci.height = m_height;

		ci.pAttachments = &attachments[0];
		ANKI_ASSERT(count == ci.attachmentCount);

		ANKI_VK_CHECK(vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_fbs[0]));
	}

	return ErrorCode::NONE;
}

void FramebufferImpl::setupAttachmentDescriptor(
	const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc) const
{
	// TODO This func won't work if it's default but this is a depth attachment

	const VkImageLayout layout = (m_defaultFb) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_MAX_ENUM;

	desc = {};
	desc.format = (m_defaultFb) ? getGrManagerImpl().getDefaultFramebufferSurfaceFormat()
								: convertFormat(att.m_texture->m_impl->m_format);
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
	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		const FramebufferAttachmentInfo& att = init.m_colorAttachments[i];

		setupAttachmentDescriptor(att, m_attachmentDescriptions[i]);

		m_references[i].attachment = i;
		m_references[i].layout = (att.m_texture) ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	const Bool hasDepthStencil = init.m_depthStencilAttachment.m_texture == true;
	if(hasDepthStencil)
	{
		VkAttachmentReference& dsReference = m_references[init.m_colorAttachmentCount];

		setupAttachmentDescriptor(init.m_depthStencilAttachment, m_attachmentDescriptions[init.m_colorAttachmentCount]);

		dsReference.attachment = init.m_colorAttachmentCount;
		dsReference.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	// Setup the render pass
	m_rpassCi.pAttachments = &m_attachmentDescriptions[0];
	m_rpassCi.attachmentCount = init.m_colorAttachmentCount + ((hasDepthStencil) ? 1 : 0);

	// Subpass
	m_subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	m_subpassDescr.colorAttachmentCount = init.m_colorAttachmentCount;
	m_subpassDescr.pColorAttachments = (init.m_colorAttachmentCount) ? &m_references[0] : nullptr;
	m_subpassDescr.pDepthStencilAttachment = (hasDepthStencil) ? &m_references[init.m_colorAttachmentCount] : nullptr;

	m_rpassCi.subpassCount = 1;
	m_rpassCi.pSubpasses = &m_subpassDescr;
}

VkRenderPass FramebufferImpl::getRenderPass(WeakArray<TextureUsageBit> usages)
{
	VkRenderPass out;

	if(!m_defaultFb)
	{
		// Create hash
		U64 hash = computeHash(&usages[0], usages.getSize() * sizeof(usages[0]));
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
			subpassDescr.pDepthStencilAttachment = &references[MAX_COLOR_ATTACHMENTS];
			ci.pAttachments = &attachmentDescriptions[0];
			ci.pSubpasses = &subpassDescr;

			for(U i = 0; i < subpassDescr.colorAttachmentCount; ++i)
			{
				const VkImageLayout lay = m_refs[i]->m_impl->computeLayout(usages[i], m_attachedMipLevels[i]);

				attachmentDescriptions[i].initialLayout = lay;
				attachmentDescriptions[i].finalLayout = lay;

				references[i].layout = lay;
			}

			if(m_refs[MAX_COLOR_ATTACHMENTS])
			{
				const U i = MAX_COLOR_ATTACHMENTS;
				const VkImageLayout lay = m_refs[i]->m_impl->computeLayout(usages[i], m_attachedMipLevels[i]);

				attachmentDescriptions[i].initialLayout = lay;
				attachmentDescriptions[i].finalLayout = lay;

				references[i].layout = lay;
			}

			ANKI_VK_CHECKF(vkCreateRenderPass(getDevice(), &ci, nullptr, &out));

			m_rpasses.pushBack(getAllocator(), hash, out);
		}
	}
	else
	{
		out = m_rpass;
	}

	ANKI_ASSERT(out);
	return out;
}

} // end namespace anki

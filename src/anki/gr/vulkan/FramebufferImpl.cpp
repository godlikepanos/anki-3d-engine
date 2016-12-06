// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
	if(m_renderPass)
	{
		vkDestroyRenderPass(getDevice(), m_renderPass, nullptr);
	}

	for(VkFramebuffer fb : m_framebuffers)
	{
		if(fb)
		{
			vkDestroyFramebuffer(getDevice(), fb, nullptr);
		}
	}
}

Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	ANKI_ASSERT(initInfoValid(init));
	m_defaultFramebuffer = init.refersToDefaultFramebuffer();

	ANKI_CHECK(initRenderPass(init));
	ANKI_CHECK(initFramebuffer(init));

	// Set clear values
	m_attachmentCount = 0;
	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
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

		++m_attachmentCount;
	}

	if(init.m_depthStencilAttachment.m_texture.isCreated())
	{
		if(init.m_depthStencilAttachment.m_loadOperation == AttachmentLoadOperation::CLEAR
			|| init.m_depthStencilAttachment.m_stencilLoadOperation == AttachmentLoadOperation::CLEAR)
		{
			m_clearVals[m_attachmentCount].depthStencil.depth =
				init.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_depth;

			m_clearVals[m_attachmentCount].depthStencil.stencil =
				init.m_depthStencilAttachment.m_clearValue.m_depthStencil.m_stencil;
		}
		else
		{
			m_clearVals[m_attachmentCount] = {};
		}

		++m_attachmentCount;
	}

	return ErrorCode::NONE;
}

Bool FramebufferImpl::initInfoValid(const FramebufferInitInfo& inf)
{
	if(!framebufferInitInfoValid(inf))
	{
		return false;
	}

	if(inf.m_depthStencilAttachment.m_texture)
	{
		const TextureImpl& impl = *inf.m_depthStencilAttachment.m_texture->m_impl;
		if(impl.m_akAspect == DepthStencilAspectBit::DEPTH_STENCIL
			&& !(impl.m_workarounds & TextureImplWorkaround::S8_TO_D24S8))
		{
			if(inf.m_depthStencilAttachment.m_aspect == DepthStencilAspectBit::NONE)
			{
				return false;
			}
		}
	}

	return true;
}

void FramebufferImpl::setupAttachmentDescriptor(const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc)
{
	// TODO This func won't work if it's default but this is a depth attachment

	VkImageLayout layout;
	if(m_defaultFramebuffer)
	{
		layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else
	{
		layout = att.m_texture->m_impl->computeLayout(att.m_usageInsideRenderPass, 0);
	}

	desc = {};
	desc.format = (m_defaultFramebuffer) ? getGrManagerImpl().getDefaultFramebufferSurfaceFormat()
										 : convertFormat(att.m_texture->m_impl->m_format);
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = convertLoadOp(att.m_loadOperation);
	desc.storeOp = convertStoreOp(att.m_storeOperation);
	desc.stencilLoadOp = convertLoadOp(att.m_stencilLoadOperation);
	desc.stencilStoreOp = convertStoreOp(att.m_stencilStoreOperation);
	desc.initialLayout = layout;
	desc.finalLayout = layout;
}

Error FramebufferImpl::initRenderPass(const FramebufferInitInfo& init)
{
	VkRenderPassCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	// First setup the attachments
	ci.attachmentCount = 0;
	Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> attachmentDescriptions;
	Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS> references;
	Bool hasDepthStencil = init.m_depthStencilAttachment.m_texture == true;

	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		const FramebufferAttachmentInfo& att = init.m_colorAttachments[i];

		setupAttachmentDescriptor(att, attachmentDescriptions[i]);

		references[i].attachment = i;
		references[i].layout = (att.m_texture) ? att.m_texture->m_impl->computeLayout(att.m_usageInsideRenderPass, 0)
											   : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		++ci.attachmentCount;
	}

	VkAttachmentReference dsReference = {};
	if(hasDepthStencil)
	{
		setupAttachmentDescriptor(init.m_depthStencilAttachment, attachmentDescriptions[ci.attachmentCount]);

		dsReference.attachment = ci.attachmentCount;
		dsReference.layout = init.m_depthStencilAttachment.m_texture->m_impl->computeLayout(
			init.m_depthStencilAttachment.m_usageInsideRenderPass, 0);

		++ci.attachmentCount;
	}

	// Setup the render pass
	ci.pAttachments = &attachmentDescriptions[0];

	// Subpass
	VkSubpassDescription spass = {};
	spass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	spass.colorAttachmentCount = init.m_colorAttachmentCount;
	spass.pColorAttachments = (init.m_colorAttachmentCount) ? &references[0] : nullptr;
	spass.pDepthStencilAttachment = (hasDepthStencil) ? &dsReference : nullptr;

	ci.subpassCount = 1;
	ci.pSubpasses = &spass;

	ANKI_VK_CHECK(vkCreateRenderPass(getDevice(), &ci, nullptr, &m_renderPass));

	return ErrorCode::NONE;
}

Error FramebufferImpl::initFramebuffer(const FramebufferInitInfo& init)
{
	Bool hasDepthStencil = init.m_depthStencilAttachment.m_texture == true;

	VkFramebufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_renderPass;
	ci.attachmentCount = init.m_colorAttachmentCount + ((hasDepthStencil) ? 1 : 0);

	ci.layers = 1;

	if(m_defaultFramebuffer)
	{
		for(U i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VkImageView view = getGrManagerImpl().getDefaultSurfaceImageView(i);
			ci.pAttachments = &view;

			m_width = getGrManagerImpl().getDefaultSurfaceWidth();
			m_height = getGrManagerImpl().getDefaultSurfaceHeight();
			ci.width = m_width;
			ci.height = m_height;

			ANKI_VK_CHECK(vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_framebuffers[i]));
		}
	}
	else
	{
		Array<VkImageView, MAX_COLOR_ATTACHMENTS + 1> attachments;
		U count = 0;

		for(U i = 0; i < init.m_colorAttachmentCount; ++i)
		{
			const FramebufferAttachmentInfo& att = init.m_colorAttachments[i];
			TextureImpl& tex = *att.m_texture->m_impl;

			attachments[count] = tex.getOrCreateSingleSurfaceView(att.m_surface, att.m_aspect);

			if(m_width == 0)
			{
				m_width = tex.m_width >> att.m_surface.m_level;
				m_height = tex.m_height >> att.m_surface.m_level;
			}

			m_refs[count++] = att.m_texture;
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

			attachments[count] = tex.getOrCreateSingleSurfaceView(att.m_surface, aspect);

			if(m_width == 0)
			{
				m_width = tex.m_width >> att.m_surface.m_level;
				m_height = tex.m_height >> att.m_surface.m_level;
			}

			m_refs[count++] = att.m_texture;
		}

		ci.width = m_width;
		ci.height = m_height;

		ci.pAttachments = &attachments[0];
		ANKI_ASSERT(count == ci.attachmentCount);

		ANKI_VK_CHECK(vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_framebuffers[0]));
	}

	return ErrorCode::NONE;
}

} // end namespace anki

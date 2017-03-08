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
	for(VkFramebuffer fb : m_fbs)
	{
		if(fb)
		{
			vkDestroyFramebuffer(getDevice(), fb, nullptr);
		}
	}

	for(auto it : m_rpasses)
	{
		VkRenderPass rpass = it;
		ANKI_ASSERT(rpass);
		vkDestroyRenderPass(getDevice(), rpass, nullptr);
	}

	m_rpasses.destroy(getAllocator());

	if(m_compatibleOrDefaultRpass)
	{
		vkDestroyRenderPass(getDevice(), m_compatibleOrDefaultRpass, nullptr);
	}
}

Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	m_defaultFb = init.refersToDefaultFramebuffer();

	// Create a renderpass.
	initRpassCreateInfo(init);
	ANKI_VK_CHECK(vkCreateRenderPass(getDevice(), &m_rpassCi, nullptr, &m_compatibleOrDefaultRpass));

	// Create the FBs
	ANKI_CHECK(initFbs(init));

	// Set clear values and some other stuff
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

		m_attachedSurfaces[i] = init.m_colorAttachments[i].m_surface;
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

		m_attachedSurfaces[MAX_COLOR_ATTACHMENTS] = init.m_depthStencilAttachment.m_surface;
	}

	return ErrorCode::NONE;
}

Error FramebufferImpl::initFbs(const FramebufferInitInfo& init)
{
	const Bool hasDepthStencil = init.m_depthStencilAttachment.m_texture == true;

	VkFramebufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_compatibleOrDefaultRpass;
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
		m_colorAttCount = 1;
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
			m_colorAttCount = i + 1;
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
			else if(tex.m_akAspect == DepthStencilAspectBit::DEPTH)
			{
				aspect = DepthStencilAspectBit::DEPTH;
			}
			else if(tex.m_akAspect == DepthStencilAspectBit::STENCIL)
			{
				aspect = DepthStencilAspectBit::STENCIL;
			}
			else
			{
				ANKI_ASSERT(!!att.m_aspect);
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

			m_refs[MAX_COLOR_ATTACHMENTS] = att.m_texture;
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
		m_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	const Bool hasDepthStencil = init.m_depthStencilAttachment.m_texture == true;
	if(hasDepthStencil)
	{
		VkAttachmentReference& dsReference = m_references[init.m_colorAttachmentCount];

		setupAttachmentDescriptor(init.m_depthStencilAttachment, m_attachmentDescriptions[init.m_colorAttachmentCount]);

		dsReference.attachment = init.m_colorAttachmentCount;
		dsReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	// Setup the render pass
	m_rpassCi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
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

VkRenderPass FramebufferImpl::getRenderPassHandle(
	const Array<VkImageLayout, MAX_COLOR_ATTACHMENTS>& colorLayouts, VkImageLayout dsLayout)
{
	VkRenderPass out;

	if(!m_defaultFb)
	{
		// Create hash
		Array<VkImageLayout, MAX_COLOR_ATTACHMENTS + 1> allLayouts;
		U allLayoutCount = 0;
		for(U i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
		{
			if(m_colorAttachmentMask.get(i))
			{
				ANKI_ASSERT(colorLayouts[i] != VK_IMAGE_LAYOUT_UNDEFINED);
				allLayouts[allLayoutCount++] = colorLayouts[i];
			}
		}

		if(m_depthAttachment || m_stencilAttachment)
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

			if(m_refs[MAX_COLOR_ATTACHMENTS])
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

			m_rpasses.pushBack(getAllocator(), hash, out);
		}
	}
	else
	{
		out = m_compatibleOrDefaultRpass;
	}

	ANKI_ASSERT(out);
	return out;
}

} // end namespace anki

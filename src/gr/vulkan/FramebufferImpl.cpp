// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/gr/Framebuffer.h>

namespace anki
{

//==============================================================================
FramebufferImpl::~FramebufferImpl()
{
	if(m_renderPass)
	{
		vkDestroyRenderPass(getDevice(), m_renderPass, nullptr);
	}

	if(m_framebuffer)
	{
		vkDestroyFramebuffer(getDevice(), m_framebuffer, nullptr);
	}
}

//==============================================================================
Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	if(init.m_colorAttachmentCount == 0
		&& !init.m_depthStencilAttachment.m_texture.isCreated())
	{
		m_defaultFramebuffer = true;
	}
	else
	{
		m_defaultFramebuffer = false;
		ANKI_CHECK(initRenderPass(init));
		ANKI_CHECK(initFramebuffer(init));
	}

	return ErrorCode::NONE;
}

//==============================================================================
void FramebufferImpl::setupAttachmentDescriptor(
	const Attachment& att, VkAttachmentDescription& desc, Bool depthStencil)
{
	VkImageLayout initLayout = (depthStencil)
		? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	ANKI_VK_MEMSET_DBG(desc);
	desc.flags = 0;
	desc.format = convertFormat(att.m_format);
	desc.samples = VK_SAMPLE_COUNT_1_BIT;
	desc.loadOp = convertLoadOp(att.m_loadOperation);
	desc.storeOp = convertStoreOp(att.m_storeOperation);
	desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	desc.initialLayout = (att.m_loadOperation == AttachmentLoadOperation::LOAD)
		? initLayout
		: VK_IMAGE_LAYOUT_UNDEFINED;
	desc.finalLayout = (att.m_storeOperation == AttachmentStoreOperation::STORE)
		? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		: VK_IMAGE_LAYOUT_UNDEFINED;
}

//==============================================================================
Error FramebufferImpl::initRenderPass(const FramebufferInitInfo& init)
{
	VkRenderPassCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	// First setup the attachments
	ci.attachmentCount = 0;
	Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1>
		attachmentDescriptions;
	Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS> references;
	Bool hasDepthStencil = init.m_depthStencilAttachment.m_format.m_components
		!= ComponentFormat::NONE;

	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		setupAttachmentDescriptor(
			init.m_colorAttachments[i], attachmentDescriptions[i], false);

		references[i].attachment = i;
		references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		++ci.attachmentCount;
	}

	VkAttachmentReference dsReference = {0, VK_IMAGE_LAYOUT_UNDEFINED};
	if(hasDepthStencil)
	{
		setupAttachmentDescriptor(init.m_depthStencilAttachment,
			attachmentDescriptions[ci.attachmentCount],
			true);

		dsReference.attachment = ci.attachmentCount;
		dsReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		++ci.attachmentCount;
	}

	// Setup the render pass
	ci.pAttachments = &attachmentDescriptions[0];

	// Subpass
	VkSubpassDescription spass;
	ANKI_VK_MEMSET_DBG(spass);
	spass.flags = 0;
	spass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	spass.inputAttachmentCount = 0;
	spass.pInputAttachments = nullptr;
	spass.colorAttachmentCount = init.m_colorAttachmentCount;
	spass.pColorAttachments =
		(init.m_colorAttachmentCount) ? &references[0] : nullptr;
	spass.pResolveAttachments = nullptr;
	spass.pDepthStencilAttachment = (hasDepthStencil) ? &dsReference : nullptr;
	spass.preserveAttachmentCount = 0;
	spass.pPreserveAttachments = nullptr;

	ci.subpassCount = 1;
	ci.pSubpasses = &spass;
	ci.dependencyCount = 0;
	ci.pDependencies = nullptr;

	ANKI_VK_CHECK(vkCreateRenderPass(getDevice(), &ci, nullptr, &m_renderPass));

	return ErrorCode::NONE;
}

//==============================================================================
Error FramebufferImpl::initFramebuffer(const FramebufferInitInfo& init)
{
	Bool hasDepthStencil = init.m_depthStencilAttachment.m_format.m_components
		!= ComponentFormat::NONE;

	VkFramebufferCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.renderPass = m_renderPass;
	ci.attachmentCount =
		init.m_colorAttachmentCount + ((hasDepthStencil) ? 1 : 0);

	// TODO set views
	// TODO set size and the rest

	ANKI_VK_CHECK(
		vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_framebuffer));

	return ErrorCode::NONE;
}

} // end namespace anki

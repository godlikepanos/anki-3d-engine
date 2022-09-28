// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/FramebufferImpl.h>
#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>

namespace anki {

FramebufferImpl::~FramebufferImpl()
{
	if(m_fbHandle)
	{
		vkDestroyFramebuffer(getDevice(), m_fbHandle, nullptr);
	}

	for(auto it : m_renderpassHandles)
	{
		VkRenderPass rpass = it;
		ANKI_ASSERT(rpass);
		vkDestroyRenderPass(getDevice(), rpass, nullptr);
	}

	m_renderpassHandles.destroy(getMemoryPool());

	if(m_compatibleRenderpassHandle)
	{
		vkDestroyRenderPass(getDevice(), m_compatibleRenderpassHandle, nullptr);
	}
}

Error FramebufferImpl::init(const FramebufferInitInfo& init)
{
	ANKI_ASSERT(init.isValid());

	// Init common
	for(U32 i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		m_colorAttachmentMask.set(i);
		m_colorAttCount = U8(i + 1);
	}

	if(init.m_depthStencilAttachment.m_textureView)
	{
		m_aspect = init.m_depthStencilAttachment.m_textureView->getSubresource().m_depthStencilAspect;
	}

	m_hasSri = init.m_shadingRateImage.m_textureView.isCreated();

	initClearValues(init);

	// Create a renderpass.
	initRpassCreateInfo(init);
	ANKI_VK_CHECK(vkCreateRenderPass2KHR(getDevice(), &m_rpassCi, nullptr, &m_compatibleRenderpassHandle));
	getGrManagerImpl().trySetVulkanHandleName(init.getName(), VK_OBJECT_TYPE_RENDER_PASS, m_compatibleRenderpassHandle);

	// Create the FB
	ANKI_CHECK(initFbs(init));

	return Error::kNone;
}

void FramebufferImpl::initClearValues(const FramebufferInitInfo& init)
{
	for(U i = 0; i < m_colorAttCount; ++i)
	{
		if(init.m_colorAttachments[i].m_loadOperation == AttachmentLoadOperation::kClear)
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
		if(init.m_depthStencilAttachment.m_loadOperation == AttachmentLoadOperation::kClear
		   || init.m_depthStencilAttachment.m_stencilLoadOperation == AttachmentLoadOperation::kClear)
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

	// Put something for SRI. Value doesn't matter
	if(hasSri())
	{
		m_clearVals[m_colorAttCount + hasDepthStencil()] = {};
	}
}

Error FramebufferImpl::initFbs(const FramebufferInitInfo& init)
{
	VkFramebufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.renderPass = m_compatibleRenderpassHandle;
	ci.attachmentCount = getTotalAttachmentCount();
	ci.layers = 1;

	Array<VkImageView, kMaxAttachments> imgViews;
	U count = 0;

	for(U i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		const FramebufferAttachmentInfo& att = init.m_colorAttachments[i];
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*att.m_textureView);
		const TextureImpl& tex = view.getTextureImpl();
		ANKI_ASSERT(tex.isSubresourceGoodForFramebufferAttachment(view.getSubresource()));

		imgViews[count] = view.getHandle();

		if(m_width == 0)
		{
			m_width = tex.getWidth() >> view.getSubresource().m_firstMipmap;
			m_height = tex.getHeight() >> view.getSubresource().m_firstMipmap;
		}

		m_viewRefs.m_color[i] = att.m_textureView;

		if(!!(tex.getTextureUsage() & TextureUsageBit::kPresent))
		{
			m_presentableTex = true;
		}

		++count;
	}

	if(hasDepthStencil())
	{
		const FramebufferAttachmentInfo& att = init.m_depthStencilAttachment;
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*att.m_textureView);
		const TextureImpl& tex = view.getTextureImpl();
		ANKI_ASSERT(tex.isSubresourceGoodForFramebufferAttachment(view.getSubresource()));

		imgViews[count] = view.getHandle();

		if(m_width == 0)
		{
			m_width = tex.getWidth() >> view.getSubresource().m_firstMipmap;
			m_height = tex.getHeight() >> view.getSubresource().m_firstMipmap;
		}

		m_viewRefs.m_depthStencil = att.m_textureView;
		++count;
	}

	if(hasSri())
	{
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*init.m_shadingRateImage.m_textureView);
		ANKI_ASSERT(view.getTextureImpl().usageValid(TextureUsageBit::kFramebufferShadingRate));
		imgViews[count] = view.getHandle();
		m_viewRefs.m_sri = init.m_shadingRateImage.m_textureView;
		++count;
	}

	ci.width = m_width;
	ci.height = m_height;

	ci.pAttachments = &imgViews[0];
	ANKI_ASSERT(count == ci.attachmentCount);

	ANKI_VK_CHECK(vkCreateFramebuffer(getDevice(), &ci, nullptr, &m_fbHandle));
	getGrManagerImpl().trySetVulkanHandleName(init.getName(), VK_OBJECT_TYPE_FRAMEBUFFER, m_fbHandle);

	return Error::kNone;
}

void FramebufferImpl::setupAttachmentDescriptor(const FramebufferAttachmentInfo& att, VkAttachmentDescription2& desc,
												VkImageLayout layout) const
{
	desc = {};
	desc.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
	desc.format = convertFormat(static_cast<const TextureViewImpl&>(*att.m_textureView).getTextureImpl().getFormat());
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
	for(U32 i = 0; i < init.m_colorAttachmentCount; ++i)
	{
		setupAttachmentDescriptor(init.m_colorAttachments[i], m_attachmentDescriptions[i],
								  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		VkAttachmentReference2& ref = m_references[i];
		ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		ref.attachment = i;
		ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if(hasDepthStencil())
	{
		setupAttachmentDescriptor(init.m_depthStencilAttachment, m_attachmentDescriptions[init.m_colorAttachmentCount],
								  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		VkAttachmentReference2& ref = m_references[init.m_colorAttachmentCount];
		ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		ref.attachment = init.m_colorAttachmentCount;
		ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}

	U32 sriAttachmentIdx = kMaxU32;
	if(init.m_shadingRateImage.m_textureView)
	{
		ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_vrs && "This requires VRS to be enabled");

		sriAttachmentIdx = init.m_colorAttachmentCount + hasDepthStencil();

		VkAttachmentDescription2& desc = m_attachmentDescriptions[sriAttachmentIdx];
		desc = {};
		desc.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		desc.format = convertFormat(
			static_cast<const TextureViewImpl&>(*init.m_shadingRateImage.m_textureView).getTextureImpl().getFormat());
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
		desc.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;

		VkAttachmentReference2& ref = m_references[sriAttachmentIdx];
		ref.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		ref.attachment = sriAttachmentIdx;
		ref.layout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
	}

	// Subpass
	m_subpassDescr.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
	m_subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	m_subpassDescr.colorAttachmentCount = init.m_colorAttachmentCount;
	m_subpassDescr.pColorAttachments = (init.m_colorAttachmentCount) ? &m_references[0] : nullptr;
	m_subpassDescr.pDepthStencilAttachment = (hasDepthStencil()) ? &m_references[init.m_colorAttachmentCount] : nullptr;

	if(init.m_shadingRateImage.m_textureView)
	{
		m_sriAttachmentInfo.sType = VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
		m_sriAttachmentInfo.shadingRateAttachmentTexelSize.width = init.m_shadingRateImage.m_texelWidth;
		m_sriAttachmentInfo.shadingRateAttachmentTexelSize.height = init.m_shadingRateImage.m_texelHeight;
		m_sriAttachmentInfo.pFragmentShadingRateAttachment = &m_references[sriAttachmentIdx];

		m_subpassDescr.pNext = &m_sriAttachmentInfo;
	}

	// Setup the render pass
	m_rpassCi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	m_rpassCi.pAttachments = &m_attachmentDescriptions[0];
	m_rpassCi.attachmentCount = getTotalAttachmentCount();
	m_rpassCi.subpassCount = 1;
	m_rpassCi.pSubpasses = &m_subpassDescr;
}

VkRenderPass FramebufferImpl::getRenderPassHandle(const Array<VkImageLayout, kMaxColorRenderTargets>& colorLayouts,
												  VkImageLayout dsLayout, VkImageLayout shadingRateImageLayout)
{
	VkRenderPass out = VK_NULL_HANDLE;

	// Create hash
	Array<VkImageLayout, kMaxAttachments> allLayouts;
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

	if(hasSri())
	{
		ANKI_ASSERT(shadingRateImageLayout != VK_IMAGE_LAYOUT_UNDEFINED);
		allLayouts[allLayoutCount++] = shadingRateImageLayout;
	}

	const U64 hash = computeHash(&allLayouts[0], allLayoutCount * sizeof(allLayouts[0]));

	// Try get it
	{
		RLockGuard<RWMutex> lock(m_renderpassHandlesMtx);
		auto it = m_renderpassHandles.find(hash);
		if(it != m_renderpassHandles.getEnd())
		{
			out = *it;
		}
	}

	if(out == VK_NULL_HANDLE)
	{
		// Create it

		WLockGuard<RWMutex> lock(m_renderpassHandlesMtx);

		// Check again
		auto it = m_renderpassHandles.find(hash);
		if(it != m_renderpassHandles.getEnd())
		{
			out = *it;
		}
		else
		{
			VkRenderPassCreateInfo2 ci = m_rpassCi;
			Array<VkAttachmentDescription2, kMaxAttachments> attachmentDescriptions = m_attachmentDescriptions;
			Array<VkAttachmentReference2, kMaxAttachments> references = m_references;
			VkSubpassDescription2 subpassDescr = m_subpassDescr;
			VkFragmentShadingRateAttachmentInfoKHR sriAttachmentInfo = m_sriAttachmentInfo;

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

				references[i].layout = lay;
				subpassDescr.pDepthStencilAttachment = &references[i];
			}

			if(hasSri())
			{
				const U i = subpassDescr.colorAttachmentCount + hasDepthStencil();
				const VkImageLayout lay = shadingRateImageLayout;

				attachmentDescriptions[i].initialLayout = lay;
				attachmentDescriptions[i].finalLayout = lay;

				references[i].layout = lay;

				sriAttachmentInfo.pFragmentShadingRateAttachment = &references[i];
				subpassDescr.pNext = &sriAttachmentInfo;
			}

			ANKI_VK_CHECKF(vkCreateRenderPass2KHR(getDevice(), &ci, nullptr, &out));
			getGrManagerImpl().trySetVulkanHandleName(getName(), VK_OBJECT_TYPE_RENDER_PASS, out);

			m_renderpassHandles.emplace(getMemoryPool(), hash, out);
		}
	}

	ANKI_ASSERT(out);
	return out;
}

} // end namespace anki

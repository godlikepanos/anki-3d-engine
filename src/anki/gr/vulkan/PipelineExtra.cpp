// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineExtra.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/Pipeline.h>

namespace anki
{

void CompatibleRenderPassCreator::destroy()
{
	for(VkRenderPass pass : m_hashmap)
	{
		ANKI_ASSERT(pass);
		vkDestroyRenderPass(m_manager->getDevice(), pass, nullptr);
	}

	m_hashmap.destroy(m_manager->getAllocator());
}

VkRenderPass CompatibleRenderPassCreator::getOrCreateCompatibleRenderPass(const PipelineInitInfo& init)
{
	VkRenderPass out = VK_NULL_HANDLE;

	// Create the key
	RenderPassKey key;
	for(U i = 0; i < init.m_color.m_attachmentCount; ++i)
	{
		key.m_colorAttachments[i] = init.m_color.m_attachments[i].m_format;
	}
	key.m_depthStencilAttachment = init.m_depthStencil.m_format;

	// Lock
	LockGuard<Mutex> lock(m_mtx);

	auto it = m_hashmap.find(key);
	if(it != m_hashmap.getEnd())
	{
		// Found the key
		out = *it;
	}
	else
	{
		// Not found, create one
		VkRenderPass rpass = createNewRenderPass(init);

		m_hashmap.pushBack(m_manager->getAllocator(), key, rpass);
		out = rpass;
	}

	ANKI_ASSERT(out);
	return out;
}

VkRenderPass CompatibleRenderPassCreator::createNewRenderPass(const PipelineInitInfo& init)
{
	VkRenderPassCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> attachmentDescriptions;
	memset(&attachmentDescriptions[0], 0, sizeof(attachmentDescriptions));

	Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS> references;
	memset(&references[0], 0, sizeof(references));

	for(U i = 0; i < init.m_color.m_attachmentCount; ++i)
	{
		// We only care about samples and format

		// Workaround unsupported formats
		VkFormat fmt;
		if(init.m_color.m_attachments[i].m_format.m_components == ComponentFormat::R8G8B8
			&& !m_manager->getR8g8b8ImagesSupported())
		{
			PixelFormat newFmt = init.m_color.m_attachments[i].m_format;
			newFmt.m_components = ComponentFormat::R8G8B8A8;
			fmt = convertFormat(newFmt);
		}
		else if(init.m_color.m_attachments[i].m_format.m_components == ComponentFormat::DEFAULT_FRAMEBUFFER)
		{
			// Default FB
			fmt = m_manager->getDefaultFramebufferSurfaceFormat();
		}
		else
		{
			fmt = convertFormat(init.m_color.m_attachments[i].m_format);
		}

		VkAttachmentDescription& desc = attachmentDescriptions[i];
		desc.format = fmt;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

		references[i].attachment = i;
		references[i].layout = VK_IMAGE_LAYOUT_GENERAL;
	}

	ci.attachmentCount = init.m_color.m_attachmentCount;

	Bool hasDepthStencil = init.m_depthStencil.m_format.m_components != ComponentFormat::NONE;
	VkAttachmentReference dsReference = {0, VK_IMAGE_LAYOUT_GENERAL};
	if(hasDepthStencil)
	{
		// Workaround unsupported formats
		VkFormat fmt;
		if(init.m_depthStencil.m_format.m_components == ComponentFormat::S8 && !m_manager->getS8ImagesSupported())
		{
			PixelFormat newFmt = PixelFormat(ComponentFormat::D24S8, TransformFormat::UNORM);
			fmt = convertFormat(newFmt);
		}
		else
		{
			fmt = convertFormat(init.m_depthStencil.m_format);
		}

		VkAttachmentDescription& desc = attachmentDescriptions[ci.attachmentCount];
		desc.format = fmt;
		desc.samples = VK_SAMPLE_COUNT_1_BIT;
		desc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

		dsReference.attachment = ci.attachmentCount;
		dsReference.layout = VK_IMAGE_LAYOUT_GENERAL;

		++ci.attachmentCount;
	}

	VkSubpassDescription desc = {};
	desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	desc.colorAttachmentCount = init.m_color.m_attachmentCount;
	desc.pColorAttachments = (init.m_color.m_attachmentCount) ? &references[0] : nullptr;
	desc.pDepthStencilAttachment = (hasDepthStencil) ? &dsReference : nullptr;

	ANKI_ASSERT(ci.attachmentCount);
	ci.pAttachments = &attachmentDescriptions[0];
	ci.subpassCount = 1;
	ci.pSubpasses = &desc;

	VkRenderPass rpass;
	ANKI_VK_CHECKF(vkCreateRenderPass(m_manager->getDevice(), &ci, nullptr, &rpass));

	return rpass;
}

} // end namespace anki

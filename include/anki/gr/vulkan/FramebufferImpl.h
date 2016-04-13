// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

// Forward
class Attachment;

/// @addtogroup vulkan
/// @{

/// Framebuffer implementation.
class FramebufferImpl : public VulkanObject
{
public:
	VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
	VkRenderPass m_renderPass = VK_NULL_HANDLE;

	FramebufferImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~FramebufferImpl();

	void init(const FramebufferInitInfo& init);

private:
	void initRenderPass(const FramebufferInitInfo& init);

	void setupAttachmentDescriptor(
		const Attachment& in, VkAttachmentDescription& out, Bool depthStencil);

	void initFramebuffer(const FramebufferInitInfo& init);
};
/// @}

} // end namespace anki

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
	Bool8 m_defaultFramebuffer = false;

	FramebufferImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~FramebufferImpl();

	ANKI_USE_RESULT Error init(const FramebufferInitInfo& init);

private:
	ANKI_USE_RESULT Error initRenderPass(const FramebufferInitInfo& init);

	void setupAttachmentDescriptor(
		const Attachment& in, VkAttachmentDescription& out, Bool depthStencil);

	ANKI_USE_RESULT Error initFramebuffer(const FramebufferInitInfo& init);
};
/// @}

} // end namespace anki

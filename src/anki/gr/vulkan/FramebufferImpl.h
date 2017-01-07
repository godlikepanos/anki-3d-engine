// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/util/HashMap.h>

namespace anki
{

// Forward
class FramebufferAttachmentInfo;

/// @addtogroup vulkan
/// @{

/// Framebuffer implementation.
class FramebufferImpl : public VulkanObject
{
public:
	FramebufferImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~FramebufferImpl();

	ANKI_USE_RESULT Error init(const FramebufferInitInfo& init);

	/// Good for pipeline creation.
	VkRenderPass getCompatibleRenderPass() const
	{
		ANKI_ASSERT(m_rpass);
		return m_rpass;
	}

	/// Use it for binding.
	VkRenderPass getRenderPass(WeakArray<TextureUsageBit> usages);

	VkFramebuffer getFramebuffer(U frame) const
	{
		ANKI_ASSERT(m_fbs[frame]);
		return m_fbs[frame];
	}

private:
	class Hasher
	{
	public:
		U64 operator()(const U64 key) const
		{
			return key;
		}
	};

	U32 m_width = 0;
	U32 m_height = 0;

	Bool8 m_defaultFb = false;

	Array<TexturePtr, MAX_COLOR_ATTACHMENTS + 1> m_refs; ///< @note The pos of every attachment is fixed.
	Array<U32, MAX_COLOR_ATTACHMENTS + 1> m_attachedMipLevels = {};

	// RenderPass create info
	VkRenderPassCreateInfo m_rpassCi = {};
	Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> m_attachmentDescriptions;
	Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS + 1> m_references;
	VkSubpassDescription m_subpassDescr = {};

	// VK objects
	VkRenderPass m_rpass = {}; ///< Compatible renderpass or default FB's renderpass.
	HashMap<U64, VkRenderPass, Hasher> m_rpasses;
	Mutex m_rpassesMtx;
	Array<VkFramebuffer, MAX_FRAMES_IN_FLIGHT> m_fbs = {};

	// Methods
	ANKI_USE_RESULT Error initFbs(const FramebufferInitInfo& init);
	void initRpassCreateInfo(const FramebufferInitInfo& init);
	void setupAttachmentDescriptor(const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc) const;
};
/// @}

} // end namespace anki

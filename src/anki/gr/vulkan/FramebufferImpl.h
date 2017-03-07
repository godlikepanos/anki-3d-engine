// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/util/HashMap.h>
#include <anki/util/BitSet.h>

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
		ANKI_ASSERT(m_compatibleOrDefaultRpass);
		return m_compatibleOrDefaultRpass;
	}

	/// Use it for binding.
	VkRenderPass getRenderPassHandle(
		const Array<VkImageLayout, MAX_COLOR_ATTACHMENTS>& colorLayouts, VkImageLayout dsLayout);

	VkFramebuffer getFramebufferHandle(U frame) const
	{
		ANKI_ASSERT(m_fbs[frame]);
		return m_fbs[frame];
	}

	void getAttachmentInfo(BitSet<MAX_COLOR_ATTACHMENTS, U8>& colorAttachments, Bool& depth, Bool& stencil) const
	{
		colorAttachments = m_colorAttachmentMask;
		depth = m_depthAttachment;
		stencil = m_stencilAttachment;
	}

	U getColorAttachmentCount() const
	{
		return m_colorAttCount;
	}

	Bool hasDepthStencil() const
	{
		return m_refs[MAX_COLOR_ATTACHMENTS].isCreated();
	}

	U getAttachmentCount() const
	{
		return m_colorAttCount + (hasDepthStencil() ? 1 : 0);
	}

	TexturePtr getColorAttachment(U att) const
	{
		ANKI_ASSERT(m_refs[att].get());
		return m_refs[att];
	}

	TexturePtr getDepthStencilAttachment() const
	{
		ANKI_ASSERT(m_refs[MAX_COLOR_ATTACHMENTS].get());
		return m_refs[MAX_COLOR_ATTACHMENTS];
	}

	const VkClearValue* getClearValues() const
	{
		return &m_clearVals[0];
	}

	Bool isDefaultFramebuffer() const
	{
		return m_defaultFb;
	}

	void getAttachmentsSize(U32& width, U32& height) const
	{
		width = m_width;
		height = m_height;
	}

	const Array<TextureSurfaceInfo, MAX_COLOR_ATTACHMENTS + 1>& getAttachedSurfaces() const
	{
		return m_attachedSurfaces;
	}

private:
	U32 m_width = 0;
	U32 m_height = 0;

	Bool8 m_defaultFb = false;

	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colorAttachmentMask = {false};
	Bool8 m_depthAttachment = false;
	Bool8 m_stencilAttachment = false;

	U8 m_colorAttCount = 0;
	Array<VkClearValue, MAX_COLOR_ATTACHMENTS + 1> m_clearVals;

	Array<TexturePtr, MAX_COLOR_ATTACHMENTS + 1> m_refs; ///< @note The pos of every attachment is fixed.
	Array<TextureSurfaceInfo, MAX_COLOR_ATTACHMENTS + 1> m_attachedSurfaces = {};

	// RenderPass create info
	VkRenderPassCreateInfo m_rpassCi = {};
	Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> m_attachmentDescriptions = {};
	Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS + 1> m_references = {};
	VkSubpassDescription m_subpassDescr = {};

	// VK objects
	VkRenderPass m_compatibleOrDefaultRpass = {}; ///< Compatible renderpass or default FB's renderpass.
	HashMap<U64, VkRenderPass> m_rpasses;
	Mutex m_rpassesMtx;
	Array<VkFramebuffer, MAX_FRAMES_IN_FLIGHT> m_fbs = {};

	// Methods
	ANKI_USE_RESULT Error initFbs(const FramebufferInitInfo& init);
	void initRpassCreateInfo(const FramebufferInitInfo& init);
	void setupAttachmentDescriptor(const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc) const;
};
/// @}

} // end namespace anki

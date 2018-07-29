// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/SwapchainFactory.h>
#include <anki/util/HashMap.h>
#include <anki/util/BitSet.h>

namespace anki
{

// Forward
class FramebufferAttachmentInfo;

/// @addtogroup vulkan
/// @{

/// Framebuffer implementation.
class FramebufferImpl final : public Framebuffer, public VulkanObject<Framebuffer, FramebufferImpl>
{
public:
	FramebufferImpl(GrManager* manager, CString name)
		: Framebuffer(manager, name)
	{
	}

	~FramebufferImpl();

	ANKI_USE_RESULT Error init(const FramebufferInitInfo& init);

	/// Good for pipeline creation.
	VkRenderPass getCompatibleRenderPass() const
	{
		ANKI_ASSERT(m_compatibleRpass);
		return m_compatibleRpass;
	}

	/// Use it for binding. It's thread-safe
	VkRenderPass getRenderPassHandle(
		const Array<VkImageLayout, MAX_COLOR_ATTACHMENTS>& colorLayouts, VkImageLayout dsLayout);

	VkFramebuffer getFramebufferHandle() const
	{
		ANKI_ASSERT(m_fb);
		return m_fb;
	}

	void getAttachmentInfo(BitSet<MAX_COLOR_ATTACHMENTS, U8>& colorAttachments, Bool& depth, Bool& stencil) const
	{
		colorAttachments = m_colorAttachmentMask;
		depth = !!(m_aspect & DepthStencilAspectBit::DEPTH);
		stencil = !!(m_aspect & DepthStencilAspectBit::STENCIL);
	}

	U getColorAttachmentCount() const
	{
		return m_colorAttCount;
	}

	Bool hasDepthStencil() const
	{
		return !!m_aspect;
	}

	U getAttachmentCount() const
	{
		return m_colorAttCount + (hasDepthStencil() ? 1 : 0);
	}

	const TextureViewPtr& getColorAttachment(U att) const
	{
		ANKI_ASSERT(m_refs[att].get());
		return m_refs[att];
	}

	const TextureViewPtr& getDepthStencilAttachment() const
	{
		ANKI_ASSERT(m_refs[MAX_COLOR_ATTACHMENTS].get());
		return m_refs[MAX_COLOR_ATTACHMENTS];
	}

	const VkClearValue* getClearValues() const
	{
		return &m_clearVals[0];
	}

	void getAttachmentsSize(U32& width, U32& height) const
	{
		ANKI_ASSERT(m_width != 0 && m_height != 0);
		width = m_width;
		height = m_height;
	}

	Bool hasPresentableTexture() const
	{
		return m_presentableTex;
	}

private:
	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colorAttachmentMask = {false};
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE;

	U8 m_colorAttCount = 0;
	Array<VkClearValue, MAX_COLOR_ATTACHMENTS + 1> m_clearVals;

	U32 m_width = 0;
	U32 m_height = 0;
	Bool8 m_presentableTex = false;

	Array<TextureViewPtr, MAX_COLOR_ATTACHMENTS + 1> m_refs; ///< @note The pos of every attachment is fixed.

	// RenderPass create info
	VkRenderPassCreateInfo m_rpassCi = {};
	Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> m_attachmentDescriptions = {};
	Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS + 1> m_references = {};
	VkSubpassDescription m_subpassDescr = {};

	// VK objects
	VkRenderPass m_compatibleRpass = {}; ///< Compatible renderpass.
	HashMap<U64, VkRenderPass> m_rpasses;
	Mutex m_rpassesMtx;
	VkFramebuffer m_fb = VK_NULL_HANDLE;

	// Methods
	ANKI_USE_RESULT Error initFbs(const FramebufferInitInfo& init);
	void initRpassCreateInfo(const FramebufferInitInfo& init);
	void initClearValues(const FramebufferInitInfo& init);
	void setupAttachmentDescriptor(
		const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc, VkImageLayout layout) const;
};
/// @}

} // end namespace anki

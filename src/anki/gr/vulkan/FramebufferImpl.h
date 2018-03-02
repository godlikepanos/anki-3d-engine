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
		if(!m_defaultFb)
		{
			ANKI_ASSERT(m_noDflt.m_compatibleRpass);
			return m_noDflt.m_compatibleRpass;
		}
		else
		{
			return m_dflt.m_swapchain->getRenderPass(m_dflt.m_loadOp);
		}
	}

	/// Sync it before you bind it. It's thread-safe
	void sync();

	/// Use it for binding. It's thread-safe
	VkRenderPass getRenderPassHandle(
		const Array<VkImageLayout, MAX_COLOR_ATTACHMENTS>& colorLayouts, VkImageLayout dsLayout);

	VkFramebuffer getFramebufferHandle(U frame) const
	{
		if(!m_defaultFb)
		{
			ANKI_ASSERT(m_noDflt.m_fb);
			return m_noDflt.m_fb;
		}
		else
		{
			return m_dflt.m_swapchain->m_framebuffers[frame];
		}
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

	TextureViewPtr getColorAttachment(U att) const
	{
		ANKI_ASSERT(m_noDflt.m_refs[att].get());
		return m_noDflt.m_refs[att];
	}

	TextureViewPtr getDepthStencilAttachment() const
	{
		ANKI_ASSERT(m_noDflt.m_refs[MAX_COLOR_ATTACHMENTS].get());
		return m_noDflt.m_refs[MAX_COLOR_ATTACHMENTS];
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
		if(!m_defaultFb)
		{
			ANKI_ASSERT(m_noDflt.m_width != 0 && m_noDflt.m_height != 0);
			width = m_noDflt.m_width;
			height = m_noDflt.m_height;
		}
		else
		{
			width = m_dflt.m_swapchain->m_surfaceWidth;
			height = m_dflt.m_swapchain->m_surfaceHeight;
		}
	}

	void getDefaultFramebufferInfo(MicroSwapchainPtr& swapchain, U32& crntBackBufferIdx) const
	{
		ANKI_ASSERT(m_defaultFb);
		swapchain = m_dflt.m_swapchain;
		crntBackBufferIdx = m_dflt.m_currentBackbufferIndex;
	}

private:
	Bool8 m_defaultFb = false;

	BitSet<MAX_COLOR_ATTACHMENTS, U8> m_colorAttachmentMask = {false};
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE;

	U8 m_colorAttCount = 0;
	Array<VkClearValue, MAX_COLOR_ATTACHMENTS + 1> m_clearVals;

	class
	{
	public:
		U32 m_width = 0;
		U32 m_height = 0;

		Array<TextureViewPtr, MAX_COLOR_ATTACHMENTS + 1> m_refs; ///< @note The pos of every attachment is fixed.

		// RenderPass create info
		VkRenderPassCreateInfo m_rpassCi = {};
		Array<VkAttachmentDescription, MAX_COLOR_ATTACHMENTS + 1> m_attachmentDescriptions = {};
		Array<VkAttachmentReference, MAX_COLOR_ATTACHMENTS + 1> m_references = {};
		VkSubpassDescription m_subpassDescr = {};

		// VK objects
		VkRenderPass m_compatibleRpass = {}; ///< Compatible renderpass or default FB's renderpass.
		HashMap<U64, VkRenderPass> m_rpasses;
		Mutex m_rpassesMtx;
		VkFramebuffer m_fb = {};
	} m_noDflt; ///< Not default FB

	class
	{
	public:
		MicroSwapchainPtr m_swapchain;
		SpinLock m_swapchainLock;
		VkAttachmentLoadOp m_loadOp = {};
		U8 m_currentBackbufferIndex = 0;
	} m_dflt; ///< Default FB

	// Methods
	ANKI_USE_RESULT Error initFbs(const FramebufferInitInfo& init);
	void initRpassCreateInfo(const FramebufferInitInfo& init);
	void initClearValues(const FramebufferInitInfo& init);
	void setupAttachmentDescriptor(
		const FramebufferAttachmentInfo& att, VkAttachmentDescription& desc, VkImageLayout layout) const;
};
/// @}

} // end namespace anki

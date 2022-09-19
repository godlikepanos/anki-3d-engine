// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Framebuffer.h>
#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/Vulkan/SwapchainFactory.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

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

	Error init(const FramebufferInitInfo& init);

	/// Good for pipeline creation.
	VkRenderPass getCompatibleRenderPass() const
	{
		ANKI_ASSERT(m_compatibleRenderpassHandle);
		return m_compatibleRenderpassHandle;
	}

	/// Use it for binding. It's thread-safe
	VkRenderPass getRenderPassHandle(const Array<VkImageLayout, kMaxColorRenderTargets>& colorLayouts,
									 VkImageLayout dsLayout, VkImageLayout shadingRateImageLayout);

	VkFramebuffer getFramebufferHandle() const
	{
		ANKI_ASSERT(m_fbHandle);
		return m_fbHandle;
	}

	void getAttachmentInfo(BitSet<kMaxColorRenderTargets, U8>& colorAttachments, Bool& depth, Bool& stencil) const
	{
		colorAttachments = m_colorAttachmentMask;
		depth = !!(m_aspect & DepthStencilAspectBit::kDepth);
		stencil = !!(m_aspect & DepthStencilAspectBit::kStencil);
	}

	U32 getColorAttachmentCount() const
	{
		return m_colorAttCount;
	}

	Bool hasDepthStencil() const
	{
		return !!m_aspect;
	}

	U32 getAttachmentCount() const
	{
		return getTotalAttachmentCount();
	}

	const TextureViewPtr& getColorAttachment(U att) const
	{
		return m_viewRefs.m_color[att];
	}

	const TextureViewPtr& getDepthStencilAttachment() const
	{
		return m_viewRefs.m_depthStencil;
	}

	const TextureViewPtr& getSriAttachment() const
	{
		return m_viewRefs.m_sri;
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

	Bool hasSri() const
	{
		return m_hasSri;
	}

private:
	static constexpr U32 kMaxAttachments = kMaxColorRenderTargets + 2; ///< Color + depth/stencil + SRI

	BitSet<kMaxColorRenderTargets, U8> m_colorAttachmentMask = {false};
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::kNone;

	U8 m_colorAttCount = 0;
	Array<VkClearValue, kMaxAttachments> m_clearVals;

	U32 m_width = 0;
	U32 m_height = 0;
	Bool m_presentableTex = false;
	Bool m_hasSri = false;

	class
	{
	public:
		Array<TextureViewPtr, kMaxColorRenderTargets> m_color;
		TextureViewPtr m_depthStencil;
		TextureViewPtr m_sri;
	} m_viewRefs;

	// VK objects
	VkRenderPass m_compatibleRenderpassHandle = VK_NULL_HANDLE; ///< Compatible renderpass. Good for pipeline creation.
	HashMap<U64, VkRenderPass> m_renderpassHandles;
	RWMutex m_renderpassHandlesMtx;
	VkFramebuffer m_fbHandle = VK_NULL_HANDLE;

	// RenderPass create info
	VkRenderPassCreateInfo2 m_rpassCi = {};
	Array<VkAttachmentDescription2, kMaxAttachments> m_attachmentDescriptions = {};
	Array<VkAttachmentReference2, kMaxAttachments> m_references = {};
	VkSubpassDescription2 m_subpassDescr = {};
	VkFragmentShadingRateAttachmentInfoKHR m_sriAttachmentInfo = {};

	// Methods
	Error initFbs(const FramebufferInitInfo& init);
	void initRpassCreateInfo(const FramebufferInitInfo& init);
	void initClearValues(const FramebufferInitInfo& init);
	void setupAttachmentDescriptor(const FramebufferAttachmentInfo& att, VkAttachmentDescription2& desc,
								   VkImageLayout layout) const;

	U32 getTotalAttachmentCount() const
	{
		return m_colorAttCount + hasDepthStencil() + hasSri();
	}
};
/// @}

} // end namespace anki

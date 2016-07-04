// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/CommandBuffer.h>

#include <anki/gr/vulkan/TextureImpl.h>

#include <anki/util/List.h>

namespace anki
{

// Forward
class CommandBufferInitInfo;

/// @addtogroup vulkan
/// @{

/// Command buffer implementation.
class CommandBufferImpl : public VulkanObject
{
public:
	/// Default constructor
	CommandBufferImpl(GrManager* manager);

	~CommandBufferImpl();

	ANKI_USE_RESULT Error init(const CommandBufferInitInfo& init);

	VkCommandBuffer getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	Bool renderedToDefaultFramebuffer() const
	{
		return m_renderedToDefaultFb;
	}

	Bool isTheFirstFramebufferOfTheFrame() const
	{
		return (m_flags & CommandBufferFlag::FRAME_FIRST)
			== CommandBufferFlag::FRAME_FIRST;
	}

	Bool isTheLastFramebufferOfTheFrame() const
	{
		return (m_flags & CommandBufferFlag::FRAME_LAST)
			== CommandBufferFlag::FRAME_LAST;
	}

	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
	{
		commandCommon();
		ANKI_ASSERT(minx < maxx && miny < maxy);
		VkViewport s;
		s.x = minx;
		s.y = miny;
		s.width = maxx - minx;
		s.height = maxy - miny;
		vkCmdSetViewport(m_handle, 0, 1, &s);

		VkRect2D scissor = {};
		scissor.extent.width = maxx - minx;
		scissor.extent.height = maxy - miny;
		scissor.offset.x = minx;
		scissor.offset.y = miny;
		vkCmdSetScissor(m_handle, 0, 1, &scissor);
	}

	void bindPipeline(PipelinePtr ppline);

	void beginRenderPass(FramebufferPtr fb);

	void endRenderPass();

	void bindResourceGroup(
		ResourceGroupPtr rc, U slot, const TransientMemoryInfo* dynInfo);

	void drawArrays(U32 count, U32 instanceCount, U32 first, U32 baseInstance)
	{
		drawcallCommon();
		vkCmdDraw(m_handle, count, instanceCount, first, baseInstance);
	}

	void uploadTextureSurface(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const TransientMemoryToken& token);

	void endRecording();

	void setImageBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkImageLayout prevLayout,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		VkImageLayout newLayout,
		VkImage img,
		const VkImageSubresourceRange& range)
	{
		ANKI_ASSERT(img);
		commandCommon();

		VkImageMemoryBarrier inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		inf.srcAccessMask = srcAccess;
		inf.dstAccessMask = dstAccess;
		inf.oldLayout = prevLayout;
		inf.newLayout = newLayout;
		inf.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		inf.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		inf.image = img;
		inf.subresourceRange = range;

		vkCmdPipelineBarrier(
			m_handle, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &inf);
	}

	void setImageBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkImageLayout prevLayout,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		VkImageLayout newLayout,
		TexturePtr tex,
		const VkImageSubresourceRange& range)
	{
		setImageBarrier(srcStage,
			srcAccess,
			prevLayout,
			dstStage,
			dstAccess,
			newLayout,
			tex->getImplementation().m_imageHandle,
			range);

		m_texList.pushBack(m_alloc, tex);
	}

private:
	StackAllocator<U8> m_alloc;

	VkCommandBuffer m_handle = VK_NULL_HANDLE;
	CommandBufferFlag m_flags = CommandBufferFlag::NONE;
	Bool8 m_renderedToDefaultFb = false;
	Bool8 m_finalized = false;
	Bool8 m_empty = true;
	Thread::Id m_tid = 0;

	Bool m_firstRpassDrawcall = true; ///< First drawcall in a renderpass.
	FramebufferPtr m_activeFb;

	/// @name cleanup_references
	/// @{
	List<PipelinePtr> m_pplineList;
	List<FramebufferPtr> m_fbList;
	List<ResourceGroupPtr> m_rcList;
	List<TexturePtr> m_texList;
/// @}

#if ANKI_ASSERTIONS
	// Debug stuff
	Bool8 m_insideRenderPass = false;
	VkSubpassContents m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;
#endif

	/// Some common operations per command.
	void commandCommon();

	void drawcallCommon();

	Bool insideRenderPass() const
	{
		return m_activeFb.isCreated();
	}
};
/// @}

} // end namespace anki

// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/CommandBuffer.h>
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

	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy);

	void setPolygonOffset(F32 factor, F32 units);

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

	void drawElements(U32 count,
		U32 instanceCount,
		U32 firstIndex,
		U32 baseVertex,
		U32 baseInstance);

	void beginOcclusionQuery(OcclusionQueryPtr query);

	void endOcclusionQuery(OcclusionQueryPtr query);

	void uploadTextureSurface(TexturePtr tex,
		const TextureSurfaceInfo& surf,
		const TransientMemoryToken& token);

	void generateMipmaps(TexturePtr tex, U depth, U face, U layer);

	void endRecording();

	void setImageBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkImageLayout prevLayout,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		VkImageLayout newLayout,
		VkImage img,
		const VkImageSubresourceRange& range);

	void setImageBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkImageLayout prevLayout,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		VkImageLayout newLayout,
		TexturePtr tex,
		const VkImageSubresourceRange& range);

	void setImageBarrier(TexturePtr tex,
		TextureUsageBit prevUsage,
		TextureUsageBit nextUsage,
		const TextureSurfaceInfo& surf);

private:
	StackAllocator<U8> m_alloc;

	VkCommandBuffer m_handle = VK_NULL_HANDLE;
	CommandBufferFlag m_flags = CommandBufferFlag::NONE;
	Bool8 m_renderedToDefaultFb = false;
	Bool8 m_finalized = false;
	Bool8 m_empty = true;
	Thread::Id m_tid = 0;

	U m_rpDrawcallCount = 0; ///< Number of drawcalls in renderpass.
	FramebufferPtr m_activeFb;

	/// @name cleanup_references
	/// @{
	List<PipelinePtr> m_pplineList;
	List<FramebufferPtr> m_fbList;
	List<ResourceGroupPtr> m_rcList;
	List<TexturePtr> m_texList;
	List<OcclusionQueryPtr> m_queryList;
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

	void beginRenderPassInternal();
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/CommandBufferImpl.inl.h>

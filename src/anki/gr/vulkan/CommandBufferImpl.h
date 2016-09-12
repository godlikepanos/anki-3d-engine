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

#define ANKI_BATCH_COMMANDS 1

// Forward
class CommandBufferInitInfo;

/// @addtogroup vulkan
/// @{

/// List the commands that can be batched.
enum class CommandBufferCommandType : U8
{
	SET_BARRIER,
	ANY_OTHER_COMMAND
};

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

	Bool isEmpty() const
	{
		return m_empty;
	}

	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy);

	void setPolygonOffset(F32 factor, F32 units);

	void bindPipeline(PipelinePtr ppline);

	void beginRenderPass(FramebufferPtr fb);

	void endRenderPass();

	void bindResourceGroup(ResourceGroupPtr rc, U slot, const TransientMemoryInfo* dynInfo);

	void drawArrays(U32 count, U32 instanceCount, U32 first, U32 baseInstance);

	void drawElements(U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance);

	void drawArraysIndirect(U32 drawCount, PtrSize offset, BufferPtr buff);

	void drawElementsIndirect(U32 drawCount, PtrSize offset, BufferPtr buff);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	void resetOcclusionQuery(OcclusionQueryPtr query);

	void beginOcclusionQuery(OcclusionQueryPtr query);

	void endOcclusionQuery(OcclusionQueryPtr query);

	void uploadTextureSurface(TexturePtr tex, const TextureSurfaceInfo& surf, const TransientMemoryToken& token);

	void uploadTextureVolume(TexturePtr tex, const TextureVolumeInfo& vol, const TransientMemoryToken& token);

	void generateMipmaps2d(TexturePtr tex, U face, U layer);

	void clearTexture(TexturePtr tex, const ClearValue& clearValue);

	void clearTextureSurface(TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& clearValue);

	void clearTextureVolume(TexturePtr tex, const TextureVolumeInfo& volume, const ClearValue& clearValue);

	void uploadBuffer(BufferPtr buff, PtrSize offset, const TransientMemoryToken& token);

	void pushSecondLevelCommandBuffer(CommandBufferPtr cmdb);

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

	void setTextureSurfaceBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf);

	void setTextureVolumeBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol);

	void setBufferBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		PtrSize offset,
		PtrSize size,
		VkBuffer buff);

	void setBufferBarrier(BufferPtr buff, BufferUsageBit before, BufferUsageBit after, PtrSize offset, PtrSize size);

	void fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value);

	void writeOcclusionQueryResultToBuffer(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff);

private:
	StackAllocator<U8> m_alloc;

	VkCommandBuffer m_handle = VK_NULL_HANDLE;
	CommandBufferFlag m_flags = CommandBufferFlag::NONE;
	Bool8 m_renderedToDefaultFb = false;
	Bool8 m_finalized = false;
	Bool8 m_empty = true;
	ThreadId m_tid = 0;

	U m_rpCommandCount = 0; ///< Number of drawcalls or pushed cmdbs in rp.
	FramebufferPtr m_activeFb;

	/// @name cleanup_references
	/// @{
	List<PipelinePtr> m_pplineList;
	List<FramebufferPtr> m_fbList;
	List<ResourceGroupPtr> m_rcList;
	List<TexturePtr> m_texList;
	List<OcclusionQueryPtr> m_queryList;
	List<BufferPtr> m_bufferList;
	List<CommandBufferPtr> m_cmdbList;
/// @}

#if ANKI_ASSERTIONS
	// Debug stuff
	Bool8 m_insideRenderPass = false;
#endif
	VkSubpassContents m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;

	CommandBufferCommandType m_lastCmdType = CommandBufferCommandType::ANY_OTHER_COMMAND;

	/// @name barrier_batch
	/// @{
	DynamicArray<VkImageMemoryBarrier> m_imgBarriers;
	DynamicArray<VkBufferMemoryBarrier> m_buffBarriers;
	U16 m_imgBarrierCount = 0;
	U16 m_buffBarrierCount = 0;
	VkPipelineStageFlags m_srcStageMask = 0;
	VkPipelineStageFlags m_dstStageMask = 0;
	/// @}

	/// Some common operations per command.
	void commandCommon(CommandBufferCommandType type);

	void drawcallCommon();

	Bool insideRenderPass() const
	{
		return m_activeFb.isCreated();
	}

	void beginRenderPassInternal();

	void endRecordingInternal();

	Bool secondLevel() const
	{
		return !!(m_flags & CommandBufferFlag::SECOND_LEVEL);
	}

	/// Flush batched image and buffer barriers.
	void flushBarriers();

	void clearTextureInternal(TexturePtr tex, const ClearValue& clearValue, const VkImageSubresourceRange& range);

	void setTextureBarrierInternal(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const VkImageSubresourceRange& range);
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/CommandBufferImpl.inl.h>

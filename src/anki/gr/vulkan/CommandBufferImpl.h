// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/Pipeline.h>
#include <anki/util/List.h>

namespace anki
{

#define ANKI_BATCH_COMMANDS 1

// Forward
class CommandBufferInitInfo;

/// @addtogroup vulkan
/// @{

#define ANKI_CMD(x_, t_)                                                                                               \
	flushBatches(CommandBufferCommandType::t_);                                                                        \
	x_;

/// List the commands that can be batched.
enum class CommandBufferCommandType : U8
{
	SET_BARRIER,
	RESET_OCCLUSION_QUERY,
	WRITE_QUERY_RESULT,
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

	Bool isSecondLevel() const
	{
		return !!(m_flags & CommandBufferFlag::SECOND_LEVEL);
	}

	void bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride)
	{
		commandCommon();
		m_state.bindVertexBuffer(binding, stride);
		VkBuffer vkbuff = buff->m_impl->getHandle();
		ANKI_CMD(vkCmdBindVertexBuffers(m_handle, binding, 1, &vkbuff, &offset), ANY_OTHER_COMMAND);
		m_bufferList.pushBack(m_alloc, buff);
	}

	void setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset)
	{
		commandCommon();
		m_state.setVertexAttribute(location, buffBinding, fmt, relativeOffset);
	}

	void bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type)
	{
		commandCommon();
		ANKI_CMD(vkCmdBindIndexBuffer(m_handle, buff->m_impl->getHandle(), offset, convertIndexType(type)),
			ANY_OTHER_COMMAND);
		m_bufferList.pushBack(m_alloc, buff);
	}

	void setPrimitiveRestart(Bool enable);

	void setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
	{
		ANKI_ASSERT(minx < maxx && miny < maxy);
		commandCommon();

		if(m_viewport[0] != minx || m_viewport[1] != miny || m_viewport[2] != maxx || m_viewport[3] != maxy)
		{
			m_viewportDirty = true;

			m_viewport[0] = minx;
			m_viewport[1] = miny;
			m_viewport[2] = maxx;
			m_viewport[3] = maxy;
		}
	}

	void setPolygonOffset(F32 factor, F32 units);

	void setStencilCompareMask(FaceSelectionBit face, U32 mask);

	void setStencilWriteMask(FaceSelectionBit face, U32 mask);

	void setStencilReference(FaceSelectionBit face, U32 ref);

	void beginRenderPass(FramebufferPtr fb);

	void endRenderPass();

	void drawArrays(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance);

	void drawElements(
		PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance);

	void drawArraysIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr& buff);

	void drawElementsIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr& buff);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	void resetOcclusionQuery(OcclusionQueryPtr query);

	void beginOcclusionQuery(OcclusionQueryPtr query);

	void endOcclusionQuery(OcclusionQueryPtr query);

	void generateMipmaps2d(TexturePtr tex, U face, U layer);

	void clearTextureSurface(
		TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& clearValue, DepthStencilAspectBit aspect);

	void clearTextureVolume(
		TexturePtr tex, const TextureVolumeInfo& volume, const ClearValue& clearValue, DepthStencilAspectBit aspect);

	void pushSecondLevelCommandBuffer(CommandBufferPtr cmdb);

	void endRecording();

	void setTextureSurfaceBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf);

	void setTextureVolumeBarrier(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol);

	void setTextureBarrierRange(
		TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const VkImageSubresourceRange& range);

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

	void bindShaderProgram(ShaderProgramPtr& prog);

	void bindUniformBuffer(U32 set, U32 binding, BufferPtr& buff, PtrSize offset, PtrSize range)
	{
		commandCommon();
		U realBinding = MAX_TEXTURE_BINDINGS + binding;
		m_dsetState[set].bindUniformBuffer(realBinding, buff.get(), offset, range);
		m_bufferList.pushBack(m_alloc, buff);
	}

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

	ShaderProgramImpl* m_graphicsProg ANKI_DBG_NULLIFY; ///< Last bound graphics program

	PipelineStateTracker m_state;

	Array<DescriptorSetState, MAX_DESCRIPTOR_SETS> m_dsetState;

	/// @name cleanup_references
	/// @{
	List<FramebufferPtr> m_fbList;
	List<TexturePtr> m_texList;
	List<OcclusionQueryPtr> m_queryList;
	List<BufferPtr> m_bufferList;
	List<CommandBufferPtr> m_cmdbList;
	List<ShaderProgramPtr> m_progs;
/// @}

#if ANKI_EXTRA_CHECKS
	// Debug stuff
	Bool8 m_insideRenderPass = false;
#endif
	VkSubpassContents m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;

	CommandBufferCommandType m_lastCmdType = CommandBufferCommandType::ANY_OTHER_COMMAND;

	/// @name state_opts
	/// @{
	Array<U16, 4> m_viewport = {{0, 0, 0, 0}};
	Bool8 m_viewportDirty = true;
	F32 m_polygonOffsetFactor = MAX_F32;
	F32 m_polygonOffsetUnits = MAX_F32;
	Array<U32, 2> m_stencilCompareMasks = {{0x5A5A5A5A, 0x5A5A5A5A}}; ///< Use a stupid number to initialize.
	Array<U32, 2> m_stencilWriteMasks = {{0x5A5A5A5A, 0x5A5A5A5A}};
	Array<U32, 2> m_stencilReferenceMasks = {{0x5A5A5A5A, 0x5A5A5A5A}};
	/// @}

	/// @name barrier_batch
	/// @{
	DynamicArray<VkImageMemoryBarrier> m_imgBarriers;
	DynamicArray<VkBufferMemoryBarrier> m_buffBarriers;
	U16 m_imgBarrierCount = 0;
	U16 m_buffBarrierCount = 0;
	VkPipelineStageFlags m_srcStageMask = 0;
	VkPipelineStageFlags m_dstStageMask = 0;
	/// @}

	/// @name reset_query_batch
	/// @{
	class QueryResetAtom
	{
	public:
		VkQueryPool m_pool;
		U32 m_queryIdx;
	};

	DynamicArray<QueryResetAtom> m_queryResetAtoms;
	U16 m_queryResetAtomCount = 0;
	/// @}

	/// @name write_query_result_batch
	/// @{
	class WriteQueryAtom
	{
	public:
		VkQueryPool m_pool;
		U32 m_queryIdx;
		VkBuffer m_buffer;
		PtrSize m_offset;
	};

	DynamicArray<WriteQueryAtom> m_writeQueryAtoms;
	U16 m_writeQueryAtomCount = 0;
	/// @}

	/// Track texture usage.
	class TextureUsageTracker
	{
	public:
		Bool findUsage(const Texture& tex, TextureUsageBit& usage) const
		{
			auto it = m_map.find(tex.getUuid());
			if(it != m_map.getEnd())
			{
				usage = (*it);
				return true;
			}
			else if(tex.m_impl->m_usageWhenEncountered != TextureUsageBit::NONE)
			{
				usage = tex.m_impl->m_usageWhenEncountered;
				return true;
			}
			else
			{
				return false;
			}
		}

		void setUsage(const Texture& tex, TextureUsageBit usage, StackAllocator<U8>& alloc)
		{
			ANKI_ASSERT(usage != TextureUsageBit::NONE);
			auto it = m_map.find(tex.getUuid());
			if(it != m_map.getEnd())
			{
				(*it) = usage;
			}
			else
			{
				m_map.pushBack(alloc, tex.getUuid(), usage);
			}
		}

		void destroy(StackAllocator<U8>& alloc)
		{
			m_map.destroy(alloc);
		}

	private:
		HashMap<U64, TextureUsageBit> m_map;
	} m_texUsageTracker;

	/// Some common operations per command.
	void commandCommon();

	/// Flush batches. Use ANKI_CMD on every vkCmdXXX to do that automatically and call it manually before adding to a
	/// batch.
	void flushBatches(CommandBufferCommandType type);

	void drawcallCommon();

	Bool insideRenderPass() const
	{
		return m_activeFb.isCreated();
	}

	void beginRenderPassInternal();

	Bool secondLevel() const
	{
		return !!(m_flags & CommandBufferFlag::SECOND_LEVEL);
	}

	/// Flush batched image and buffer barriers.
	void flushBarriers();

	void flushQueryResets();

	void flushWriteQueryResults();

	void clearTextureInternal(TexturePtr tex, const ClearValue& clearValue, const VkImageSubresourceRange& range);

	void setImageBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkImageLayout prevLayout,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		VkImageLayout newLayout,
		VkImage img,
		const VkImageSubresourceRange& range);
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/CommandBufferImpl.inl.h>

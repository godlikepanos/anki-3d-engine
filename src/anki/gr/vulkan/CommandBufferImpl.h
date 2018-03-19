// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/CommandBufferFactory.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/Shader.h>
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

#define ANKI_CMD(x_, t_) \
	flushBatches(CommandBufferCommandType::t_); \
	x_;

/// List the commands that can be batched.
enum class CommandBufferCommandType : U8
{
	SET_BARRIER,
	RESET_OCCLUSION_QUERY,
	WRITE_QUERY_RESULT,
	PUSH_SECOND_LEVEL,
	ANY_OTHER_COMMAND
};

/// Command buffer implementation.
class CommandBufferImpl final : public CommandBuffer, public VulkanObject<CommandBuffer, CommandBufferImpl>
{
public:
	/// Default constructor
	CommandBufferImpl(GrManager* manager, CString name)
		: CommandBuffer(manager, name)
	{
	}

	~CommandBufferImpl();

	ANKI_USE_RESULT Error init(const CommandBufferInitInfo& init);

	void setFence(MicroFencePtr& fence)
	{
		m_microCmdb->setFence(fence);
	}

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

	void bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride, VertexStepRate stepRate)
	{
		commandCommon();
		m_state.bindVertexBuffer(binding, stride, stepRate);
		VkBuffer vkbuff = static_cast<const BufferImpl&>(*buff).getHandle();
		ANKI_CMD(vkCmdBindVertexBuffers(m_handle, binding, 1, &vkbuff, &offset), ANY_OTHER_COMMAND);
		m_microCmdb->pushObjectRef(buff);
	}

	void setVertexAttribute(U32 location, U32 buffBinding, const Format fmt, PtrSize relativeOffset)
	{
		commandCommon();
		m_state.setVertexAttribute(location, buffBinding, fmt, relativeOffset);
	}

	void bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type)
	{
		commandCommon();
		ANKI_CMD(vkCmdBindIndexBuffer(
					 m_handle, static_cast<const BufferImpl&>(*buff).getHandle(), offset, convertIndexType(type)),
			ANY_OTHER_COMMAND);
		m_microCmdb->pushObjectRef(buff);
	}

	void setPrimitiveRestart(Bool enable)
	{
		commandCommon();
		m_state.setPrimitiveRestart(enable);
	}

	void setFillMode(FillMode mode)
	{
		commandCommon();
		m_state.setFillMode(mode);
	}

	void setCullMode(FaceSelectionBit mode)
	{
		commandCommon();
		m_state.setCullMode(mode);
	}

	void setViewport(U32 minx, U32 miny, U32 width, U32 height)
	{
		ANKI_ASSERT(width > 0 && height > 0);
		commandCommon();

		if(m_viewport[0] != minx || m_viewport[1] != miny || m_viewport[2] != width || m_viewport[3] != height)
		{
			m_viewportDirty = true;

			m_viewport[0] = minx;
			m_viewport[1] = miny;
			m_viewport[2] = width;
			m_viewport[3] = height;
		}
	}

	void setScissor(U32 minx, U32 miny, U32 width, U32 height)
	{
		ANKI_ASSERT(width > 0 && height > 0);
		commandCommon();

		if(m_scissor[0] != minx || m_scissor[1] != miny || m_scissor[2] != width || m_scissor[3] != height)
		{
			m_scissorDirty = true;

			m_scissor[0] = minx;
			m_scissor[1] = miny;
			m_scissor[2] = width;
			m_scissor[3] = height;
		}
	}

	void setPolygonOffset(F32 factor, F32 units)
	{
		commandCommon();
		m_state.setPolygonOffset(factor, units);
	}

	void setStencilOperations(FaceSelectionBit face,
		StencilOperation stencilFail,
		StencilOperation stencilPassDepthFail,
		StencilOperation stencilPassDepthPass)
	{
		commandCommon();
		m_state.setStencilOperations(face, stencilFail, stencilPassDepthFail, stencilPassDepthPass);
	}

	void setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
	{
		commandCommon();
		m_state.setStencilCompareOperation(face, comp);
	}

	void setStencilCompareMask(FaceSelectionBit face, U32 mask);

	void setStencilWriteMask(FaceSelectionBit face, U32 mask);

	void setStencilReference(FaceSelectionBit face, U32 ref);

	void setDepthWrite(Bool enable)
	{
		commandCommon();
		m_state.setDepthWrite(enable);
	}

	void setDepthCompareOperation(CompareOperation op)
	{
		commandCommon();
		m_state.setDepthCompareOperation(op);
	}

	void setAlphaToCoverage(Bool enable)
	{
		commandCommon();
		m_state.setAlphaToCoverage(enable);
	}

	void setColorChannelWriteMask(U32 attachment, ColorBit mask)
	{
		commandCommon();
		m_state.setColorChannelWriteMask(attachment, mask);
	}

	void setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
	{
		commandCommon();
		m_state.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA);
	}

	void setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
	{
		commandCommon();
		m_state.setBlendOperation(attachment, funcRgb, funcA);
	}

	void bindTextureAndSamplerInternal(
		U32 set, U32 binding, TextureViewPtr& texView, SamplerPtr sampler, TextureUsageBit usage)
	{
		commandCommon();
		const U realBinding = binding;
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
		const TextureImpl& tex = static_cast<const TextureImpl&>(*view.m_tex);
		ANKI_ASSERT(tex.isSubresourceGoodForSampling(view.getSubresource()));
		const VkImageLayout lay = tex.computeLayout(usage, 0);

		m_dsetState[set].bindTextureAndSampler(realBinding, &view, sampler.get(), lay);

		m_microCmdb->pushObjectRef(texView);
		m_microCmdb->pushObjectRef(sampler);
	}

	void bindImage(U32 set, U32 binding, TextureViewPtr& img)
	{
		commandCommon();
		const U realBinding =
			binding + MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS;
		m_dsetState[set].bindImage(realBinding, img.get());
		m_microCmdb->pushObjectRef(img);
	}

	void beginRenderPass(FramebufferPtr fb,
		const Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS>& colorAttachmentUsages,
		TextureUsageBit depthStencilAttachmentUsage,
		U32 minx,
		U32 miny,
		U32 width,
		U32 height);

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

	void generateMipmaps2d(TextureViewPtr texView);

	void clearTextureView(TextureViewPtr texView, const ClearValue& clearValue);

	void pushSecondLevelCommandBuffer(CommandBufferPtr cmdb);

	void endRecording();

	void setTextureBarrier(TexturePtr tex,
		TextureUsageBit prevUsage,
		TextureUsageBit nextUsage,
		const TextureSubresourceInfo& subresource);

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
		const U realBinding = MAX_TEXTURE_BINDINGS + binding;
		m_dsetState[set].bindUniformBuffer(realBinding, buff.get(), offset, range);
		m_microCmdb->pushObjectRef(buff);
	}

	void bindStorageBuffer(U32 set, U32 binding, BufferPtr& buff, PtrSize offset, PtrSize range)
	{
		commandCommon();
		const U realBinding = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + binding;
		m_dsetState[set].bindStorageBuffer(realBinding, buff.get(), offset, range);
		m_microCmdb->pushObjectRef(buff);
	}

	void copyBufferToTextureViewInternal(BufferPtr buff, PtrSize offset, PtrSize range, TextureViewPtr texView);

	void copyBufferToBuffer(BufferPtr& src, PtrSize srcOffset, BufferPtr& dst, PtrSize dstOffset, PtrSize range);

	void setPushConstants(const void* data, U32 dataSize);

private:
	StackAllocator<U8> m_alloc;

	MicroCommandBufferPtr m_microCmdb;
	VkCommandBuffer m_handle = VK_NULL_HANDLE;
	CommandBufferFlag m_flags = CommandBufferFlag::NONE;
	Bool8 m_renderedToDefaultFb = false;
	Bool8 m_finalized = false;
	Bool8 m_empty = true;
	Bool8 m_beganRecording = false;
	ThreadId m_tid = ~ThreadId(0);
#if ANKI_EXTRA_CHECKS
	U32 m_commandCount = 0;
	U32 m_setPushConstantsSize = 0;
#endif

	U m_rpCommandCount = 0; ///< Number of drawcalls or pushed cmdbs in rp.
	FramebufferPtr m_activeFb;
	Array<U32, 4> m_renderArea = {{0, 0, MAX_U32, MAX_U32}};
	Array<U32, 2> m_fbSize = {{0, 0}};
	Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS> m_colorAttachmentUsages = {};
	TextureUsageBit m_depthStencilAttachmentUsage = TextureUsageBit::NONE;

	ShaderProgramImpl* m_graphicsProg ANKI_DBG_NULLIFY; ///< Last bound graphics program

	PipelineStateTracker m_state;

	Array<DescriptorSetState, MAX_DESCRIPTOR_SETS> m_dsetState;

	ShaderProgramImpl* m_computeProg ANKI_DBG_NULLIFY;

	VkSubpassContents m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;

	CommandBufferCommandType m_lastCmdType = CommandBufferCommandType::ANY_OTHER_COMMAND;

	/// @name state_opts
	/// @{
	Array<U32, 4> m_viewport = {{0, 0, 0, 0}};
	Array<U32, 4> m_scissor = {{0, 0, MAX_U32, MAX_U32}};
	Bool8 m_viewportDirty = true;
	VkViewport m_lastViewport = {};
	Bool8 m_scissorDirty = true;
	VkRect2D m_lastScissor = {{-1, -1}, {MAX_U32, MAX_U32}};
	Array<U32, 2> m_stencilCompareMasks = {{0x5A5A5A5A, 0x5A5A5A5A}}; ///< Use a stupid number to initialize.
	Array<U32, 2> m_stencilWriteMasks = {{0x5A5A5A5A, 0x5A5A5A5A}};
	Array<U32, 2> m_stencilReferenceMasks = {{0x5A5A5A5A, 0x5A5A5A5A}};

	/// Rebind the above dynamic state. Needed after pushing secondary command buffers (they dirty the state).
	void rebindDynamicState();
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

	/// @name push_second_level_batch
	/// @{
	DynamicArray<VkCommandBuffer> m_secondLevelAtoms;
	U16 m_secondLevelAtomCount = 0;
	/// @}

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

	void setImageBarrier(VkPipelineStageFlags srcStage,
		VkAccessFlags srcAccess,
		VkImageLayout prevLayout,
		VkPipelineStageFlags dstStage,
		VkAccessFlags dstAccess,
		VkImageLayout newLayout,
		VkImage img,
		const VkImageSubresourceRange& range);

	void beginRecording();

	Bool flipViewport() const;

	static VkViewport computeViewport(U32* viewport, U32 fbWidth, U32 fbHeight, Bool flipvp)
	{
		const U32 minx = viewport[0];
		const U32 miny = viewport[1];
		const U32 width = min<U32>(fbWidth, viewport[2]);
		const U32 height = min<U32>(fbHeight, viewport[3]);
		ANKI_ASSERT(width > 0 && height > 0);
		ANKI_ASSERT(minx + width <= fbWidth);
		ANKI_ASSERT(miny + height <= fbHeight);

		VkViewport s = {};
		s.x = minx;
		s.y = (flipvp) ? (fbHeight - miny) : miny; // Move to the bottom;
		s.width = width;
		s.height = (flipvp) ? -F32(height) : height;
		s.minDepth = 0.0f;
		s.maxDepth = 1.0f;
		return s;
	}

	static VkRect2D computeScissor(U32* scissor, U32 fbWidth, U32 fbHeight, Bool flipvp)
	{
		const U32 minx = scissor[0];
		const U32 miny = scissor[1];
		const U32 width = min<U32>(fbWidth, scissor[2]);
		const U32 height = min<U32>(fbHeight, scissor[3]);
		ANKI_ASSERT(minx + width <= fbWidth);
		ANKI_ASSERT(miny + height <= fbHeight);

		VkRect2D out = {};
		out.extent.width = width;
		out.extent.height = height;
		out.offset.x = minx;
		out.offset.y = (flipvp) ? (fbHeight - (miny + height)) : miny;

		return out;
	}
};
/// @}

} // end namespace anki

#include <anki/gr/vulkan/CommandBufferImpl.inl.h>

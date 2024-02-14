// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Vulkan/CommandBufferFactory.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Shader.h>
#include <AnKi/Gr/Vulkan/PipelineQueryImpl.h>
#include <AnKi/Gr/Vulkan/BufferImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/Vulkan/Pipeline.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Util/List.h>

namespace anki {

#define ANKI_BATCH_COMMANDS 1

// Forward
class CommandBufferInitInfo;

/// @addtogroup vulkan
/// @{

/// Command buffer implementation.
class CommandBufferImpl final : public CommandBuffer
{
public:
	/// Default constructor
	CommandBufferImpl(CString name)
		: CommandBuffer(name)
	{
	}

	~CommandBufferImpl();

	Error init(const CommandBufferInitInfo& init);

	void setFence(MicroFence* fence)
	{
		m_microCmdb->setFence(fence);
	}

	const MicroCommandBufferPtr& getMicroCommandBuffer()
	{
		return m_microCmdb;
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
		return !!(m_flags & CommandBufferFlag::kSecondLevel);
	}

	ANKI_FORCE_INLINE void bindVertexBufferInternal(U32 binding, Buffer* buff, PtrSize offset, PtrSize stride, VertexStepRate stepRate)
	{
		commandCommon();
		m_state.bindVertexBuffer(binding, stride, stepRate);
		const VkBuffer vkbuff = static_cast<const BufferImpl&>(*buff).getHandle();
		vkCmdBindVertexBuffers(m_handle, binding, 1, &vkbuff, &offset);
	}

	ANKI_FORCE_INLINE void setVertexAttributeInternal(U32 location, U32 buffBinding, const Format fmt, PtrSize relativeOffset)
	{
		commandCommon();
		m_state.setVertexAttribute(location, buffBinding, fmt, relativeOffset);
	}

	ANKI_FORCE_INLINE void bindIndexBufferInternal(Buffer* buff, PtrSize offset, IndexType type)
	{
		commandCommon();
		vkCmdBindIndexBuffer(m_handle, static_cast<const BufferImpl&>(*buff).getHandle(), offset, convertIndexType(type));
	}

	ANKI_FORCE_INLINE void setPrimitiveRestartInternal(Bool enable)
	{
		commandCommon();
		m_state.setPrimitiveRestart(enable);
	}

	ANKI_FORCE_INLINE void setFillModeInternal(FillMode mode)
	{
		commandCommon();
		m_state.setFillMode(mode);
	}

	ANKI_FORCE_INLINE void setCullModeInternal(FaceSelectionBit mode)
	{
		commandCommon();
		m_state.setCullMode(mode);
	}

	ANKI_FORCE_INLINE void setViewportInternal(U32 minx, U32 miny, U32 width, U32 height)
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

	ANKI_FORCE_INLINE void setScissorInternal(U32 minx, U32 miny, U32 width, U32 height)
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

	ANKI_FORCE_INLINE void setPolygonOffsetInternal(F32 factor, F32 units)
	{
		commandCommon();
		m_state.setPolygonOffset(factor, units);
		vkCmdSetDepthBias(m_handle, factor, 0.0f, units);
	}

	ANKI_FORCE_INLINE void setStencilOperationsInternal(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
														StencilOperation stencilPassDepthPass)
	{
		commandCommon();
		m_state.setStencilOperations(face, stencilFail, stencilPassDepthFail, stencilPassDepthPass);
	}

	ANKI_FORCE_INLINE void setStencilCompareOperationInternal(FaceSelectionBit face, CompareOperation comp)
	{
		commandCommon();
		m_state.setStencilCompareOperation(face, comp);
	}

	void setStencilCompareMaskInternal(FaceSelectionBit face, U32 mask);

	void setStencilWriteMaskInternal(FaceSelectionBit face, U32 mask);

	void setStencilReferenceInternal(FaceSelectionBit face, U32 ref);

	ANKI_FORCE_INLINE void setDepthWriteInternal(Bool enable)
	{
		commandCommon();
		m_state.setDepthWrite(enable);
	}

	ANKI_FORCE_INLINE void setDepthCompareOperationInternal(CompareOperation op)
	{
		commandCommon();
		m_state.setDepthCompareOperation(op);
	}

	ANKI_FORCE_INLINE void setAlphaToCoverageInternal(Bool enable)
	{
		commandCommon();
		m_state.setAlphaToCoverage(enable);
	}

	ANKI_FORCE_INLINE void setColorChannelWriteMaskInternal(U32 attachment, ColorBit mask)
	{
		commandCommon();
		m_state.setColorChannelWriteMask(attachment, mask);
	}

	ANKI_FORCE_INLINE void setBlendFactorsInternal(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
	{
		commandCommon();
		m_state.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA);
	}

	ANKI_FORCE_INLINE void setBlendOperationInternal(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
	{
		commandCommon();
		m_state.setBlendOperation(attachment, funcRgb, funcA);
	}

	ANKI_FORCE_INLINE void bindTextureInternal(U32 set, U32 binding, TextureView* texView, U32 arrayIdx)
	{
		commandCommon();
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
		const TextureImpl& tex = view.getTextureImpl();
		ANKI_ASSERT(tex.isSubresourceGoodForSampling(view.getSubresource()));
		const VkImageLayout lay = tex.computeLayout(TextureUsageBit::kAllSampled & tex.getTextureUsage(), 0);

		m_dsetState[set].bindTexture(binding, arrayIdx, texView, lay);
	}

	ANKI_FORCE_INLINE void bindSamplerInternal(U32 set, U32 binding, Sampler* sampler, U32 arrayIdx)
	{
		commandCommon();
		m_dsetState[set].bindSampler(binding, arrayIdx, sampler);
		m_microCmdb->pushObjectRef(sampler);
	}

	ANKI_FORCE_INLINE void bindUavTextureInternal(U32 set, U32 binding, TextureView* img, U32 arrayIdx)
	{
		commandCommon();
		m_dsetState[set].bindUavTexture(binding, arrayIdx, img);

		const Bool isPresentable = !!(static_cast<const TextureViewImpl&>(*img).getTextureImpl().getTextureUsage() & TextureUsageBit::kPresent);
		if(isPresentable)
		{
			m_renderedToDefaultFb = true;
		}
	}

	ANKI_FORCE_INLINE void bindAccelerationStructureInternal(U32 set, U32 binding, AccelerationStructure* as, U32 arrayIdx)
	{
		commandCommon();
		m_dsetState[set].bindAccelerationStructure(binding, arrayIdx, as);
		m_microCmdb->pushObjectRef(as);
	}

	ANKI_FORCE_INLINE void bindAllBindlessInternal(U32 set)
	{
		commandCommon();
		m_dsetState[set].bindBindlessDescriptorSet();
	}

	void beginRenderPassInternal(Framebuffer* fb, const Array<TextureUsageBit, kMaxColorRenderTargets>& colorAttachmentUsages,
								 TextureUsageBit depthStencilAttachmentUsage, U32 minx, U32 miny, U32 width, U32 height);

	void endRenderPassInternal();

	ANKI_FORCE_INLINE void setVrsRateInternal(VrsRate rate)
	{
		ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_vrs);
		ANKI_ASSERT(rate < VrsRate::kCount);
		commandCommon();

		if(m_vrsRate != rate)
		{
			m_vrsRate = rate;
			m_vrsRateDirty = true;
		}
	}

	ANKI_FORCE_INLINE void drawInternal(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
	{
		m_state.setPrimitiveTopology(topology);
		drawcallCommon();
		vkCmdDraw(m_handle, count, instanceCount, first, baseInstance);
	}

	ANKI_FORCE_INLINE void drawIndexedInternal(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex,
											   U32 baseInstance)
	{
		m_state.setPrimitiveTopology(topology);
		drawcallCommon();
		vkCmdDrawIndexed(m_handle, count, instanceCount, firstIndex, baseVertex, baseInstance);
	}

	ANKI_FORCE_INLINE void drawIndirectInternal(PrimitiveTopology topology, U32 drawCount, PtrSize offset, Buffer* buff)
	{
		m_state.setPrimitiveTopology(topology);
		drawcallCommon();
		const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
		ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((offset % 4) == 0);
		ANKI_ASSERT((offset + sizeof(DrawIndirectArgs) * drawCount) <= impl.getSize());

		vkCmdDrawIndirect(m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawIndirectArgs));
	}

	ANKI_FORCE_INLINE void drawIndexedIndirectInternal(PrimitiveTopology topology, U32 drawCount, PtrSize offset, Buffer* buff)
	{
		m_state.setPrimitiveTopology(topology);
		drawcallCommon();
		const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
		ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((offset % 4) == 0);
		ANKI_ASSERT((offset + sizeof(DrawIndexedIndirectArgs) * drawCount) <= impl.getSize());

		vkCmdDrawIndexedIndirect(m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawIndexedIndirectArgs));
	}

	ANKI_FORCE_INLINE void drawMeshTasksInternal(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
	{
		ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));
		drawcallCommon();
		vkCmdDrawMeshTasksEXT(m_handle, groupCountX, groupCountY, groupCountZ);
	}

	ANKI_FORCE_INLINE void drawMeshTasksIndirectInternal(Buffer* argBuffer, PtrSize argBufferOffset)
	{
		ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));
		ANKI_ASSERT((argBufferOffset % 4) == 0);
		const BufferImpl& impl = static_cast<const BufferImpl&>(*argBuffer);
		ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((argBufferOffset + sizeof(DispatchIndirectArgs)) <= impl.getSize());

		m_state.setPrimitiveTopology(PrimitiveTopology::kTriangles); // Not sure if that's needed
		drawcallCommon();
		vkCmdDrawMeshTasksIndirectEXT(m_handle, impl.getHandle(), argBufferOffset, 1, sizeof(DispatchIndirectArgs));
	}

	ANKI_FORCE_INLINE void drawIndexedIndirectCountInternal(PrimitiveTopology topology, Buffer* argBuffer, PtrSize argBufferOffset,
															U32 argBufferStride, Buffer* countBuffer, PtrSize countBufferOffset, U32 maxDrawCount)
	{
		m_state.setPrimitiveTopology(topology);
		drawcallCommon();
		const BufferImpl& argBufferImpl = static_cast<const BufferImpl&>(*argBuffer);
		ANKI_ASSERT(argBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((argBufferOffset % 4) == 0);
		ANKI_ASSERT(argBufferStride >= sizeof(DrawIndexedIndirectArgs));
		ANKI_ASSERT(argBufferOffset + maxDrawCount * argBufferStride <= argBuffer->getSize());

		const BufferImpl& countBufferImpl = static_cast<const BufferImpl&>(*countBuffer);
		ANKI_ASSERT(countBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((countBufferOffset % 4) == 0);
		ANKI_ASSERT(countBufferOffset + sizeof(U32) <= countBuffer->getSize());

		ANKI_ASSERT(maxDrawCount > 0 && maxDrawCount <= getGrManagerImpl().getDeviceCapabilities().m_maxDrawIndirectCount);

		vkCmdDrawIndexedIndirectCountKHR(m_handle, argBufferImpl.getHandle(), argBufferOffset, countBufferImpl.getHandle(), countBufferOffset,
										 maxDrawCount, argBufferStride);
	}

	ANKI_FORCE_INLINE void drawIndirectCountInternal(PrimitiveTopology topology, Buffer* argBuffer, PtrSize argBufferOffset, U32 argBufferStride,
													 Buffer* countBuffer, PtrSize countBufferOffset, U32 maxDrawCount)
	{
		m_state.setPrimitiveTopology(topology);
		drawcallCommon();
		const BufferImpl& argBufferImpl = static_cast<const BufferImpl&>(*argBuffer);
		ANKI_ASSERT(argBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((argBufferOffset % 4) == 0);
		ANKI_ASSERT(argBufferStride >= sizeof(DrawIndirectArgs));
		ANKI_ASSERT(argBufferOffset + maxDrawCount * argBufferStride <= argBuffer->getSize());

		const BufferImpl& countBufferImpl = static_cast<const BufferImpl&>(*countBuffer);
		ANKI_ASSERT(countBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
		ANKI_ASSERT((countBufferOffset % 4) == 0);
		ANKI_ASSERT(countBufferOffset + maxDrawCount * sizeof(U32) <= countBuffer->getSize());

		ANKI_ASSERT(maxDrawCount > 0 && maxDrawCount <= getGrManagerImpl().getDeviceCapabilities().m_maxDrawIndirectCount);

		vkCmdDrawIndirectCountKHR(m_handle, argBufferImpl.getHandle(), argBufferOffset, countBufferImpl.getHandle(), countBufferOffset, maxDrawCount,
								  argBufferStride);
	}

	void dispatchComputeInternal(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

	void dispatchComputeIndirectInternal(Buffer* argBuffer, PtrSize argBufferOffset);

	void traceRaysInternal(Buffer* sbtBuffer, PtrSize sbtBufferOffset, U32 sbtRecordSize, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width,
						   U32 height, U32 depth);

	void resetOcclusionQueriesInternal(ConstWeakArray<OcclusionQuery*> queries);

	void beginOcclusionQueryInternal(OcclusionQuery* query);

	void endOcclusionQueryInternal(OcclusionQuery* query);

	ANKI_FORCE_INLINE void beginPipelineQueryInternal(PipelineQuery* query)
	{
		commandCommon();
		const VkQueryPool handle = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryPool();
		const U32 idx = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryIndex();
		ANKI_ASSERT(handle);
		vkCmdBeginQuery(m_handle, handle, idx, 0);
		m_microCmdb->pushObjectRef(query);
	}

	ANKI_FORCE_INLINE void endPipelineQueryInternal(PipelineQuery* query)
	{
		commandCommon();
		const VkQueryPool handle = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryPool();
		const U32 idx = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryIndex();
		ANKI_ASSERT(handle);
		vkCmdEndQuery(m_handle, handle, idx);
		m_microCmdb->pushObjectRef(query);
	}

	void resetTimestampQueriesInternal(ConstWeakArray<TimestampQuery*> queries);

	void writeTimestampInternal(TimestampQuery* query);

	void generateMipmaps2dInternal(TextureView* texView);

	void clearTextureViewInternal(TextureView* texView, const ClearValue& clearValue);

	void pushSecondLevelCommandBuffersInternal(ConstWeakArray<CommandBuffer*> cmdbs);

	// To enable using Anki's commandbuffers for external workloads
	void beginRecordingExt()
	{
		commandCommon();
	}

	void endRecording();

	void setPipelineBarrierInternal(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
									ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures);

	void fillBufferInternal(Buffer* buff, PtrSize offset, PtrSize size, U32 value);

	void writeOcclusionQueriesResultToBufferInternal(ConstWeakArray<OcclusionQuery*> queries, PtrSize offset, Buffer* buff);

	void bindShaderProgramInternal(ShaderProgram* prog);

	ANKI_FORCE_INLINE void bindConstantBufferInternal(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, U32 arrayIdx)
	{
		commandCommon();
		m_dsetState[set].bindConstantBuffer(binding, arrayIdx, buff, offset, range);
	}

	ANKI_FORCE_INLINE void bindUavBufferInternal(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, U32 arrayIdx)
	{
		commandCommon();
		m_dsetState[set].bindUavBuffer(binding, arrayIdx, buff, offset, range);
	}

	ANKI_FORCE_INLINE void bindReadOnlyTextureBufferInternal(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, Format fmt,
															 U32 arrayIdx)
	{
		commandCommon();
		m_dsetState[set].bindReadOnlyTextureBuffer(binding, arrayIdx, buff, offset, range, fmt);
	}

	void copyBufferToTextureViewInternal(Buffer* buff, PtrSize offset, PtrSize range, TextureView* texView);

	void copyBufferToBufferInternal(Buffer* src, Buffer* dst, ConstWeakArray<CopyBufferToBufferInfo> copies);

	void buildAccelerationStructureInternal(AccelerationStructure* as, Buffer* scratchBuffer, PtrSize scratchBufferOffset);

	void upscaleInternal(GrUpscaler* upscaler, TextureView* inColor, TextureView* outUpscaledColor, TextureView* motionVectors, TextureView* depth,
						 TextureView* exposure, const Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& motionVectorsScale);

	void setPushConstantsInternal(const void* data, U32 dataSize);

	ANKI_FORCE_INLINE void setRasterizationOrderInternal(RasterizationOrder order)
	{
		commandCommon();

		if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kAMD_rasterization_order))
		{
			m_state.setRasterizationOrder(order);
		}
	}

	ANKI_FORCE_INLINE void setLineWidthInternal(F32 width)
	{
		commandCommon();
		vkCmdSetLineWidth(m_handle, width);

#if ANKI_ASSERTIONS_ENABLED
		m_lineWidthSet = true;
#endif
	}

	ANKI_FORCE_INLINE void pushDebugMarkerInternal(CString name, Vec3 color)
	{
		if(m_debugMarkers) [[unlikely]]
		{
			commandCommon();

			VkDebugUtilsLabelEXT label = {};
			label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			label.pLabelName = name.cstr();
			label.color[0] = color[0];
			label.color[1] = color[1];
			label.color[2] = color[2];
			label.color[3] = 1.0f;
			vkCmdBeginDebugUtilsLabelEXT(m_handle, &label);
		}

#if ANKI_EXTRA_CHECKS
		++m_debugMarkersPushed;
#endif
	}

	ANKI_FORCE_INLINE void popDebugMarkerInternal()
	{
		if(m_debugMarkers) [[unlikely]]
		{
			commandCommon();
			vkCmdEndDebugUtilsLabelEXT(m_handle);
		}

#if ANKI_EXTRA_CHECKS
		ANKI_ASSERT(m_debugMarkersPushed > 0);
		--m_debugMarkersPushed;
#endif
	}

private:
	StackMemoryPool* m_pool = nullptr;

	MicroCommandBufferPtr m_microCmdb;
	VkCommandBuffer m_handle = VK_NULL_HANDLE;
	ThreadId m_tid = ~ThreadId(0);
	CommandBufferFlag m_flags = CommandBufferFlag::kNone;
	Bool m_renderedToDefaultFb : 1 = false;
	Bool m_finalized : 1 = false;
	Bool m_empty : 1 = true;
	Bool m_beganRecording : 1 = false;
	Bool m_debugMarkers : 1 = false;
#if ANKI_EXTRA_CHECKS
	U32 m_commandCount = 0;
	U32 m_setPushConstantsSize = 0;
	U32 m_debugMarkersPushed = 0;
#endif

	Framebuffer* m_activeFb = nullptr;
	Array<U32, 4> m_renderArea = {0, 0, kMaxU32, kMaxU32};
	Array<U32, 2> m_fbSize = {0, 0};
	U32 m_rpCommandCount = 0; ///< Number of drawcalls or pushed cmdbs in rp.
	Array<TextureUsageBit, kMaxColorRenderTargets> m_colorAttachmentUsages = {};
	TextureUsageBit m_depthStencilAttachmentUsage = TextureUsageBit::kNone;

	PipelineStateTracker m_state;

	Array<DSStateTracker, kMaxDescriptorSets> m_dsetState;

	ShaderProgramImpl* m_graphicsProg ANKI_DEBUG_CODE(= nullptr); ///< Last bound graphics program
	ShaderProgramImpl* m_computeProg ANKI_DEBUG_CODE(= nullptr);
	ShaderProgramImpl* m_rtProg ANKI_DEBUG_CODE(= nullptr);

	VkSubpassContents m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;

	/// @name state_opts
	/// @{
	Array<U32, 4> m_viewport = {0, 0, 0, 0};
	Array<U32, 4> m_scissor = {0, 0, kMaxU32, kMaxU32};
	VkViewport m_lastViewport = {};
	Bool m_viewportDirty = true;
	Bool m_scissorDirty = true;
	VkRect2D m_lastScissor = {{-1, -1}, {kMaxU32, kMaxU32}};
	Array<U32, 2> m_stencilCompareMasks = {0x5A5A5A5A, 0x5A5A5A5A}; ///< Use a stupid number to initialize.
	Array<U32, 2> m_stencilWriteMasks = {0x5A5A5A5A, 0x5A5A5A5A};
	Array<U32, 2> m_stencilReferenceMasks = {0x5A5A5A5A, 0x5A5A5A5A};
#if ANKI_ASSERTIONS_ENABLED
	Bool m_lineWidthSet = false;
#endif
	Bool m_vrsRateDirty = true;
	VrsRate m_vrsRate = VrsRate::k1x1;

	/// Rebind the above dynamic state. Needed after pushing secondary command buffers (they dirty the state).
	void rebindDynamicState();
	/// @}

	/// Some common operations per command.
	ANKI_FORCE_INLINE void commandCommon()
	{
		ANKI_ASSERT(!m_finalized);
#if ANKI_EXTRA_CHECKS
		++m_commandCount;
#endif
		m_empty = false;

		if(!m_beganRecording) [[unlikely]]
		{
			beginRecording();
			m_beganRecording = true;
		}

		ANKI_ASSERT(Thread::getCurrentThreadId() == m_tid && "Commands must be recorder and flushed by the thread this command buffer was created");
		ANKI_ASSERT(m_handle);
	}

	void drawcallCommon();

	void dispatchCommon();

	Bool insideRenderPass() const
	{
		return m_activeFb != nullptr;
	}

	void beginRenderPassInternal();

	Bool secondLevel() const
	{
		return !!(m_flags & CommandBufferFlag::kSecondLevel);
	}

	void setImageBarrier(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkImageLayout prevLayout, VkPipelineStageFlags dstStage,
						 VkAccessFlags dstAccess, VkImageLayout newLayout, VkImage img, const VkImageSubresourceRange& range);

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
		s.x = F32(minx);
		s.y = (flipvp) ? F32(fbHeight - miny) : F32(miny); // Move to the bottom;
		s.width = F32(width);
		s.height = (flipvp) ? -F32(height) : F32(height);
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

	const ShaderProgramImpl& getBoundProgram()
	{
		if(m_graphicsProg)
		{
			ANKI_ASSERT(m_computeProg == nullptr && m_rtProg == nullptr);
			return *m_graphicsProg;
		}
		else if(m_computeProg)
		{
			ANKI_ASSERT(m_graphicsProg == nullptr && m_rtProg == nullptr);
			return *m_computeProg;
		}
		else
		{
			ANKI_ASSERT(m_graphicsProg == nullptr && m_computeProg == nullptr && m_rtProg != nullptr);
			return *m_rtProg;
		}
	}
};
/// @}

} // end namespace anki

#include <AnKi/Gr/Vulkan/CommandBufferImpl.inl.h>

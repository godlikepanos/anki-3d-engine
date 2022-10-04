// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/CommandBufferImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/OcclusionQuery.h>
#include <AnKi/Gr/Vulkan/OcclusionQueryImpl.h>
#include <AnKi/Gr/TimestampQuery.h>
#include <AnKi/Gr/Vulkan/TimestampQueryImpl.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

inline void CommandBufferImpl::setStencilCompareMaskInternal(FaceSelectionBit face, U32 mask)
{
	commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::kFront) && m_stencilCompareMasks[0] != mask)
	{
		m_stencilCompareMasks[0] = mask;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::kBack) && m_stencilCompareMasks[1] != mask)
	{
		m_stencilCompareMasks[1] = mask;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		vkCmdSetStencilCompareMask(m_handle, flags, mask);
	}
}

inline void CommandBufferImpl::setStencilWriteMaskInternal(FaceSelectionBit face, U32 mask)
{
	commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::kFront) && m_stencilWriteMasks[0] != mask)
	{
		m_stencilWriteMasks[0] = mask;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::kBack) && m_stencilWriteMasks[1] != mask)
	{
		m_stencilWriteMasks[1] = mask;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		vkCmdSetStencilWriteMask(m_handle, flags, mask);
	}
}

inline void CommandBufferImpl::setStencilReferenceInternal(FaceSelectionBit face, U32 ref)
{
	commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::kFront) && m_stencilReferenceMasks[0] != ref)
	{
		m_stencilReferenceMasks[0] = ref;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::kBack) && m_stencilReferenceMasks[1] != ref)
	{
		m_stencilWriteMasks[1] = ref;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		vkCmdSetStencilReference(m_handle, flags, ref);
	}
}

inline void CommandBufferImpl::setImageBarrier(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess,
											   VkImageLayout prevLayout, VkPipelineStageFlags dstStage,
											   VkAccessFlags dstAccess, VkImageLayout newLayout, VkImage img,
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

	vkCmdPipelineBarrier(m_handle, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &inf);
	ANKI_TRACE_INC_COUNTER(VK_PIPELINE_BARRIERS, 1);
}

inline void CommandBufferImpl::drawArraysInternal(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first,
												  U32 baseInstance)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	vkCmdDraw(m_handle, count, instanceCount, first, baseInstance);
}

inline void CommandBufferImpl::drawElementsInternal(PrimitiveTopology topology, U32 count, U32 instanceCount,
													U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	vkCmdDrawIndexed(m_handle, count, instanceCount, firstIndex, baseVertex, baseInstance);
}

inline void CommandBufferImpl::drawArraysIndirectInternal(PrimitiveTopology topology, U32 drawCount, PtrSize offset,
														  const BufferPtr& buff)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(DrawArraysIndirectInfo) * drawCount) <= impl.getSize());

	vkCmdDrawIndirect(m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawArraysIndirectInfo));
}

inline void CommandBufferImpl::drawElementsIndirectInternal(PrimitiveTopology topology, U32 drawCount, PtrSize offset,
															const BufferPtr& buff)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(DrawElementsIndirectInfo) * drawCount) <= impl.getSize());

	vkCmdDrawIndexedIndirect(m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawElementsIndirectInfo));
}

inline void CommandBufferImpl::dispatchComputeInternal(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(m_computeProg);
	ANKI_ASSERT(m_computeProg->getReflectionInfo().m_pushConstantsSize == m_setPushConstantsSize
				&& "Forgot to set pushConstants");

	commandCommon();

	getGrManagerImpl().beginMarker(m_handle, m_computeProg->getName(), Vec3(1.0f, 1.0f, 0.0f));

	// Bind descriptors
	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(m_computeProg->getReflectionInfo().m_descriptorSetMask.get(i))
		{
			DescriptorSet dset;
			Bool dirty;
			Array<PtrSize, kMaxBindingsPerDescriptorSet> dynamicOffsetsPtrSize;
			U32 dynamicOffsetCount;
			if(getGrManagerImpl().getDescriptorSetFactory().newDescriptorSet(*m_pool, m_dsetState[i], dset, dirty,
																			 dynamicOffsetsPtrSize, dynamicOffsetCount))
			{
				ANKI_VK_LOGF("Cannot recover");
			}

			if(dirty)
			{
				// Vulkan should have had the dynamic offsets as VkDeviceSize and not U32. Workaround that.
				Array<U32, kMaxBindingsPerDescriptorSet> dynamicOffsets;
				for(U32 i = 0; i < dynamicOffsetCount; ++i)
				{
					dynamicOffsets[i] = U32(dynamicOffsetsPtrSize[i]);
				}

				VkDescriptorSet dsHandle = dset.getHandle();

				vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE,
										m_computeProg->getPipelineLayout().getHandle(), i, 1, &dsHandle,
										dynamicOffsetCount, &dynamicOffsets[0]);
			}
		}
	}

	vkCmdDispatch(m_handle, groupCountX, groupCountY, groupCountZ);

	getGrManagerImpl().endMarker(m_handle);
}

inline void CommandBufferImpl::traceRaysInternal(const BufferPtr& sbtBuffer, PtrSize sbtBufferOffset,
												 U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount,
												 U32 width, U32 height, U32 depth)
{
	const PtrSize sbtRecordSize = sbtRecordSize32;
	ANKI_ASSERT(hitGroupSbtRecordCount > 0);
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	ANKI_ASSERT(m_rtProg);
	const ShaderProgramImpl& sprog = static_cast<const ShaderProgramImpl&>(*m_rtProg);
	ANKI_ASSERT(sprog.getReflectionInfo().m_pushConstantsSize == m_setPushConstantsSize
				&& "Forgot to set pushConstants");

	ANKI_ASSERT(rayTypeCount == sprog.getMissShaderCount() && "All the miss shaders should be in use");
	ANKI_ASSERT((hitGroupSbtRecordCount % rayTypeCount) == 0);
	const PtrSize sbtRecordCount = 1 + rayTypeCount + hitGroupSbtRecordCount;
	[[maybe_unused]] const PtrSize sbtBufferSize = sbtRecordCount * sbtRecordSize;
	ANKI_ASSERT(sbtBufferSize + sbtBufferOffset <= sbtBuffer->getSize());
	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_sbtRecordAlignment, sbtBufferOffset));

	commandCommon();

	getGrManagerImpl().beginMarker(m_handle, m_rtProg->getName(), Vec3(0.0f, 0.0f, 1.0f));

	// Bind descriptors
	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(sprog.getReflectionInfo().m_descriptorSetMask.get(i))
		{
			DescriptorSet dset;
			Bool dirty;
			Array<PtrSize, kMaxBindingsPerDescriptorSet> dynamicOffsetsPtrSize;
			U32 dynamicOffsetCount;
			if(getGrManagerImpl().getDescriptorSetFactory().newDescriptorSet(*m_pool, m_dsetState[i], dset, dirty,
																			 dynamicOffsetsPtrSize, dynamicOffsetCount))
			{
				ANKI_VK_LOGF("Cannot recover");
			}

			if(dirty)
			{
				// Vulkan should have had the dynamic offsets as VkDeviceSize and not U32. Workaround that.
				Array<U32, kMaxBindingsPerDescriptorSet> dynamicOffsets;
				for(U32 i = 0; i < dynamicOffsetCount; ++i)
				{
					dynamicOffsets[i] = U32(dynamicOffsetsPtrSize[i]);
				}

				VkDescriptorSet dsHandle = dset.getHandle();

				vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
										sprog.getPipelineLayout().getHandle(), i, 1, &dsHandle, dynamicOffsetCount,
										&dynamicOffsets[0]);
			}
		}
	}

	Array<VkStridedDeviceAddressRegionKHR, 4> regions;
	const U64 stbBufferAddress = sbtBuffer->getGpuAddress() + sbtBufferOffset;
	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_sbtRecordAlignment, stbBufferAddress));

	// Rgen
	regions[0].deviceAddress = stbBufferAddress;
	regions[0].stride = sbtRecordSize;
	regions[0].size = sbtRecordSize;

	// Miss
	regions[1].deviceAddress = regions[0].deviceAddress + regions[0].size;
	regions[1].stride = sbtRecordSize;
	regions[1].size = sbtRecordSize * rayTypeCount;

	// Hit
	regions[2].deviceAddress = regions[1].deviceAddress + regions[1].size;
	regions[2].stride = sbtRecordSize * rayTypeCount;
	regions[2].size = sbtRecordSize * hitGroupSbtRecordCount;

	// Callable, nothing for now
	regions[3] = VkStridedDeviceAddressRegionKHR();

	vkCmdTraceRaysKHR(m_handle, &regions[0], &regions[1], &regions[2], &regions[3], width, height, depth);

	getGrManagerImpl().endMarker(m_handle);
}

inline void CommandBufferImpl::resetOcclusionQueriesInternal(ConstWeakArray<OcclusionQuery*> queries)
{
	ANKI_ASSERT(queries.getSize() > 0);

	commandCommon();

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		OcclusionQuery* query = queries[i];
		ANKI_ASSERT(query);
		const VkQueryPool poolHandle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
		const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();

		vkCmdResetQueryPool(m_handle, poolHandle, idx, 1);
		m_microCmdb->pushObjectRef(query);
	}
}

inline void CommandBufferImpl::beginOcclusionQueryInternal(const OcclusionQueryPtr& query)
{
	commandCommon();

	const VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);

	vkCmdBeginQuery(m_handle, handle, idx, 0);

	m_microCmdb->pushObjectRef(query);
}

inline void CommandBufferImpl::endOcclusionQueryInternal(const OcclusionQueryPtr& query)
{
	commandCommon();

	const VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);

	vkCmdEndQuery(m_handle, handle, idx);

	m_microCmdb->pushObjectRef(query);
}

inline void CommandBufferImpl::resetTimestampQueriesInternal(ConstWeakArray<TimestampQuery*> queries)
{
	ANKI_ASSERT(queries.getSize() > 0);

	commandCommon();

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		TimestampQuery* query = queries[i];
		ANKI_ASSERT(query);
		const VkQueryPool poolHandle = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryPool();
		const U32 idx = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryIndex();

		vkCmdResetQueryPool(m_handle, poolHandle, idx, 1);
		m_microCmdb->pushObjectRef(query);
	}
}

inline void CommandBufferImpl::writeTimestampInternal(const TimestampQueryPtr& query)
{
	commandCommon();

	const VkQueryPool handle = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryIndex();

	vkCmdWriteTimestamp(m_handle, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, handle, idx);

	m_microCmdb->pushObjectRef(query);
}

inline void CommandBufferImpl::clearTextureViewInternal(const TextureViewPtr& texView, const ClearValue& clearValue)
{
	commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();

	VkClearColorValue vclear;
	static_assert(sizeof(vclear) == sizeof(clearValue), "See file");
	memcpy(&vclear, &clearValue, sizeof(clearValue));

	if(!view.getSubresource().m_depthStencilAspect)
	{
		VkImageSubresourceRange vkRange = view.getVkImageSubresourceRange();
		vkCmdClearColorImage(m_handle, tex.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vclear, 1, &vkRange);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	m_microCmdb->pushObjectRef(texView);
}

inline void CommandBufferImpl::pushSecondLevelCommandBuffersInternal(ConstWeakArray<CommandBuffer*> cmdbs)
{
	ANKI_ASSERT(cmdbs.getSize() > 0);
	commandCommon();
	ANKI_ASSERT(insideRenderPass());
	ANKI_ASSERT(m_subpassContents == VK_SUBPASS_CONTENTS_MAX_ENUM
				|| m_subpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	m_subpassContents = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

	if(ANKI_UNLIKELY(m_rpCommandCount == 0))
	{
		beginRenderPassInternal();
	}

	DynamicArrayRaii<VkCommandBuffer> handles(m_pool, cmdbs.getSize());
	for(U32 i = 0; i < cmdbs.getSize(); ++i)
	{
		ANKI_ASSERT(static_cast<const CommandBufferImpl&>(*cmdbs[i]).m_finalized);
		handles[i] = static_cast<const CommandBufferImpl&>(*cmdbs[i]).m_handle;
		m_microCmdb->pushObjectRef(cmdbs[i]);
	}

	vkCmdExecuteCommands(m_handle, handles.getSize(), handles.getBegin());

	++m_rpCommandCount;
}

inline void CommandBufferImpl::drawcallCommon()
{
	// Preconditions
	commandCommon();
	ANKI_ASSERT(m_graphicsProg);
	ANKI_ASSERT(insideRenderPass() || secondLevel());
	ANKI_ASSERT(m_subpassContents == VK_SUBPASS_CONTENTS_MAX_ENUM || m_subpassContents == VK_SUBPASS_CONTENTS_INLINE);
	ANKI_ASSERT(m_graphicsProg->getReflectionInfo().m_pushConstantsSize == m_setPushConstantsSize
				&& "Forgot to set pushConstants");

	m_subpassContents = VK_SUBPASS_CONTENTS_INLINE;

	if(ANKI_UNLIKELY(m_rpCommandCount == 0) && !secondLevel())
	{
		beginRenderPassInternal();
	}

	++m_rpCommandCount;

	// Get or create ppline
	Pipeline ppline;
	Bool stateDirty;
	m_graphicsProg->getPipelineFactory().getOrCreatePipeline(m_state, ppline, stateDirty);

	if(stateDirty)
	{
		vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ppline.getHandle());
	}

	// Bind dsets
	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(m_graphicsProg->getReflectionInfo().m_descriptorSetMask.get(i))
		{
			DescriptorSet dset;
			Bool dirty;
			Array<PtrSize, kMaxBindingsPerDescriptorSet> dynamicOffsetsPtrSize;
			U32 dynamicOffsetCount;
			if(getGrManagerImpl().getDescriptorSetFactory().newDescriptorSet(*m_pool, m_dsetState[i], dset, dirty,
																			 dynamicOffsetsPtrSize, dynamicOffsetCount))
			{
				ANKI_VK_LOGF("Cannot recover");
			}

			if(dirty)
			{
				// Vulkan should have had the dynamic offsets as VkDeviceSize and not U32. Workaround that.
				Array<U32, kMaxBindingsPerDescriptorSet> dynamicOffsets;
				for(U32 i = 0; i < dynamicOffsetCount; ++i)
				{
					dynamicOffsets[i] = U32(dynamicOffsetsPtrSize[i]);
				}

				VkDescriptorSet dsHandle = dset.getHandle();

				vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS,
										m_graphicsProg->getPipelineLayout().getHandle(), i, 1, &dsHandle,
										dynamicOffsetCount, &dynamicOffsets[0]);
			}
		}
	}

	// Flush viewport
	if(ANKI_UNLIKELY(m_viewportDirty))
	{
		const Bool flipvp = flipViewport();

		U32 fbWidth, fbHeight;
		static_cast<const FramebufferImpl&>(*m_activeFb).getAttachmentsSize(fbWidth, fbHeight);

		VkViewport vp = computeViewport(&m_viewport[0], fbWidth, fbHeight, flipvp);

		// Additional optimization
		if(memcmp(&vp, &m_lastViewport, sizeof(vp)) != 0)
		{
			vkCmdSetViewport(m_handle, 0, 1, &vp);
			m_lastViewport = vp;
		}

		m_viewportDirty = false;
	}

	// Flush scissor
	if(ANKI_UNLIKELY(m_scissorDirty))
	{
		const Bool flipvp = flipViewport();

		U32 fbWidth, fbHeight;
		static_cast<const FramebufferImpl&>(*m_activeFb).getAttachmentsSize(fbWidth, fbHeight);

		VkRect2D scissor = computeScissor(&m_scissor[0], fbWidth, fbHeight, flipvp);

		// Additional optimization
		if(memcmp(&scissor, &m_lastScissor, sizeof(scissor)) != 0)
		{
			vkCmdSetScissor(m_handle, 0, 1, &scissor);
			m_lastScissor = scissor;
		}

		m_scissorDirty = false;
	}

	// VRS
	if(getGrManagerImpl().getDeviceCapabilities().m_vrs && m_vrsRateDirty)
	{
		const VkExtent2D extend = convertVrsShadingRate(m_vrsRate);
		Array<VkFragmentShadingRateCombinerOpKHR, 2> combiner;
		combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR; // Keep pipeline rating over primitive
		combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR; // Max of attachment and pipeline rates
		vkCmdSetFragmentShadingRateKHR(m_handle, &extend, &combiner[0]);

		m_vrsRateDirty = false;
	}

	// Some checks
#if ANKI_ENABLE_ASSERTIONS
	if(m_state.getPrimitiveTopology() == PrimitiveTopology::kLines
	   || m_state.getPrimitiveTopology() == PrimitiveTopology::kLineStip)
	{
		ANKI_ASSERT(m_lineWidthSet == true);
	}
#endif

	ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
}

inline void CommandBufferImpl::fillBufferInternal(const BufferPtr& buff, PtrSize offset, PtrSize size, U32 value)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kTransferDestination));

	ANKI_ASSERT(offset < impl.getSize());
	ANKI_ASSERT((offset % 4) == 0 && "Should be multiple of 4");

	size = (size == kMaxPtrSize) ? (impl.getActualSize() - offset) : size;
	alignRoundUp(4, size); // Needs to be multiple of 4
	ANKI_ASSERT(offset + size <= impl.getActualSize());
	ANKI_ASSERT((size % 4) == 0 && "Should be multiple of 4");

	vkCmdFillBuffer(m_handle, impl.getHandle(), offset, size, value);

	m_microCmdb->pushObjectRef(buff);
}

inline void CommandBufferImpl::writeOcclusionQueriesResultToBufferInternal(ConstWeakArray<OcclusionQuery*> queries,
																		   PtrSize offset, const BufferPtr& buff)
{
	ANKI_ASSERT(queries.getSize() > 0);
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());

	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kTransferDestination));

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		ANKI_ASSERT(queries[i]);
		ANKI_ASSERT((offset % 4) == 0);
		ANKI_ASSERT((offset + sizeof(U32)) <= impl.getSize());

		OcclusionQueryImpl* q = static_cast<OcclusionQueryImpl*>(queries[i]);

		vkCmdCopyQueryPoolResults(m_handle, q->m_handle.getQueryPool(), q->m_handle.getQueryIndex(), 1,
								  impl.getHandle(), offset, sizeof(U32), VK_QUERY_RESULT_PARTIAL_BIT);

		offset += sizeof(U32);
		m_microCmdb->pushObjectRef(q);
	}

	m_microCmdb->pushObjectRef(buff);
}

inline void CommandBufferImpl::bindShaderProgramInternal(const ShaderProgramPtr& prog)
{
	commandCommon();

	ShaderProgramImpl& impl = static_cast<ShaderProgramImpl&>(*prog);

	if(impl.isGraphics())
	{
		m_graphicsProg = &impl;
		m_computeProg = nullptr; // Unbind the compute prog. Doesn't work like vulkan
		m_rtProg = nullptr; // See above
		m_state.bindShaderProgram(&impl);
	}
	else if(!!(impl.getStages() & ShaderTypeBit::kCompute))
	{
		m_computeProg = &impl;
		m_graphicsProg = nullptr; // See comment in the if()
		m_rtProg = nullptr; // See above

		// Bind the pipeline now
		vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, impl.getComputePipelineHandle());
	}
	else
	{
		ANKI_ASSERT(!!(impl.getStages() & ShaderTypeBit::kAllRayTracing));
		m_computeProg = nullptr;
		m_graphicsProg = nullptr;
		m_rtProg = &impl;

		// Bind now
		vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, impl.getRayTracingPipelineHandle());
	}

	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(impl.getReflectionInfo().m_descriptorSetMask.get(i))
		{
			m_dsetState[i].setLayout(impl.getDescriptorSetLayout(i));
		}
		else
		{
			// According to the spec the bound DS may be disturbed if the ppline layout is not compatible. Play it safe
			// and dirty the slot. That will force rebind of the DS at drawcall time.
			m_dsetState[i].setLayout(DescriptorSetLayout());
		}
	}

	m_microCmdb->pushObjectRef(prog);

#if ANKI_EXTRA_CHECKS
	m_setPushConstantsSize = 0;
#endif
}

inline void CommandBufferImpl::copyBufferToBufferInternal(const BufferPtr& src, PtrSize srcOffset, const BufferPtr& dst,
														  PtrSize dstOffset, PtrSize range)
{
	ANKI_ASSERT(static_cast<const BufferImpl&>(*src).usageValid(BufferUsageBit::kTransferSource));
	ANKI_ASSERT(static_cast<const BufferImpl&>(*dst).usageValid(BufferUsageBit::kTransferDestination));
	ANKI_ASSERT(srcOffset + range <= src->getSize());
	ANKI_ASSERT(dstOffset + range <= dst->getSize());

	commandCommon();

	VkBufferCopy region = {};
	region.srcOffset = srcOffset;
	region.dstOffset = dstOffset;
	region.size = range;

	vkCmdCopyBuffer(m_handle, static_cast<const BufferImpl&>(*src).getHandle(),
					static_cast<const BufferImpl&>(*dst).getHandle(), 1, &region);

	m_microCmdb->pushObjectRef(src);
	m_microCmdb->pushObjectRef(dst);
}

inline Bool CommandBufferImpl::flipViewport() const
{
	return static_cast<const FramebufferImpl&>(*m_activeFb).hasPresentableTexture();
}

inline void CommandBufferImpl::setPushConstantsInternal(const void* data, U32 dataSize)
{
	ANKI_ASSERT(data && dataSize && dataSize % 16 == 0);
	const ShaderProgramImpl& prog = getBoundProgram();
	ANKI_ASSERT(prog.getReflectionInfo().m_pushConstantsSize == dataSize
				&& "The bound program should have push constants equal to the \"dataSize\" parameter");

	commandCommon();

	vkCmdPushConstants(m_handle, prog.getPipelineLayout().getHandle(), VK_SHADER_STAGE_ALL, 0, dataSize, data);

#if ANKI_EXTRA_CHECKS
	m_setPushConstantsSize = dataSize;
#endif
}

inline void CommandBufferImpl::setRasterizationOrderInternal(RasterizationOrder order)
{
	commandCommon();

	if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kAMD_rasterization_order))
	{
		m_state.setRasterizationOrder(order);
	}
}

inline void CommandBufferImpl::setLineWidthInternal(F32 width)
{
	commandCommon();
	vkCmdSetLineWidth(m_handle, width);

#if ANKI_ENABLE_ASSERTIONS
	m_lineWidthSet = true;
#endif
}

inline void CommandBufferImpl::setVrsRateInternal(VrsRate rate)
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

} // end namespace anki

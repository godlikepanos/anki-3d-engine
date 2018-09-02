// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/vulkan/OcclusionQueryImpl.h>
#include <anki/core/Trace.h>

namespace anki
{

inline void CommandBufferImpl::setStencilCompareMask(FaceSelectionBit face, U32 mask)
{
	commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::FRONT) && m_stencilCompareMasks[0] != mask)
	{
		m_stencilCompareMasks[0] = mask;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::BACK) && m_stencilCompareMasks[1] != mask)
	{
		m_stencilCompareMasks[1] = mask;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		ANKI_CMD(vkCmdSetStencilCompareMask(m_handle, flags, mask), ANY_OTHER_COMMAND);
	}
}

inline void CommandBufferImpl::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::FRONT) && m_stencilWriteMasks[0] != mask)
	{
		m_stencilWriteMasks[0] = mask;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::BACK) && m_stencilWriteMasks[1] != mask)
	{
		m_stencilWriteMasks[1] = mask;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		ANKI_CMD(vkCmdSetStencilWriteMask(m_handle, flags, mask), ANY_OTHER_COMMAND);
	}
}

inline void CommandBufferImpl::setStencilReference(FaceSelectionBit face, U32 ref)
{
	commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::FRONT) && m_stencilReferenceMasks[0] != ref)
	{
		m_stencilReferenceMasks[0] = ref;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::BACK) && m_stencilReferenceMasks[1] != ref)
	{
		m_stencilWriteMasks[1] = ref;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		ANKI_CMD(vkCmdSetStencilReference(m_handle, flags, ref), ANY_OTHER_COMMAND);
	}
}

inline void CommandBufferImpl::setImageBarrier(VkPipelineStageFlags srcStage,
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

#if ANKI_BATCH_COMMANDS
	flushBatches(CommandBufferCommandType::SET_BARRIER);

	if(m_imgBarriers.getSize() <= m_imgBarrierCount)
	{
		m_imgBarriers.resize(m_alloc, max<U>(2, m_imgBarrierCount * 2));
	}

	m_imgBarriers[m_imgBarrierCount++] = inf;

	m_srcStageMask |= srcStage;
	m_dstStageMask |= dstStage;
#else
	ANKI_CMD(vkCmdPipelineBarrier(m_handle, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &inf), ANY_OTHER_COMMAND);
	ANKI_TRACE_INC_COUNTER(VK_PIPELINE_BARRIERS, 1);
#endif
}

inline void CommandBufferImpl::setTextureBarrierRange(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const VkImageSubresourceRange& range)
{
	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);
	ANKI_ASSERT(impl.usageValid(prevUsage));
	ANKI_ASSERT(impl.usageValid(nextUsage));

	VkPipelineStageFlags srcStage;
	VkAccessFlags srcAccess;
	VkImageLayout oldLayout;
	VkPipelineStageFlags dstStage;
	VkAccessFlags dstAccess;
	VkImageLayout newLayout;
	impl.computeBarrierInfo(prevUsage, nextUsage, range.baseMipLevel, srcStage, srcAccess, dstStage, dstAccess);
	oldLayout = impl.computeLayout(prevUsage, range.baseMipLevel);
	newLayout = impl.computeLayout(nextUsage, range.baseMipLevel);

	setImageBarrier(srcStage, srcAccess, oldLayout, dstStage, dstAccess, newLayout, impl.m_imageHandle, range);

	m_microCmdb->pushObjectRef(tex);
}

inline void CommandBufferImpl::setTextureBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSubresourceInfo& subresource_)
{
	TextureSubresourceInfo subresource = subresource_;
	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);

	// The transition of the non zero mip levels happens inside CommandBufferImpl::generateMipmapsX so limit the
	// subresource
	if(nextUsage == TextureUsageBit::GENERATE_MIPMAPS)
	{
		ANKI_ASSERT(impl.isSubresourceGoodForMipmapGeneration(subresource));

		subresource.m_firstMipmap = 0;
		subresource.m_mipmapCount = 1;
	}

	ANKI_ASSERT(tex->isSubresourceValid(subresource));

	VkImageSubresourceRange range;
	impl.computeVkImageSubresourceRange(subresource, range);
	setTextureBarrierRange(tex, prevUsage, nextUsage, range);
}

inline void CommandBufferImpl::setTextureSurfaceBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf)
{
	if(ANKI_UNLIKELY(surf.m_level > 0 && nextUsage == TextureUsageBit::GENERATE_MIPMAPS))
	{
		// This transition happens inside CommandBufferImpl::generateMipmapsX. No need to do something
		return;
	}

	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);

	VkImageSubresourceRange range;
	impl.computeVkImageSubresourceRange(TextureSubresourceInfo(surf, impl.getDepthStencilAspect()), range);
	setTextureBarrierRange(tex, prevUsage, nextUsage, range);
}

inline void CommandBufferImpl::setTextureVolumeBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol)
{
	if(vol.m_level > 0)
	{
		ANKI_ASSERT(!(nextUsage & TextureUsageBit::GENERATE_MIPMAPS)
					&& "This transition happens inside CommandBufferImpl::generateMipmaps");
	}

	const TextureImpl& impl = static_cast<const TextureImpl&>(*tex);

	VkImageSubresourceRange range;
	impl.computeVkImageSubresourceRange(TextureSubresourceInfo(vol, impl.getDepthStencilAspect()), range);
	setTextureBarrierRange(tex, prevUsage, nextUsage, range);
}

inline void CommandBufferImpl::setBufferBarrier(VkPipelineStageFlags srcStage,
	VkAccessFlags srcAccess,
	VkPipelineStageFlags dstStage,
	VkAccessFlags dstAccess,
	PtrSize offset,
	PtrSize size,
	VkBuffer buff)
{
	ANKI_ASSERT(buff);
	commandCommon();

	VkBufferMemoryBarrier b = {};
	b.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	b.srcAccessMask = srcAccess;
	b.dstAccessMask = dstAccess;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.buffer = buff;
	b.offset = offset;
	b.size = size;

#if ANKI_BATCH_COMMANDS
	flushBatches(CommandBufferCommandType::SET_BARRIER);

	if(m_buffBarriers.getSize() <= m_buffBarrierCount)
	{
		m_buffBarriers.resize(m_alloc, max<U>(2, m_buffBarrierCount * 2));
	}

	m_buffBarriers[m_buffBarrierCount++] = b;

	m_srcStageMask |= srcStage;
	m_dstStageMask |= dstStage;
#else
	ANKI_CMD(vkCmdPipelineBarrier(m_handle, srcStage, dstStage, 0, 0, nullptr, 1, &b, 0, nullptr), ANY_OTHER_COMMAND);
	ANKI_TRACE_INC_COUNTER(VK_PIPELINE_BARRIERS, 1);
#endif
}

inline void CommandBufferImpl::setBufferBarrier(
	BufferPtr buff, BufferUsageBit before, BufferUsageBit after, PtrSize offset, PtrSize size)
{
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);

	VkPipelineStageFlags srcStage;
	VkAccessFlags srcAccess;
	VkPipelineStageFlags dstStage;
	VkAccessFlags dstAccess;
	impl.computeBarrierInfo(before, after, srcStage, srcAccess, dstStage, dstAccess);

	setBufferBarrier(srcStage, srcAccess, dstStage, dstAccess, offset, size, impl.getHandle());

	m_microCmdb->pushObjectRef(buff);
}

inline void CommandBufferImpl::drawArrays(
	PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	ANKI_CMD(vkCmdDraw(m_handle, count, instanceCount, first, baseInstance), ANY_OTHER_COMMAND);
}

inline void CommandBufferImpl::drawElements(
	PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	ANKI_CMD(vkCmdDrawIndexed(m_handle, count, instanceCount, firstIndex, baseVertex, baseInstance), ANY_OTHER_COMMAND);
}

inline void CommandBufferImpl::drawArraysIndirect(
	PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr& buff)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::INDIRECT_GRAPHICS));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(DrawArraysIndirectInfo) * drawCount) <= impl.getSize());

	ANKI_CMD(vkCmdDrawIndirect(m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawArraysIndirectInfo)),
		ANY_OTHER_COMMAND);
}

inline void CommandBufferImpl::drawElementsIndirect(
	PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr& buff)
{
	m_state.setPrimitiveTopology(topology);
	drawcallCommon();
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::INDIRECT_ALL));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(DrawElementsIndirectInfo) * drawCount) <= impl.getSize());

	ANKI_CMD(vkCmdDrawIndexedIndirect(m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawElementsIndirectInfo)),
		ANY_OTHER_COMMAND);
}

inline void CommandBufferImpl::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(m_computeProg);
	ANKI_ASSERT(!!(m_flags & CommandBufferFlag::COMPUTE_WORK));
	ANKI_ASSERT(m_computeProg->getReflectionInfo().m_pushConstantsSize == m_setPushConstantsSize
				&& "Forgot to set pushConstants");

	commandCommon();

	// Bind descriptors
	for(U i = 0; i < MAX_DESCRIPTOR_SETS; ++i)
	{
		if(m_computeProg->getReflectionInfo().m_descriptorSetMask.get(i))
		{
			DescriptorSet dset;
			Bool dirty;
			Array<U32, MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS> dynamicOffsets;
			U dynamicOffsetCount;
			if(getGrManagerImpl().getDescriptorSetFactory().newDescriptorSet(
				   m_tid, m_dsetState[i], dset, dirty, dynamicOffsets, dynamicOffsetCount))
			{
				ANKI_VK_LOGF("Cannot recover");
			}

			if(dirty)
			{
				VkDescriptorSet dsHandle = dset.getHandle();

				ANKI_CMD(vkCmdBindDescriptorSets(m_handle,
							 VK_PIPELINE_BIND_POINT_COMPUTE,
							 m_computeProg->getPipelineLayout().getHandle(),
							 i,
							 1,
							 &dsHandle,
							 dynamicOffsetCount,
							 &dynamicOffsets[0]),
					ANY_OTHER_COMMAND);
			}
		}
	}

	ANKI_CMD(vkCmdDispatch(m_handle, groupCountX, groupCountY, groupCountZ), ANY_OTHER_COMMAND);
}

inline void CommandBufferImpl::resetOcclusionQuery(OcclusionQueryPtr query)
{
	commandCommon();

	VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.m_pool;
	U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.m_queryIndex;
	ANKI_ASSERT(handle);

#if ANKI_BATCH_COMMANDS
	flushBatches(CommandBufferCommandType::RESET_OCCLUSION_QUERY);

	if(m_queryResetAtoms.getSize() <= m_queryResetAtomCount)
	{
		m_queryResetAtoms.resize(m_alloc, max<U>(2, m_queryResetAtomCount * 2));
	}

	QueryResetAtom atom;
	atom.m_pool = handle;
	atom.m_queryIdx = idx;

	m_queryResetAtoms[m_queryResetAtomCount++] = atom;
#else
	ANKI_CMD(vkCmdResetQueryPool(m_handle, handle, idx, 1), ANY_OTHER_COMMAND);
#endif

	m_microCmdb->pushObjectRef(query);
}

inline void CommandBufferImpl::beginOcclusionQuery(OcclusionQueryPtr query)
{
	commandCommon();

	VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.m_pool;
	U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.m_queryIndex;
	ANKI_ASSERT(handle);

	ANKI_CMD(vkCmdBeginQuery(m_handle, handle, idx, 0), ANY_OTHER_COMMAND);

	m_microCmdb->pushObjectRef(query);
}

inline void CommandBufferImpl::endOcclusionQuery(OcclusionQueryPtr query)
{
	commandCommon();

	VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.m_pool;
	U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.m_queryIndex;
	ANKI_ASSERT(handle);

	ANKI_CMD(vkCmdEndQuery(m_handle, handle, idx), ANY_OTHER_COMMAND);

	m_microCmdb->pushObjectRef(query);
}

inline void CommandBufferImpl::clearTextureView(TextureViewPtr texView, const ClearValue& clearValue)
{
	commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = static_cast<const TextureImpl&>(*view.m_tex);

	VkClearColorValue vclear;
	static_assert(sizeof(vclear) == sizeof(clearValue), "See file");
	memcpy(&vclear, &clearValue, sizeof(clearValue));

	if(!view.getSubresource().m_depthStencilAspect)
	{
		VkImageSubresourceRange vkRange = view.getVkImageSubresourceRange();
		ANKI_CMD(vkCmdClearColorImage(
					 m_handle, tex.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vclear, 1, &vkRange),
			ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	m_microCmdb->pushObjectRef(texView);
}

inline void CommandBufferImpl::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	commandCommon();
	ANKI_ASSERT(insideRenderPass());
	ANKI_ASSERT(m_subpassContents == VK_SUBPASS_CONTENTS_MAX_ENUM
				|| m_subpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	ANKI_ASSERT(static_cast<const CommandBufferImpl&>(*cmdb).m_finalized);

	m_subpassContents = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

	if(ANKI_UNLIKELY(m_rpCommandCount == 0))
	{
		beginRenderPassInternal();
	}

#if ANKI_BATCH_COMMANDS
	flushBatches(CommandBufferCommandType::PUSH_SECOND_LEVEL);

	if(m_secondLevelAtoms.getSize() <= m_secondLevelAtomCount)
	{
		m_secondLevelAtoms.resize(m_alloc, max<U>(8, m_secondLevelAtomCount * 2));
	}

	m_secondLevelAtoms[m_secondLevelAtomCount++] = static_cast<const CommandBufferImpl&>(*cmdb).m_handle;
#else
	ANKI_CMD(
		vkCmdExecuteCommands(m_handle, 1, &static_cast<const CommandBufferImpl&>(*cmdb).m_handle), ANY_OTHER_COMMAND);
#endif

	++m_rpCommandCount;
	m_microCmdb->pushObjectRef(cmdb);
}

inline void CommandBufferImpl::drawcallCommon()
{
	// Preconditions
	commandCommon();
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
	ANKI_ASSERT(m_graphicsProg);
	Pipeline ppline;
	Bool stateDirty;
	m_graphicsProg->getPipelineFactory().newPipeline(m_state, ppline, stateDirty);

	if(stateDirty)
	{
		ANKI_CMD(vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ppline.getHandle()), ANY_OTHER_COMMAND);
	}

	// Bind dsets
	for(U i = 0; i < MAX_DESCRIPTOR_SETS; ++i)
	{
		if(m_graphicsProg->getReflectionInfo().m_descriptorSetMask.get(i))
		{
			DescriptorSet dset;
			Bool dirty;
			Array<U32, MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS> dynamicOffsets;
			U dynamicOffsetCount;
			if(getGrManagerImpl().getDescriptorSetFactory().newDescriptorSet(
				   m_tid, m_dsetState[i], dset, dirty, dynamicOffsets, dynamicOffsetCount))
			{
				ANKI_VK_LOGF("Cannot recover");
			}

			if(dirty)
			{
				VkDescriptorSet dsHandle = dset.getHandle();

				ANKI_CMD(vkCmdBindDescriptorSets(m_handle,
							 VK_PIPELINE_BIND_POINT_GRAPHICS,
							 m_graphicsProg->getPipelineLayout().getHandle(),
							 i,
							 1,
							 &dsHandle,
							 dynamicOffsetCount,
							 &dynamicOffsets[0]),
					ANY_OTHER_COMMAND);
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
			ANKI_CMD(vkCmdSetViewport(m_handle, 0, 1, &vp), ANY_OTHER_COMMAND);
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
			ANKI_CMD(vkCmdSetScissor(m_handle, 0, 1, &scissor), ANY_OTHER_COMMAND);
			m_lastScissor = scissor;
		}

		m_scissorDirty = false;
	}

	ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
}

inline void CommandBufferImpl::commandCommon()
{
	ANKI_ASSERT(!m_finalized);

#if ANKI_EXTRA_CHECKS
	++m_commandCount;
#endif

	m_empty = false;

	if(ANKI_UNLIKELY(!m_beganRecording))
	{
		beginRecording();
		m_beganRecording = true;
	}

	ANKI_ASSERT(Thread::getCurrentThreadId() == m_tid
				&& "Commands must be recorder and flushed by the thread this command buffer was created");

	ANKI_ASSERT(m_handle);
}

inline void CommandBufferImpl::flushBatches(CommandBufferCommandType type)
{
	if(type != m_lastCmdType)
	{
		switch(m_lastCmdType)
		{
		case CommandBufferCommandType::SET_BARRIER:
			flushBarriers();
			break;
		case CommandBufferCommandType::RESET_OCCLUSION_QUERY:
			flushQueryResets();
			break;
		case CommandBufferCommandType::WRITE_QUERY_RESULT:
			flushWriteQueryResults();
			break;
		case CommandBufferCommandType::PUSH_SECOND_LEVEL:
			ANKI_ASSERT(m_secondLevelAtomCount > 0);
			vkCmdExecuteCommands(m_handle, m_secondLevelAtomCount, &m_secondLevelAtoms[0]);
			m_secondLevelAtomCount = 0;
			break;
		case CommandBufferCommandType::ANY_OTHER_COMMAND:
			break;
		default:
			ANKI_ASSERT(0);
		}

		m_lastCmdType = type;
	}
}

inline void CommandBufferImpl::fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::FILL));

	ANKI_ASSERT(offset < impl.getSize());
	ANKI_ASSERT((offset % 4) == 0 && "Should be multiple of 4");

	size = (size == MAX_PTR_SIZE) ? (impl.getActualSize() - offset) : size;
	alignRoundUp(4, size); // Needs to be multiple of 4
	ANKI_ASSERT(offset + size <= impl.getActualSize());
	ANKI_ASSERT((size % 4) == 0 && "Should be multiple of 4");

	ANKI_CMD(vkCmdFillBuffer(m_handle, impl.getHandle(), offset, size, value), ANY_OTHER_COMMAND);

	m_microCmdb->pushObjectRef(buff);
}

inline void CommandBufferImpl::writeOcclusionQueryResultToBuffer(
	OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());

	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::QUERY_RESULT));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(U32)) <= impl.getSize());

	const OcclusionQueryImpl& q = static_cast<const OcclusionQueryImpl&>(*query);

#if ANKI_BATCH_COMMANDS
	flushBatches(CommandBufferCommandType::WRITE_QUERY_RESULT);

	if(m_writeQueryAtoms.getSize() <= m_writeQueryAtomCount)
	{
		m_writeQueryAtoms.resize(m_alloc, max<U>(2, m_writeQueryAtomCount * 2));
	}

	WriteQueryAtom atom;
	atom.m_pool = q.m_handle.m_pool;
	atom.m_queryIdx = q.m_handle.m_queryIndex;
	atom.m_buffer = impl.getHandle();
	atom.m_offset = offset;

	m_writeQueryAtoms[m_writeQueryAtomCount++] = atom;
#else
	ANKI_CMD(vkCmdCopyQueryPoolResults(m_handle,
				 q.m_handle.m_pool,
				 q.m_handle.m_queryIndex,
				 1,
				 impl.getHandle(),
				 offset,
				 sizeof(U32),
				 VK_QUERY_RESULT_PARTIAL_BIT),
		ANY_OTHER_COMMAND);
#endif

	m_microCmdb->pushObjectRef(query);
	m_microCmdb->pushObjectRef(buff);
}

inline void CommandBufferImpl::bindShaderProgram(ShaderProgramPtr& prog)
{
	commandCommon();

	ShaderProgramImpl& impl = static_cast<ShaderProgramImpl&>(*prog);

	if(impl.isGraphics())
	{
		m_graphicsProg = &impl;
		m_computeProg = nullptr; // Unbind the compute prog. Doesn't work like vulkan
		m_state.bindShaderProgram(prog);
	}
	else
	{
		m_computeProg = &impl;
		m_graphicsProg = nullptr; // See comment in the if()

		// Bind the pipeline now
		ANKI_CMD(vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, impl.getComputePipelineHandle()),
			ANY_OTHER_COMMAND);
	}

	for(U i = 0; i < MAX_DESCRIPTOR_SETS; ++i)
	{
		if(impl.getReflectionInfo().m_descriptorSetMask.get(i))
		{
			m_dsetState[i].setLayout(impl.getDescriptorSetLayout(i));
		}
	}

	m_microCmdb->pushObjectRef(prog);

#if ANKI_EXTRA_CHECKS
	m_setPushConstantsSize = 0;
#endif
}

inline void CommandBufferImpl::copyBufferToBuffer(
	BufferPtr& src, PtrSize srcOffset, BufferPtr& dst, PtrSize dstOffset, PtrSize range)
{
	commandCommon();

	VkBufferCopy region = {};
	region.srcOffset = srcOffset;
	region.dstOffset = dstOffset;
	region.size = range;

	ANKI_CMD(vkCmdCopyBuffer(m_handle,
				 static_cast<const BufferImpl&>(*src).getHandle(),
				 static_cast<const BufferImpl&>(*dst).getHandle(),
				 1,
				 &region),
		ANY_OTHER_COMMAND);

	m_microCmdb->pushObjectRef(src);
	m_microCmdb->pushObjectRef(dst);
}

inline Bool CommandBufferImpl::flipViewport() const
{
	return static_cast<const FramebufferImpl&>(*m_activeFb).hasPresentableTexture()
		   && !!(getGrManagerImpl().getExtensions() & VulkanExtensions::KHR_MAINENANCE1);
}

inline void CommandBufferImpl::setPushConstants(const void* data, U32 dataSize)
{
	ANKI_ASSERT(data && dataSize && dataSize % 16 == 0);
	const ShaderProgramImpl* prog = (m_graphicsProg) ? m_graphicsProg : m_computeProg;
	ANKI_ASSERT(prog && "Need have bound the ShaderProgram first");
	ANKI_ASSERT(prog->getReflectionInfo().m_pushConstantsSize == dataSize
				&& "The bound program should have push constants equal to the \"dataSize\" parameter");

	commandCommon();

	ANKI_CMD(
		vkCmdPushConstants(m_handle, prog->getPipelineLayout().getHandle(), VK_SHADER_STAGE_ALL, 0, dataSize, data),
		ANY_OTHER_COMMAND);

#if ANKI_EXTRA_CHECKS
	m_setPushConstantsSize = dataSize;
#endif
}

inline void CommandBufferImpl::setRasterizationOrder(RasterizationOrder order)
{
	commandCommon();

	if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::AMD_RASTERIZATION_ORDER))
	{
		m_state.setRasterizationOrder(order);
	}
}

} // end namespace anki

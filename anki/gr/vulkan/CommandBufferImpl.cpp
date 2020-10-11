// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/gr/vulkan/AccelerationStructureImpl.h>

#include <algorithm>

namespace anki
{

CommandBufferImpl::~CommandBufferImpl()
{
	if(m_empty)
	{
		ANKI_VK_LOGW("Command buffer was empty");
	}

	if(!m_finalized)
	{
		ANKI_VK_LOGW("Command buffer was not flushed");
	}

	m_imgBarriers.destroy(m_alloc);
	m_buffBarriers.destroy(m_alloc);
	m_memBarriers.destroy(m_alloc);
	m_queryResetAtoms.destroy(m_alloc);
	m_writeQueryAtoms.destroy(m_alloc);
	m_secondLevelAtoms.destroy(m_alloc);
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	m_tid = Thread::getCurrentThreadId();
	m_flags = init.m_flags;

	ANKI_CHECK(getGrManagerImpl().getCommandBufferFactory().newCommandBuffer(m_tid, m_flags, m_microCmdb));
	m_handle = m_microCmdb->getHandle();

	m_alloc = m_microCmdb->getFastAllocator();

	// Store some of the init info for later
	if(!!(m_flags & CommandBufferFlag::SECOND_LEVEL))
	{
		m_activeFb = init.m_framebuffer;
		m_colorAttachmentUsages = init.m_colorAttachmentUsages;
		m_depthStencilAttachmentUsage = init.m_depthStencilAttachmentUsage;
		m_state.beginRenderPass(m_activeFb);
	}

	for(DescriptorSetState& state : m_dsetState)
	{
		state.init(m_alloc);
	}

	return Error::NONE;
}

void CommandBufferImpl::beginRecording()
{
	// Do the begin
	VkCommandBufferInheritanceInfo inheritance = {};
	inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin.pInheritanceInfo = &inheritance;

	if(!!(m_flags & CommandBufferFlag::SECOND_LEVEL))
	{
		FramebufferImpl& impl = static_cast<FramebufferImpl&>(*m_activeFb);

		// Calc the layouts
		Array<VkImageLayout, MAX_COLOR_ATTACHMENTS> colAttLayouts;
		for(U i = 0; i < impl.getColorAttachmentCount(); ++i)
		{
			const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*impl.getColorAttachment(i));
			colAttLayouts[i] = view.getTextureImpl().computeLayout(m_colorAttachmentUsages[i], 0);
		}

		VkImageLayout dsAttLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
		if(impl.hasDepthStencil())
		{
			const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*impl.getDepthStencilAttachment());
			dsAttLayout = view.getTextureImpl().computeLayout(m_depthStencilAttachmentUsage, 0);
		}

		inheritance.renderPass = impl.getRenderPassHandle(colAttLayouts, dsAttLayout);
		inheritance.subpass = 0;
		inheritance.framebuffer = impl.getFramebufferHandle();

		begin.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}

	vkBeginCommandBuffer(m_handle, &begin);
}

void CommandBufferImpl::beginRenderPass(FramebufferPtr fb,
										const Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS>& colorAttachmentUsages,
										TextureUsageBit depthStencilAttachmentUsage, U32 minx, U32 miny, U32 width,
										U32 height)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());

	m_rpCommandCount = 0;
	m_activeFb = fb;

	FramebufferImpl& fbimpl = static_cast<FramebufferImpl&>(*fb);

	U32 fbWidth, fbHeight;
	fbimpl.getAttachmentsSize(fbWidth, fbHeight);
	m_fbSize[0] = fbWidth;
	m_fbSize[1] = fbHeight;

	ANKI_ASSERT(minx < fbWidth && miny < fbHeight);

	const U32 maxx = min<U32>(minx + width, fbWidth);
	const U32 maxy = min<U32>(miny + height, fbHeight);
	width = maxx - minx;
	height = maxy - miny;
	ANKI_ASSERT(minx + width <= fbWidth && miny + height <= fbHeight);

	m_renderArea[0] = minx;
	m_renderArea[1] = miny;
	m_renderArea[2] = width;
	m_renderArea[3] = height;

	m_colorAttachmentUsages = colorAttachmentUsages;
	m_depthStencilAttachmentUsage = depthStencilAttachmentUsage;

	m_microCmdb->pushObjectRef(fb);

	m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;

	// Re-set the viewport and scissor because sometimes they are set clamped
	m_viewportDirty = true;
	m_scissorDirty = true;
}

void CommandBufferImpl::beginRenderPassInternal()
{
	m_state.beginRenderPass(m_activeFb);

	FramebufferImpl& impl = static_cast<FramebufferImpl&>(*m_activeFb);

	VkRenderPassBeginInfo bi = {};
	bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	bi.clearValueCount = impl.getAttachmentCount();
	bi.pClearValues = impl.getClearValues();
	bi.framebuffer = impl.getFramebufferHandle();

	// Calc the layouts
	Array<VkImageLayout, MAX_COLOR_ATTACHMENTS> colAttLayouts;
	for(U i = 0; i < impl.getColorAttachmentCount(); ++i)
	{
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*impl.getColorAttachment(i));
		colAttLayouts[i] = view.getTextureImpl().computeLayout(m_colorAttachmentUsages[i], 0);
	}

	VkImageLayout dsAttLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
	if(impl.hasDepthStencil())
	{
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*impl.getDepthStencilAttachment());
		dsAttLayout = view.getTextureImpl().computeLayout(m_depthStencilAttachmentUsage, 0);
	}

	bi.renderPass = impl.getRenderPassHandle(colAttLayouts, dsAttLayout);

	const Bool flipvp = flipViewport();
	bi.renderArea.offset.x = m_renderArea[0];
	if(flipvp)
	{
		ANKI_ASSERT(m_renderArea[3] <= m_fbSize[1]);
	}
	bi.renderArea.offset.y = (flipvp) ? m_fbSize[1] - (m_renderArea[1] + m_renderArea[3]) : m_renderArea[1];
	bi.renderArea.extent.width = m_renderArea[2];
	bi.renderArea.extent.height = m_renderArea[3];

	getGrManagerImpl().beginMarker(m_handle, impl.getName());
	ANKI_CMD(vkCmdBeginRenderPass(m_handle, &bi, m_subpassContents), ANY_OTHER_COMMAND);

	if(impl.hasPresentableTexture())
	{
		m_renderedToDefaultFb = true;
	}
}

void CommandBufferImpl::endRenderPass()
{
	commandCommon();
	ANKI_ASSERT(insideRenderPass());
	if(m_rpCommandCount == 0)
	{
		m_subpassContents = VK_SUBPASS_CONTENTS_INLINE;
		beginRenderPassInternal();
	}

	ANKI_CMD(vkCmdEndRenderPass(m_handle), ANY_OTHER_COMMAND);
	getGrManagerImpl().endMarker(m_handle);

	m_activeFb.reset(nullptr);
	m_state.endRenderPass();

	// After pushing second level command buffers the state is undefined. Reset the tracker and rebind the dynamic state
	if(m_subpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS)
	{
		m_state.reset();
		rebindDynamicState();
	}
}

void CommandBufferImpl::endRecording()
{
	commandCommon();

	ANKI_ASSERT(!m_finalized);
	ANKI_ASSERT(!m_empty);

	ANKI_CMD(ANKI_VK_CHECKF(vkEndCommandBuffer(m_handle)), ANY_OTHER_COMMAND);
	m_finalized = true;

#if ANKI_EXTRA_CHECKS
	if(!!(m_flags & CommandBufferFlag::SMALL_BATCH))
	{
		if(m_commandCount > COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS * 4)
		{
			ANKI_VK_LOGW("Command buffer has too many commands: %u", m_commandCount);
		}
	}
	else
	{
		if(m_commandCount <= COMMAND_BUFFER_SMALL_BATCH_MAX_COMMANDS / 4)
		{
			ANKI_VK_LOGW("Command buffer has too few commands: %u", m_commandCount);
		}
	}
#endif
}

void CommandBufferImpl::generateMipmaps2d(TextureViewPtr texView)
{
	commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();
	ANKI_ASSERT(tex.getTextureType() != TextureType::_3D && "Not for 3D");
	ANKI_ASSERT(tex.isSubresourceGoodForMipmapGeneration(view.getSubresource()));

	const U32 blitCount = tex.getMipmapCount() - 1u;
	if(blitCount == 0)
	{
		// Nothing to be done, flush the previous commands though because you may batch (and sort) things you shouldn't
		flushBatches(CommandBufferCommandType::ANY_OTHER_COMMAND);
		return;
	}

	const DepthStencilAspectBit aspect = view.getSubresource().m_depthStencilAspect;
	const U32 face = view.getSubresource().m_firstFace;
	const U32 layer = view.getSubresource().m_firstLayer;

	for(U32 i = 0; i < blitCount; ++i)
	{
		// Transition source
		if(i > 0)
		{
			VkImageSubresourceRange range;
			tex.computeVkImageSubresourceRange(TextureSubresourceInfo(TextureSurfaceInfo(i, 0, face, layer), aspect),
											   range);

			setImageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
							VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex.m_imageHandle,
							range);
		}

		// Transition destination
		{
			VkImageSubresourceRange range;
			tex.computeVkImageSubresourceRange(
				TextureSubresourceInfo(TextureSurfaceInfo(i + 1, 0, face, layer), aspect), range);

			setImageBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED,
							VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
							VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, tex.m_imageHandle, range);
		}

		// Setup the blit struct
		I32 srcWidth = tex.getWidth() >> i;
		I32 srcHeight = tex.getHeight() >> i;

		I32 dstWidth = tex.getWidth() >> (i + 1);
		I32 dstHeight = tex.getHeight() >> (i + 1);

		ANKI_ASSERT(srcWidth > 0 && srcHeight > 0 && dstWidth > 0 && dstHeight > 0);

		U32 vkLayer = 0;
		switch(tex.getTextureType())
		{
		case TextureType::_2D:
		case TextureType::_2D_ARRAY:
			break;
		case TextureType::CUBE:
			vkLayer = face;
			break;
		case TextureType::CUBE_ARRAY:
			vkLayer = layer * 6 + face;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		VkImageBlit blit;
		blit.srcSubresource.aspectMask = convertImageAspect(aspect);
		blit.srcSubresource.baseArrayLayer = vkLayer;
		blit.srcSubresource.layerCount = 1;
		blit.srcSubresource.mipLevel = i;
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {srcWidth, srcHeight, 1};

		blit.dstSubresource.aspectMask = convertImageAspect(aspect);
		blit.dstSubresource.baseArrayLayer = vkLayer;
		blit.dstSubresource.layerCount = 1;
		blit.dstSubresource.mipLevel = i + 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {dstWidth, dstHeight, 1};

		ANKI_CMD(vkCmdBlitImage(m_handle, tex.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex.m_imageHandle,
								VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
								(!!aspect) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR),
				 ANY_OTHER_COMMAND);
	}

	// Hold the reference
	m_microCmdb->pushObjectRef(texView);
}

void CommandBufferImpl::flushBarriers()
{
	if(m_imgBarrierCount == 0 && m_buffBarrierCount == 0 && m_memBarrierCount == 0)
	{
		return;
	}

	// Sort
	//

	if(m_imgBarrierCount > 0)
	{
		std::sort(&m_imgBarriers[0], &m_imgBarriers[0] + m_imgBarrierCount,
				  [](const VkImageMemoryBarrier& a, const VkImageMemoryBarrier& b) -> Bool {
					  if(a.image != b.image)
					  {
						  return a.image < b.image;
					  }

					  if(a.subresourceRange.aspectMask != b.subresourceRange.aspectMask)
					  {
						  return a.subresourceRange.aspectMask < b.subresourceRange.aspectMask;
					  }

					  if(a.oldLayout != b.oldLayout)
					  {
						  return a.oldLayout < b.oldLayout;
					  }

					  if(a.newLayout != b.newLayout)
					  {
						  return a.newLayout < b.newLayout;
					  }

					  if(a.subresourceRange.baseArrayLayer != b.subresourceRange.baseArrayLayer)
					  {
						  return a.subresourceRange.baseArrayLayer < b.subresourceRange.baseArrayLayer;
					  }

					  if(a.subresourceRange.baseMipLevel != b.subresourceRange.baseMipLevel)
					  {
						  return a.subresourceRange.baseMipLevel < b.subresourceRange.baseMipLevel;
					  }

					  return false;
				  });
	}

	// Batch
	//

	DynamicArrayAuto<VkImageMemoryBarrier> finalImgBarriers(m_alloc);
	U32 finalImgBarrierCount = 0;
	if(m_imgBarrierCount > 0)
	{
		DynamicArrayAuto<VkImageMemoryBarrier> squashedBarriers(m_alloc);
		U32 squashedBarrierCount = 0;

		squashedBarriers.create(m_imgBarrierCount);

		// Squash the mips by reducing the barriers
		for(U32 i = 0; i < m_imgBarrierCount; ++i)
		{
			const VkImageMemoryBarrier* prev = (i > 0) ? &m_imgBarriers[i - 1] : nullptr;
			const VkImageMemoryBarrier& crnt = m_imgBarriers[i];

			if(prev && prev->image == crnt.image
			   && prev->subresourceRange.aspectMask == crnt.subresourceRange.aspectMask
			   && prev->oldLayout == crnt.oldLayout && prev->newLayout == crnt.newLayout
			   && prev->srcAccessMask == crnt.srcAccessMask && prev->dstAccessMask == crnt.dstAccessMask
			   && prev->subresourceRange.baseMipLevel + prev->subresourceRange.levelCount
					  == crnt.subresourceRange.baseMipLevel
			   && prev->subresourceRange.baseArrayLayer == crnt.subresourceRange.baseArrayLayer
			   && prev->subresourceRange.layerCount == crnt.subresourceRange.layerCount)
			{
				// Can batch
				squashedBarriers[squashedBarrierCount - 1].subresourceRange.levelCount +=
					crnt.subresourceRange.levelCount;
			}
			else
			{
				// Can't batch, create new barrier
				squashedBarriers[squashedBarrierCount++] = crnt;
			}
		}

		ANKI_ASSERT(squashedBarrierCount);

		// Squash the layers
		finalImgBarriers.create(squashedBarrierCount);

		for(U32 i = 0; i < squashedBarrierCount; ++i)
		{
			const VkImageMemoryBarrier* prev = (i > 0) ? &squashedBarriers[i - 1] : nullptr;
			const VkImageMemoryBarrier& crnt = squashedBarriers[i];

			if(prev && prev->image == crnt.image
			   && prev->subresourceRange.aspectMask == crnt.subresourceRange.aspectMask
			   && prev->oldLayout == crnt.oldLayout && prev->newLayout == crnt.newLayout
			   && prev->srcAccessMask == crnt.srcAccessMask && prev->dstAccessMask == crnt.dstAccessMask
			   && prev->subresourceRange.baseMipLevel == crnt.subresourceRange.baseMipLevel
			   && prev->subresourceRange.levelCount == crnt.subresourceRange.levelCount
			   && prev->subresourceRange.baseArrayLayer + prev->subresourceRange.layerCount
					  == crnt.subresourceRange.baseArrayLayer)
			{
				// Can batch
				finalImgBarriers[finalImgBarrierCount - 1].subresourceRange.layerCount +=
					crnt.subresourceRange.layerCount;
			}
			else
			{
				// Can't batch, create new barrier
				finalImgBarriers[finalImgBarrierCount++] = crnt;
			}
		}

		ANKI_ASSERT(finalImgBarrierCount);
	}

	// Finish the job
	//
	vkCmdPipelineBarrier(m_handle, m_srcStageMask, m_dstStageMask, 0, m_memBarrierCount,
						 (m_memBarrierCount) ? &m_memBarriers[0] : nullptr, m_buffBarrierCount,
						 (m_buffBarrierCount) ? &m_buffBarriers[0] : nullptr, finalImgBarrierCount,
						 (finalImgBarrierCount) ? &finalImgBarriers[0] : nullptr);

	ANKI_TRACE_INC_COUNTER(VK_PIPELINE_BARRIERS, 1);

	m_imgBarrierCount = 0;
	m_buffBarrierCount = 0;
	m_memBarrierCount = 0;
	m_srcStageMask = 0;
	m_dstStageMask = 0;
}

void CommandBufferImpl::flushQueryResets()
{
	if(m_queryResetAtoms.getSize() == 0)
	{
		return;
	}

	std::sort(m_queryResetAtoms.getBegin(), m_queryResetAtoms.getEnd(),
			  [](const QueryResetAtom& a, const QueryResetAtom& b) -> Bool {
				  if(a.m_pool != b.m_pool)
				  {
					  return a.m_pool < b.m_pool;
				  }

				  ANKI_ASSERT(a.m_queryIdx != b.m_queryIdx && "Tried to reset the same query more than once");
				  return a.m_queryIdx < b.m_queryIdx;
			  });

	U32 firstQuery = m_queryResetAtoms[0].m_queryIdx;
	U32 queryCount = 1;
	VkQueryPool pool = m_queryResetAtoms[0].m_pool;
	for(U32 i = 1; i < m_queryResetAtoms.getSize(); ++i)
	{
		const QueryResetAtom& crnt = m_queryResetAtoms[i];
		const QueryResetAtom& prev = m_queryResetAtoms[i - 1];

		if(crnt.m_pool == prev.m_pool && crnt.m_queryIdx == prev.m_queryIdx + 1)
		{
			// Can batch
			++queryCount;
		}
		else
		{
			// Flush batch
			vkCmdResetQueryPool(m_handle, pool, firstQuery, queryCount);

			// New batch
			firstQuery = crnt.m_queryIdx;
			queryCount = 1;
			pool = crnt.m_pool;
		}
	}

	vkCmdResetQueryPool(m_handle, pool, firstQuery, queryCount);

	m_queryResetAtoms.destroy(m_alloc);
}

void CommandBufferImpl::flushWriteQueryResults()
{
	if(m_writeQueryAtoms.getSize() == 0)
	{
		return;
	}

	std::sort(&m_writeQueryAtoms[0], &m_writeQueryAtoms[0] + m_writeQueryAtoms.getSize(),
			  [](const WriteQueryAtom& a, const WriteQueryAtom& b) -> Bool {
				  if(a.m_pool != b.m_pool)
				  {
					  return a.m_pool < b.m_pool;
				  }

				  if(a.m_buffer != b.m_buffer)
				  {
					  return a.m_buffer < b.m_buffer;
				  }

				  if(a.m_offset != b.m_offset)
				  {
					  return a.m_offset < b.m_offset;
				  }

				  ANKI_ASSERT(a.m_queryIdx != b.m_queryIdx && "Tried to write the same query more than once");
				  return a.m_queryIdx < b.m_queryIdx;
			  });

	U32 firstQuery = m_writeQueryAtoms[0].m_queryIdx;
	U32 queryCount = 1;
	VkQueryPool pool = m_writeQueryAtoms[0].m_pool;
	PtrSize offset = m_writeQueryAtoms[0].m_offset;
	VkBuffer buff = m_writeQueryAtoms[0].m_buffer;
	for(U32 i = 1; i < m_writeQueryAtoms.getSize(); ++i)
	{
		const WriteQueryAtom& crnt = m_writeQueryAtoms[i];
		const WriteQueryAtom& prev = m_writeQueryAtoms[i - 1];

		if(crnt.m_pool == prev.m_pool && crnt.m_buffer == prev.m_buffer && prev.m_queryIdx + 1 == crnt.m_queryIdx
		   && prev.m_offset + sizeof(U32) == crnt.m_offset)
		{
			// Can batch
			++queryCount;
		}
		else
		{
			// Flush batch
			vkCmdCopyQueryPoolResults(m_handle, pool, firstQuery, queryCount, buff, offset, sizeof(U32),
									  VK_QUERY_RESULT_PARTIAL_BIT);

			// New batch
			firstQuery = crnt.m_queryIdx;
			queryCount = 1;
			pool = crnt.m_pool;
			buff = crnt.m_buffer;
		}
	}

	vkCmdCopyQueryPoolResults(m_handle, pool, firstQuery, queryCount, buff, offset, sizeof(U32),
							  VK_QUERY_RESULT_PARTIAL_BIT);

	m_writeQueryAtoms.resize(m_alloc, 0);
}

void CommandBufferImpl::copyBufferToTextureViewInternal(BufferPtr buff, PtrSize offset, PtrSize range,
														TextureViewPtr texView)
{
	commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();
	ANKI_ASSERT(tex.usageValid(TextureUsageBit::TRANSFER_DESTINATION));
	ANKI_ASSERT(tex.isSubresourceGoodForCopyFromBuffer(view.getSubresource()));
	const VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	const Bool is3D = tex.getTextureType() == TextureType::_3D;
	const VkImageAspectFlags aspect = convertImageAspect(view.getSubresource().m_depthStencilAspect);

	const TextureSurfaceInfo surf(view.getSubresource().m_firstMipmap, view.getSubresource().m_firstFace, 0,
								  view.getSubresource().m_firstLayer);
	const TextureVolumeInfo vol(view.getSubresource().m_firstMipmap);

	// Compute the sizes of the mip
	const U32 width = tex.getWidth() >> surf.m_level;
	const U32 height = tex.getHeight() >> surf.m_level;
	ANKI_ASSERT(width && height);
	const U32 depth = (is3D) ? (tex.getDepth() >> surf.m_level) : 1u;

	if(!tex.m_workarounds)
	{
		if(!is3D)
		{
			ANKI_ASSERT(range == computeSurfaceSize(width, height, tex.getFormat()));
		}
		else
		{
			ANKI_ASSERT(range == computeVolumeSize(width, height, depth, tex.getFormat()));
		}

		// Copy
		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.baseArrayLayer = (is3D) ? tex.computeVkArrayLayer(vol) : tex.computeVkArrayLayer(surf);
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = surf.m_level;
		region.imageOffset = {0, 0, 0};
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = depth;
		region.bufferOffset = offset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		ANKI_CMD(vkCmdCopyBufferToImage(m_handle, static_cast<const BufferImpl&>(*buff).getHandle(), tex.m_imageHandle,
										layout, 1, &region),
				 ANY_OTHER_COMMAND);
	}
	else if(!!(tex.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Create a new shadow buffer
		const PtrSize shadowSize = (is3D) ? computeVolumeSize(width, height, depth, Format::R8G8B8A8_UNORM)
										  : computeSurfaceSize(width, height, Format::R8G8B8A8_UNORM);
		BufferPtr shadow = getManager().newBuffer(
			BufferInitInfo(shadowSize, BufferUsageBit::ALL_TRANSFER, BufferMapAccessBit::NONE, "Workaround"));
		const VkBuffer shadowHandle = static_cast<const BufferImpl&>(*shadow).getHandle();
		m_microCmdb->pushObjectRef(shadow);

		// Copy to shadow buffer in batches. If the number of pixels is high and we do a single vkCmdCopyBuffer we will
		// need many regions. That allocation will be huge so do the copies in batches.
		const U32 regionCount = width * height * depth;
		const U32 REGIONS_PER_CMD_COPY_BUFFER = 32;
		const U32 cmdCopyBufferCount = (regionCount + REGIONS_PER_CMD_COPY_BUFFER - 1) / REGIONS_PER_CMD_COPY_BUFFER;
		for(U32 cmdCopyBuffer = 0; cmdCopyBuffer < cmdCopyBufferCount; ++cmdCopyBuffer)
		{
			const U32 beginRegion = cmdCopyBuffer * REGIONS_PER_CMD_COPY_BUFFER;
			const U32 endRegion = min(regionCount, (cmdCopyBuffer + 1) * REGIONS_PER_CMD_COPY_BUFFER);
			ANKI_ASSERT(beginRegion < regionCount);
			ANKI_ASSERT(endRegion <= regionCount);

			const U32 crntRegionCount = endRegion - beginRegion;
			DynamicArrayAuto<VkBufferCopy> regions(m_alloc);
			regions.create(crntRegionCount);

			// Populate regions
			U32 count = 0;
			for(U32 regionIdx = beginRegion; regionIdx < endRegion; ++regionIdx)
			{
				U32 x, y, d;
				unflatten3dArrayIndex(width, height, depth, regionIdx, x, y, d);

				VkBufferCopy& c = regions[count++];

				if(is3D)
				{
					c.srcOffset = (d * height * width + y * width + x) * 3 + offset;
					c.dstOffset = (d * height * width + y * width + x) * 4 + 0;
				}
				else
				{
					c.srcOffset = (y * width + x) * 3 + offset;
					c.dstOffset = (y * width + x) * 4 + 0;
				}
				c.size = 3;
			}

			// Do the copy to the shadow buffer
			ANKI_CMD(vkCmdCopyBuffer(m_handle, static_cast<const BufferImpl&>(*buff).getHandle(), shadowHandle,
									 regions.getSize(), &regions[0]),
					 ANY_OTHER_COMMAND);
		}

		// Set barrier
		setBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
						 VK_ACCESS_TRANSFER_READ_BIT, 0, shadowSize, shadowHandle);

		// Do the copy to the image
		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.baseArrayLayer = (is3D) ? tex.computeVkArrayLayer(vol) : tex.computeVkArrayLayer(surf);
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = surf.m_level;
		region.imageOffset = {0, 0, 0};
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = depth;
		region.bufferOffset = 0;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		ANKI_CMD(vkCmdCopyBufferToImage(m_handle, shadowHandle, tex.m_imageHandle, layout, 1, &region),
				 ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_ASSERT(0);
	}

	m_microCmdb->pushObjectRef(texView);
	m_microCmdb->pushObjectRef(buff);
}

void CommandBufferImpl::rebindDynamicState()
{
	m_viewportDirty = true;
	m_lastViewport = {};
	m_scissorDirty = true;
	m_lastScissor = {};

	// Rebind the stencil compare mask
	if(m_stencilCompareMasks[0] == m_stencilCompareMasks[1])
	{
		ANKI_CMD(vkCmdSetStencilCompareMask(m_handle, VK_STENCIL_FACE_FRONT_BIT | VK_STENCIL_FACE_BACK_BIT,
											m_stencilCompareMasks[0]),
				 ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_CMD(vkCmdSetStencilCompareMask(m_handle, VK_STENCIL_FACE_FRONT_BIT, m_stencilCompareMasks[0]),
				 ANY_OTHER_COMMAND);
		ANKI_CMD(vkCmdSetStencilCompareMask(m_handle, VK_STENCIL_FACE_BACK_BIT, m_stencilCompareMasks[1]),
				 ANY_OTHER_COMMAND);
	}

	// Rebind the stencil write mask
	if(m_stencilWriteMasks[0] == m_stencilWriteMasks[1])
	{
		ANKI_CMD(vkCmdSetStencilWriteMask(m_handle, VK_STENCIL_FACE_FRONT_BIT | VK_STENCIL_FACE_BACK_BIT,
										  m_stencilWriteMasks[0]),
				 ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_CMD(vkCmdSetStencilWriteMask(m_handle, VK_STENCIL_FACE_FRONT_BIT, m_stencilWriteMasks[0]),
				 ANY_OTHER_COMMAND);
		ANKI_CMD(vkCmdSetStencilWriteMask(m_handle, VK_STENCIL_FACE_BACK_BIT, m_stencilWriteMasks[1]),
				 ANY_OTHER_COMMAND);
	}

	// Rebind the stencil reference
	if(m_stencilReferenceMasks[0] == m_stencilReferenceMasks[1])
	{
		ANKI_CMD(vkCmdSetStencilReference(m_handle, VK_STENCIL_FACE_FRONT_BIT | VK_STENCIL_FACE_BACK_BIT,
										  m_stencilReferenceMasks[0]),
				 ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_CMD(vkCmdSetStencilReference(m_handle, VK_STENCIL_FACE_FRONT_BIT, m_stencilReferenceMasks[0]),
				 ANY_OTHER_COMMAND);
		ANKI_CMD(vkCmdSetStencilReference(m_handle, VK_STENCIL_FACE_BACK_BIT, m_stencilReferenceMasks[1]),
				 ANY_OTHER_COMMAND);
	}
}

void CommandBufferImpl::buildAccelerationStructureInternal(AccelerationStructurePtr& as)
{
	commandCommon();

	// Get objects
	const AccelerationStructureImpl& asImpl = static_cast<AccelerationStructureImpl&>(*as);

	// Create the scrach buffer
	BufferInitInfo bufferInit;
	bufferInit.m_usage = InternalBufferUsageBit::ACCELERATION_STRUCTURE_BUILD_SCRATCH;
	bufferInit.m_size = asImpl.getBuildScratchBufferSize();
	BufferPtr scratchBuff = getManager().newBuffer(bufferInit);

	// Create the build info
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo;
	VkAccelerationStructureBuildOffsetInfoKHR offsetInfo;
	asImpl.generateBuildInfo(scratchBuff->getGpuAddress(), buildInfo, offsetInfo);

	// Do the command
	VkAccelerationStructureBuildOffsetInfoKHR* offsetInfoPtr = &offsetInfo;
	ANKI_CMD(vkCmdBuildAccelerationStructureKHR(m_handle, 1, &buildInfo, &offsetInfoPtr), ANY_OTHER_COMMAND);

	// Push refs
	m_microCmdb->pushObjectRef(as);
	m_microCmdb->pushObjectRef(scratchBuff);
}

} // end namespace anki

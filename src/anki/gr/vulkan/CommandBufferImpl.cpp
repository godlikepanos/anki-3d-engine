// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>

#include <algorithm>

namespace anki
{

CommandBufferImpl::CommandBufferImpl(GrManager* manager)
	: VulkanObject(manager)
{
}

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

	if(m_handle)
	{
		getGrManagerImpl().deleteCommandBuffer(m_handle, m_flags, m_tid);
	}

	m_fbList.destroy(m_alloc);
	m_texList.destroy(m_alloc);
	m_queryList.destroy(m_alloc);
	m_bufferList.destroy(m_alloc);
	m_cmdbList.destroy(m_alloc);
	m_progs.destroy(m_alloc);
	m_samplerList.destroy(m_alloc);

	m_imgBarriers.destroy(m_alloc);
	m_buffBarriers.destroy(m_alloc);
	m_queryResetAtoms.destroy(m_alloc);
	m_writeQueryAtoms.destroy(m_alloc);
	m_secondLevelAtoms.destroy(m_alloc);
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	auto& pool = getGrManagerImpl().getAllocator().getMemoryPool();
	m_alloc = StackAllocator<U8>(
		pool.getAllocationCallback(), pool.getAllocationCallbackUserData(), init.m_hints.m_chunkSize, 1.0, 0, false);

	m_texUsageTracker.init(m_alloc);

	m_flags = init.m_flags;
	m_tid = Thread::getCurrentThreadId();

	m_handle = getGrManagerImpl().newCommandBuffer(m_tid, m_flags);
	ANKI_ASSERT(m_handle);

	if(!!(m_flags & CommandBufferFlag::SECOND_LEVEL))
	{
		m_activeFb = init.m_framebuffer;
		m_state.beginRenderPass(m_activeFb);
	}

	return ErrorCode::NONE;
}

void CommandBufferImpl::beginRecording()
{
	VkCommandBufferInheritanceInfo inheritance = {};
	inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin.pInheritanceInfo = &inheritance;

	if(!!(m_flags & CommandBufferFlag::SECOND_LEVEL))
	{
		FramebufferImpl& impl = *m_activeFb->m_impl;

		// Calc the layouts
		Array<VkImageLayout, MAX_COLOR_ATTACHMENTS> colAttLayouts;
		for(U i = 0; i < impl.getColorAttachmentCount(); ++i)
		{
			colAttLayouts[i] = impl.getColorAttachment(i)->m_impl->findLayoutFromTracker(
				impl.getAttachedSurfaces()[i], m_texUsageTracker);
		}

		VkImageLayout dsAttLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
		if(impl.hasDepthStencil())
		{
			dsAttLayout = impl.getDepthStencilAttachment()->m_impl->findLayoutFromTracker(
				impl.getAttachedSurfaces()[MAX_COLOR_ATTACHMENTS], m_texUsageTracker);
		}

		inheritance.renderPass = impl.getRenderPassHandle(colAttLayouts, dsAttLayout);
		inheritance.subpass = 0;

		if(!impl.isDefaultFramebuffer())
		{
			inheritance.framebuffer = impl.getFramebufferHandle(0);
		}
		else
		{
			inheritance.framebuffer = impl.getFramebufferHandle(getGrManagerImpl().getCurrentBackbufferIndex());
		}

		begin.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}

	vkBeginCommandBuffer(m_handle, &begin);
}

void CommandBufferImpl::beginRenderPass(FramebufferPtr fb)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());

	m_rpCommandCount = 0;
	m_activeFb = fb;

	m_fbList.pushBack(m_alloc, fb);

	m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;
}

void CommandBufferImpl::beginRenderPassInternal()
{
	m_state.beginRenderPass(m_activeFb);

	FramebufferImpl& impl = *m_activeFb->m_impl;

	VkRenderPassBeginInfo bi = {};
	bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	bi.clearValueCount = impl.getAttachmentCount();
	bi.pClearValues = impl.getClearValues();

	if(!impl.isDefaultFramebuffer())
	{
		// Bind a non-default FB

		bi.framebuffer = impl.getFramebufferHandle(0);
		impl.getAttachmentsSize(bi.renderArea.extent.width, bi.renderArea.extent.height);

		// Calc the layouts
		Array<VkImageLayout, MAX_COLOR_ATTACHMENTS> colAttLayouts;
		for(U i = 0; i < impl.getColorAttachmentCount(); ++i)
		{
			colAttLayouts[i] = impl.getColorAttachment(i)->m_impl->findLayoutFromTracker(
				impl.getAttachedSurfaces()[i], m_texUsageTracker);
		}

		VkImageLayout dsAttLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
		if(impl.hasDepthStencil())
		{
			dsAttLayout = impl.getDepthStencilAttachment()->m_impl->findLayoutFromTracker(
				impl.getAttachedSurfaces()[MAX_COLOR_ATTACHMENTS], m_texUsageTracker);
		}

		bi.renderPass = impl.getRenderPassHandle(colAttLayouts, dsAttLayout);
	}
	else
	{
		// Bind the default FB
		m_renderedToDefaultFb = true;

		bi.framebuffer = impl.getFramebufferHandle(getGrManagerImpl().getCurrentBackbufferIndex());
		Array<VkImageLayout, MAX_COLOR_ATTACHMENTS> dummy;
		bi.renderPass = impl.getRenderPassHandle(dummy, VK_IMAGE_LAYOUT_MAX_ENUM);

		bi.renderArea.extent.width = getGrManagerImpl().getDefaultSurfaceWidth();
		bi.renderArea.extent.height = getGrManagerImpl().getDefaultSurfaceHeight();

		// Perform the transition
		setImageBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			getGrManagerImpl().getDefaultSurfaceImage(getGrManagerImpl().getCurrentBackbufferIndex()),
			VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
	}

	ANKI_CMD(vkCmdBeginRenderPass(m_handle, &bi, m_subpassContents), ANY_OTHER_COMMAND);
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

	// Default FB barrier/transition
	if(m_activeFb->m_impl->isDefaultFramebuffer())
	{
		setImageBarrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			getGrManagerImpl().getDefaultSurfaceImage(getGrManagerImpl().getCurrentBackbufferIndex()),
			VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
	}

	m_activeFb.reset(nullptr);
	m_state.endRenderPass();
}

void CommandBufferImpl::endRecording()
{
	commandCommon();

	ANKI_ASSERT(!m_finalized);
	ANKI_ASSERT(!m_empty);

	ANKI_CMD(ANKI_VK_CHECKF(vkEndCommandBuffer(m_handle)), ANY_OTHER_COMMAND);
	m_finalized = true;
}

void CommandBufferImpl::generateMipmaps2d(TexturePtr tex, U face, U layer)
{
	commandCommon();

	const TextureImpl& impl = *tex->m_impl;
	ANKI_ASSERT(impl.m_type != TextureType::_3D && "Not for 3D");

	for(U i = 0; i < impl.m_mipCount - 1u; ++i)
	{
		// Transition source
		if(i > 0)
		{
			VkImageSubresourceRange range;
			impl.computeSubResourceRange(TextureSurfaceInfo(i, 0, face, layer), impl.m_akAspect, range);

			setImageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				impl.m_imageHandle,
				range);
		}

		// Transition destination
		{
			VkImageSubresourceRange range;
			impl.computeSubResourceRange(TextureSurfaceInfo(i + 1, 0, face, layer), impl.m_akAspect, range);

			setImageBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				impl.m_imageHandle,
				range);
		}

		// Setup the blit struct
		I32 srcWidth = impl.m_width >> i;
		I32 srcHeight = impl.m_height >> i;

		I32 dstWidth = impl.m_width >> (i + 1);
		I32 dstHeight = impl.m_height >> (i + 1);

		ANKI_ASSERT(srcWidth > 0 && srcHeight > 0 && dstWidth > 0 && dstHeight > 0);

		U vkLayer = 0;
		switch(impl.m_type)
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
		blit.srcSubresource.aspectMask = impl.m_aspect;
		blit.srcSubresource.baseArrayLayer = vkLayer;
		blit.srcSubresource.layerCount = 1;
		blit.srcSubresource.mipLevel = i;
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {srcWidth, srcHeight, 1};

		blit.dstSubresource.aspectMask = impl.m_aspect;
		blit.dstSubresource.baseArrayLayer = vkLayer;
		blit.dstSubresource.layerCount = 1;
		blit.dstSubresource.mipLevel = i + 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {dstWidth, dstHeight, 1};

		ANKI_CMD(vkCmdBlitImage(m_handle,
					 impl.m_imageHandle,
					 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					 impl.m_imageHandle,
					 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					 1,
					 &blit,
					 (impl.m_depthStencil) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR),
			ANY_OTHER_COMMAND);
	}

	// Hold the reference
	m_texList.pushBack(m_alloc, tex);
}

void CommandBufferImpl::flushBarriers()
{
	if(m_imgBarrierCount == 0 && m_buffBarrierCount == 0)
	{
		return;
	}

	// Sort
	//

	if(m_imgBarrierCount > 0)
	{
		std::sort(&m_imgBarriers[0],
			&m_imgBarriers[0] + m_imgBarrierCount,
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
	U finalImgBarrierCount = 0;
	if(m_imgBarrierCount > 0)
	{
		DynamicArrayAuto<VkImageMemoryBarrier> squashedBarriers(m_alloc);
		U squashedBarrierCount = 0;

		squashedBarriers.create(m_imgBarrierCount);

		// Squash the mips by reducing the barriers
		for(U i = 0; i < m_imgBarrierCount; ++i)
		{
			const VkImageMemoryBarrier* prev = (i > 0) ? &m_imgBarriers[i - 1] : nullptr;
			const VkImageMemoryBarrier& crnt = m_imgBarriers[i];

			if(prev && prev->image == crnt.image
				&& prev->subresourceRange.aspectMask == crnt.subresourceRange.aspectMask
				&& prev->oldLayout == crnt.oldLayout
				&& prev->newLayout == crnt.newLayout
				&& prev->srcAccessMask == crnt.srcAccessMask
				&& prev->dstAccessMask == crnt.dstAccessMask
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

		for(U i = 0; i < squashedBarrierCount; ++i)
		{
			const VkImageMemoryBarrier* prev = (i > 0) ? &squashedBarriers[i - 1] : nullptr;
			const VkImageMemoryBarrier& crnt = squashedBarriers[i];

			if(prev && prev->image == crnt.image
				&& prev->subresourceRange.aspectMask == crnt.subresourceRange.aspectMask
				&& prev->oldLayout == crnt.oldLayout
				&& prev->newLayout == crnt.newLayout
				&& prev->srcAccessMask == crnt.srcAccessMask
				&& prev->dstAccessMask == crnt.dstAccessMask
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
	vkCmdPipelineBarrier(m_handle,
		m_srcStageMask,
		m_dstStageMask,
		0,
		0,
		nullptr,
		m_buffBarrierCount,
		(m_buffBarrierCount) ? &m_buffBarriers[0] : nullptr,
		finalImgBarrierCount,
		(finalImgBarrierCount) ? &finalImgBarriers[0] : nullptr);

	ANKI_TRACE_INC_COUNTER(VK_PIPELINE_BARRIERS, 1);

	m_imgBarrierCount = 0;
	m_buffBarrierCount = 0;
	m_srcStageMask = 0;
	m_dstStageMask = 0;
}

void CommandBufferImpl::flushQueryResets()
{
	if(m_queryResetAtomCount == 0)
	{
		return;
	}

	std::sort(&m_queryResetAtoms[0],
		&m_queryResetAtoms[0] + m_queryResetAtomCount,
		[](const QueryResetAtom& a, const QueryResetAtom& b) -> Bool {
			if(a.m_pool != b.m_pool)
			{
				return a.m_pool < b.m_pool;
			}

			ANKI_ASSERT(a.m_queryIdx != b.m_queryIdx && "Tried to reset the same query more than once");
			return a.m_queryIdx < b.m_queryIdx;
		});

	U firstQuery = m_queryResetAtoms[0].m_queryIdx;
	U queryCount = 1;
	VkQueryPool pool = m_queryResetAtoms[0].m_pool;
	for(U i = 1; i < m_queryResetAtomCount; ++i)
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

	m_queryResetAtomCount = 0;
}

void CommandBufferImpl::flushWriteQueryResults()
{
	if(m_writeQueryAtomCount == 0)
	{
		return;
	}

	std::sort(&m_writeQueryAtoms[0],
		&m_writeQueryAtoms[0] + m_writeQueryAtomCount,
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

	U firstQuery = m_writeQueryAtoms[0].m_queryIdx;
	U queryCount = 1;
	VkQueryPool pool = m_writeQueryAtoms[0].m_pool;
	PtrSize offset = m_writeQueryAtoms[0].m_offset;
	VkBuffer buff = m_writeQueryAtoms[0].m_buffer;
	for(U i = 1; i < m_writeQueryAtomCount; ++i)
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
			vkCmdCopyQueryPoolResults(
				m_handle, pool, firstQuery, queryCount, buff, offset, sizeof(U32), VK_QUERY_RESULT_PARTIAL_BIT);

			// New batch
			firstQuery = crnt.m_queryIdx;
			queryCount = 1;
			pool = crnt.m_pool;
			buff = crnt.m_buffer;
		}
	}

	vkCmdCopyQueryPoolResults(
		m_handle, pool, firstQuery, queryCount, buff, offset, sizeof(U32), VK_QUERY_RESULT_PARTIAL_BIT);

	m_writeQueryAtomCount = 0;
}

void CommandBufferImpl::copyBufferToTextureSurface(
	BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureSurfaceInfo& surf)
{
	commandCommon();

	TextureImpl& impl = *tex->m_impl;
	impl.checkSurfaceOrVolume(surf);
	ANKI_ASSERT(impl.usageValid(TextureUsageBit::TRANSFER_DESTINATION));
	const VkImageLayout layout = impl.findLayoutFromTracker(surf, m_texUsageTracker);

	if(!impl.m_workarounds)
	{
		U width = impl.m_width >> surf.m_level;
		U height = impl.m_height >> surf.m_level;
		ANKI_ASSERT(range == computeSurfaceSize(width, height, impl.m_format));

		// Copy
		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = impl.m_aspect;
		region.imageSubresource.baseArrayLayer = impl.computeVkArrayLayer(surf);
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = surf.m_level;
		region.imageOffset = {0, 0, I32(surf.m_depth)};
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = 1;
		region.bufferOffset = offset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		ANKI_CMD(vkCmdCopyBufferToImage(m_handle, buff->m_impl->getHandle(), impl.m_imageHandle, layout, 1, &region),
			ANY_OTHER_COMMAND);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		U width = impl.m_width >> surf.m_level;
		U height = impl.m_height >> surf.m_level;

		// Create a new shadow buffer
		const PtrSize shadowSize =
			computeSurfaceSize(width, height, PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM));
		BufferPtr shadow =
			getGrManager().newInstance<Buffer>(shadowSize, BufferUsageBit::TRANSFER_ALL, BufferMapAccessBit::NONE);
		const VkBuffer shadowHandle = shadow->m_impl->getHandle();
		m_bufferList.pushBack(m_alloc, shadow);

		// Create the copy regions
		DynamicArrayAuto<VkBufferCopy> copies(m_alloc);
		copies.create(width * height);
		U count = 0;
		for(U x = 0; x < width; ++x)
		{
			for(U y = 0; y < height; ++y)
			{
				VkBufferCopy& c = copies[count++];
				c.srcOffset = (y * width + x) * 3 + offset;
				c.dstOffset = (y * width + x) * 4 + 0;
				c.size = 3;
			}
		}

		// Copy buffer to buffer
		ANKI_CMD(vkCmdCopyBuffer(m_handle, buff->m_impl->getHandle(), shadowHandle, copies.getSize(), &copies[0]),
			ANY_OTHER_COMMAND);

		// Set barrier
		setBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			0,
			shadowSize,
			shadow->m_impl->getHandle());

		// Do the copy to the image
		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = impl.m_aspect;
		region.imageSubresource.baseArrayLayer = impl.computeVkArrayLayer(surf);
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = surf.m_level;
		region.imageOffset = {0, 0, I32(surf.m_depth)};
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = 1;
		region.bufferOffset = 0;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		ANKI_CMD(
			vkCmdCopyBufferToImage(m_handle, shadowHandle, impl.m_imageHandle, layout, 1, &region), ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_ASSERT(0);
	}

	m_texList.pushBack(m_alloc, tex);
	m_bufferList.pushBack(m_alloc, buff);
}

void CommandBufferImpl::copyBufferToTextureVolume(
	BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureVolumeInfo& vol)
{
	commandCommon();

	TextureImpl& impl = *tex->m_impl;
	impl.checkSurfaceOrVolume(vol);
	ANKI_ASSERT(impl.usageValid(TextureUsageBit::TRANSFER_DESTINATION));
	const VkImageLayout layout = impl.findLayoutFromTracker(vol, m_texUsageTracker);

	if(!impl.m_workarounds)
	{
		U width = impl.m_width >> vol.m_level;
		U height = impl.m_height >> vol.m_level;
		U depth = impl.m_depth >> vol.m_level;
		(void)depth;
		ANKI_ASSERT(range == computeVolumeSize(width, height, depth, impl.m_format));

		// Copy
		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = impl.m_aspect;
		region.imageSubresource.baseArrayLayer = impl.computeVkArrayLayer(vol);
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = vol.m_level;
		region.imageOffset = {0, 0, 0};
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = impl.m_depth;
		region.bufferOffset = offset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		ANKI_CMD(vkCmdCopyBufferToImage(m_handle, buff->m_impl->getHandle(), impl.m_imageHandle, layout, 1, &region),
			ANY_OTHER_COMMAND);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Find the offset to the RGBA staging buff
		U width = impl.m_width >> vol.m_level;
		U height = impl.m_height >> vol.m_level;
		U depth = impl.m_depth >> vol.m_level;

		// Create a new shadow buffer
		const PtrSize shadowSize =
			computeVolumeSize(width, height, depth, PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM));
		BufferPtr shadow =
			getGrManager().newInstance<Buffer>(shadowSize, BufferUsageBit::TRANSFER_ALL, BufferMapAccessBit::NONE);
		const VkBuffer shadowHandle = shadow->m_impl->getHandle();
		m_bufferList.pushBack(m_alloc, shadow);

		// Create the copy regions
		DynamicArrayAuto<VkBufferCopy> copies(m_alloc);
		copies.create(width * height * depth);
		U count = 0;
		for(U x = 0; x < width; ++x)
		{
			for(U y = 0; y < height; ++y)
			{
				for(U d = 0; d < depth; ++d)
				{
					VkBufferCopy& c = copies[count++];
					c.srcOffset = (d * height * width + y * width + x) * 3 + offset;
					c.dstOffset = (d * height * width + y * width + x) * 4 + 0;
					c.size = 3;
				}
			}
		}

		// Copy buffer to buffer
		ANKI_CMD(vkCmdCopyBuffer(m_handle, buff->m_impl->getHandle(), shadowHandle, copies.getSize(), &copies[0]),
			ANY_OTHER_COMMAND);

		// Set barrier
		setBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			0,
			shadowSize,
			shadowHandle);

		// Do the copy to the image
		VkBufferImageCopy region;
		region.imageSubresource.aspectMask = impl.m_aspect;
		region.imageSubresource.baseArrayLayer = impl.computeVkArrayLayer(vol);
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = vol.m_level;
		region.imageOffset = {0, 0, 0};
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = depth;
		region.bufferOffset = 0;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		ANKI_CMD(vkCmdCopyBufferToImage(
					 m_handle, shadowHandle, impl.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region),
			ANY_OTHER_COMMAND);
	}
	else
	{
		ANKI_ASSERT(0);
	}

	m_texList.pushBack(m_alloc, tex);
	m_bufferList.pushBack(m_alloc, buff);
}

} // end namespace anki

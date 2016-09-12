// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/Pipeline.h>
#include <anki/gr/vulkan/PipelineImpl.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/gr/ResourceGroup.h>
#include <anki/gr/vulkan/ResourceGroupImpl.h>

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
		ANKI_LOGW("Command buffer was empty");
	}

	if(!m_finalized)
	{
		ANKI_LOGW("Command buffer was not flushed");
	}

	if(m_handle)
	{
		getGrManagerImpl().deleteCommandBuffer(m_handle, m_flags, m_tid);
	}

	m_pplineList.destroy(m_alloc);
	m_fbList.destroy(m_alloc);
	m_rcList.destroy(m_alloc);
	m_texList.destroy(m_alloc);
	m_queryList.destroy(m_alloc);
	m_bufferList.destroy(m_alloc);
	m_cmdbList.destroy(m_alloc);

	m_imgBarriers.destroy(m_alloc);
	m_buffBarriers.destroy(m_alloc);
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	auto& pool = getGrManagerImpl().getAllocator().getMemoryPool();
	m_alloc = StackAllocator<U8>(
		pool.getAllocationCallback(), pool.getAllocationCallbackUserData(), init.m_hints.m_chunkSize, 1.0, 0, false);

	m_flags = init.m_flags;
	m_tid = Thread::getCurrentThreadId();

	m_handle = getGrManagerImpl().newCommandBuffer(m_tid, m_flags);
	ANKI_ASSERT(m_handle);

	// Begin recording
	VkCommandBufferInheritanceInfo inheritance = {};
	inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin.pInheritanceInfo = &inheritance;

	if(!!(m_flags & CommandBufferFlag::SECOND_LEVEL))
	{
		const FramebufferImpl& impl = init.m_framebuffer->getImplementation();

		inheritance.renderPass = impl.getRenderPassHandle();
		inheritance.subpass = 0;

		if(!impl.isDefaultFramebuffer())
		{
			inheritance.framebuffer = impl.getFramebufferHandle(0);
		}
		else
		{
			inheritance.framebuffer = impl.getFramebufferHandle(getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT);
		}

		begin.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	}

	vkBeginCommandBuffer(m_handle, &begin);

	return ErrorCode::NONE;
}

void CommandBufferImpl::bindPipeline(PipelinePtr ppline)
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);
	vkCmdBindPipeline(m_handle, ppline->getImplementation().getBindPoint(), ppline->getImplementation().getHandle());

	m_pplineList.pushBack(m_alloc, ppline);
}

void CommandBufferImpl::beginRenderPass(FramebufferPtr fb)
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);
	ANKI_ASSERT(!insideRenderPass());

	m_rpCommandCount = 0;
	m_activeFb = fb;

	m_fbList.pushBack(m_alloc, fb);

#if ANKI_ASSERTIONS
	m_subpassContents = VK_SUBPASS_CONTENTS_MAX_ENUM;
#endif
}

void CommandBufferImpl::beginRenderPassInternal()
{
	VkRenderPassBeginInfo bi = {};
	bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	FramebufferImpl& impl = m_activeFb->getImplementation();
	bi.renderPass = impl.getRenderPassHandle();
	bi.clearValueCount = impl.getAttachmentCount();
	bi.pClearValues = impl.getClearValues();

	if(!impl.isDefaultFramebuffer())
	{
		// Bind a non-default FB

		bi.framebuffer = impl.getFramebufferHandle(0);

		impl.getAttachmentsSize(bi.renderArea.extent.width, bi.renderArea.extent.height);
	}
	else
	{
		// Bind the default FB
		m_renderedToDefaultFb = true;

		bi.framebuffer = impl.getFramebufferHandle(getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT);

		bi.renderArea.extent.width = getGrManagerImpl().getDefaultSurfaceWidth();
		bi.renderArea.extent.height = getGrManagerImpl().getDefaultSurfaceHeight();

		// Perform the transition
		setImageBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			getGrManagerImpl().getDefaultSurfaceImage(getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT),
			VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
	}

	vkCmdBeginRenderPass(m_handle, &bi, m_subpassContents);
}

void CommandBufferImpl::endRenderPass()
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);
	ANKI_ASSERT(insideRenderPass());
	ANKI_ASSERT(m_rpCommandCount > 0);

	vkCmdEndRenderPass(m_handle);

	// Default FB barrier/transition
	if(m_activeFb->getImplementation().isDefaultFramebuffer())
	{
		setImageBarrier(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			getGrManagerImpl().getDefaultSurfaceImage(getGrManagerImpl().getFrame() % MAX_FRAMES_IN_FLIGHT),
			VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
	}

	m_activeFb.reset(nullptr);
}

void CommandBufferImpl::endRecordingInternal()
{
	flushBarriers();

	ANKI_ASSERT(!m_finalized);
	ANKI_ASSERT(!m_empty);

	ANKI_VK_CHECKF(vkEndCommandBuffer(m_handle));
	m_finalized = true;
}

void CommandBufferImpl::endRecording()
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);
	endRecordingInternal();
}

void CommandBufferImpl::bindResourceGroup(ResourceGroupPtr rc, U slot, const TransientMemoryInfo* dynInfo)
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);
	const ResourceGroupImpl& impl = rc->getImplementation();

	if(impl.hasDescriptorSet())
	{
		Array<U32, MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS> dynOffsets = {{}};

		impl.setupDynamicOffsets(dynInfo, &dynOffsets[0]);

		VkDescriptorSet dset = impl.getHandle();
		vkCmdBindDescriptorSets(m_handle,
			impl.getPipelineBindPoint(),
			getGrManagerImpl().getGlobalPipelineLayout(),
			slot,
			1,
			&dset,
			dynOffsets.getSize(),
			&dynOffsets[0]);
	}

	// Bind vertex and index buffer only in the first set
	if(slot == 0)
	{
		Array<VkBuffer, MAX_VERTEX_ATTRIBUTES> buffers = {{}};
		Array<VkDeviceSize, MAX_VERTEX_ATTRIBUTES> offsets = {{}};
		U bindingCount = 0;
		impl.getVertexBindingInfo(dynInfo, &buffers[0], &offsets[0], bindingCount);
		if(bindingCount)
		{
			vkCmdBindVertexBuffers(m_handle, 0, bindingCount, &buffers[0], &offsets[0]);
		}

		VkBuffer idxBuff;
		VkDeviceSize idxBuffOffset;
		VkIndexType idxType;
		if(impl.getIndexBufferInfo(idxBuff, idxBuffOffset, idxType))
		{
			vkCmdBindIndexBuffer(m_handle, idxBuff, idxBuffOffset, idxType);
		}
	}

	// Hold the reference
	m_rcList.pushBack(m_alloc, rc);
}

void CommandBufferImpl::generateMipmaps2d(TexturePtr tex, U face, U layer)
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);

	const TextureImpl& impl = tex->getImplementation();
	ANKI_ASSERT(impl.m_type != TextureType::_3D && "Not for 3D");

	for(U i = 0; i < impl.m_mipCount - 1u; ++i)
	{
		// Transition source
		if(i > 0)
		{
			VkImageSubresourceRange range;
			impl.computeSubResourceRange(TextureSurfaceInfo(i, 0, face, layer), range);

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
			impl.computeSubResourceRange(TextureSurfaceInfo(i + 1, 0, face, layer), range);

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
		blit.srcSubresource.aspectMask = impl.m_aspect & (~VK_IMAGE_ASPECT_STENCIL_BIT);
		blit.srcSubresource.baseArrayLayer = vkLayer;
		blit.srcSubresource.layerCount = 1;
		blit.srcSubresource.mipLevel = i;
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {srcWidth, srcHeight, 1};

		blit.dstSubresource.aspectMask = impl.m_aspect & (~VK_IMAGE_ASPECT_STENCIL_BIT);
		blit.dstSubresource.baseArrayLayer = vkLayer;
		blit.dstSubresource.layerCount = 1;
		blit.dstSubresource.mipLevel = i + 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {dstWidth, dstHeight, 1};

		flushBarriers();

		vkCmdBlitImage(m_handle,
			impl.m_imageHandle,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			impl.m_imageHandle,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&blit,
			(impl.m_depthStencil) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
	}

	// Hold the reference
	m_texList.pushBack(m_alloc, tex);
}

void CommandBufferImpl::uploadTextureSurface(
	TexturePtr tex, const TextureSurfaceInfo& surf, const TransientMemoryToken& token)
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);

	TextureImpl& impl = tex->getImplementation();
	impl.checkSurface(surf);
	ANKI_ASSERT(impl.usageValid(TextureUsageBit::UPLOAD));

	if(!impl.m_workarounds)
	{
		U width = impl.m_width >> surf.m_level;
		U height = impl.m_height >> surf.m_level;
		ANKI_ASSERT(token.m_range == computeSurfaceSize(width, height, impl.m_format));

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
		region.bufferOffset = token.m_offset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		vkCmdCopyBufferToImage(m_handle,
			getGrManagerImpl().getTransientMemoryManager().getBufferHandle(token.m_usage),
			impl.m_imageHandle,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Find the offset to the RGBA staging buff
		U width = impl.m_width >> surf.m_level;
		U height = impl.m_height >> surf.m_level;
		PtrSize dstOffset =
			computeSurfaceSize(width, height, PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM));
		alignRoundUp(16, dstOffset);
		ANKI_ASSERT(token.m_range
			== dstOffset
				+ computeSurfaceSize(width, height, PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM)));
		dstOffset += token.m_offset;

		// Create the copy regions
		DynamicArrayAuto<VkBufferCopy> copies(m_alloc);
		copies.create(width * height);
		U count = 0;
		for(U x = 0; x < width; ++x)
		{
			for(U y = 0; y < height; ++y)
			{
				VkBufferCopy& c = copies[count++];
				c.srcOffset = (y * width + x) * 3 + token.m_offset;
				c.dstOffset = (y * width + x) * 4 + dstOffset;
				c.size = 3;
			}
		}

		// Copy buffer to buffer
		VkBuffer buffHandle = getGrManagerImpl().getTransientMemoryManager().getBufferHandle(token.m_usage);
		vkCmdCopyBuffer(m_handle, buffHandle, buffHandle, copies.getSize(), &copies[0]);

		// Set barrier
		setBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			dstOffset,
			token.m_range - (dstOffset - token.m_offset),
			buffHandle);
		flushBarriers();

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
		region.bufferOffset = dstOffset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		vkCmdCopyBufferToImage(
			m_handle, buffHandle, impl.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	else
	{
		ANKI_ASSERT(0);
	}

	m_texList.pushBack(m_alloc, tex);
}

void CommandBufferImpl::uploadTextureVolume(
	TexturePtr tex, const TextureVolumeInfo& vol, const TransientMemoryToken& token)
{
	commandCommon(CommandBufferCommandType::ANY_OTHER_COMMAND);

	TextureImpl& impl = tex->getImplementation();
	impl.checkVolume(vol);
	ANKI_ASSERT(impl.usageValid(TextureUsageBit::UPLOAD));

	if(!impl.m_workarounds)
	{
		U width = impl.m_width >> vol.m_level;
		U height = impl.m_height >> vol.m_level;
		U depth = impl.m_depth >> vol.m_level;
		(void)depth;
		ANKI_ASSERT(token.m_range == computeVolumeSize(width, height, depth, impl.m_format));

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
		region.bufferOffset = token.m_offset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		vkCmdCopyBufferToImage(m_handle,
			getGrManagerImpl().getTransientMemoryManager().getBufferHandle(token.m_usage),
			impl.m_imageHandle,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);
	}
	else if(!!(impl.m_workarounds & TextureImplWorkaround::R8G8B8_TO_R8G8B8A8))
	{
		// Find the offset to the RGBA staging buff
		U width = impl.m_width >> vol.m_level;
		U height = impl.m_height >> vol.m_level;
		U depth = impl.m_depth >> vol.m_level;
		PtrSize dstOffset =
			computeVolumeSize(width, height, depth, PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM));
		alignRoundUp(16, dstOffset);
		ANKI_ASSERT(token.m_range
			== dstOffset + computeVolumeSize(
							   width, height, depth, PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM)));
		dstOffset += token.m_offset;

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
					c.srcOffset = (d * height * width + y * width + x) * 3 + token.m_offset;
					c.dstOffset = (d * height * width + y * width + x) * 4 + dstOffset;
					c.size = 3;
				}
			}
		}

		// Copy buffer to buffer
		VkBuffer buffHandle = getGrManagerImpl().getTransientMemoryManager().getBufferHandle(token.m_usage);
		vkCmdCopyBuffer(m_handle, buffHandle, buffHandle, copies.getSize(), &copies[0]);

		// Set barrier
		setBufferBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_ACCESS_TRANSFER_READ_BIT,
			dstOffset,
			token.m_range - (dstOffset - token.m_offset),
			buffHandle);
		flushBarriers();

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
		region.bufferOffset = dstOffset;
		region.bufferImageHeight = 0;
		region.bufferRowLength = 0;

		vkCmdCopyBufferToImage(
			m_handle, buffHandle, impl.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}
	else
	{
		ANKI_ASSERT(0);
	}

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
				squashedBarriers[squashedBarrierCount].subresourceRange.levelCount += crnt.subresourceRange.levelCount;
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
				finalImgBarriers[finalImgBarrierCount].subresourceRange.layerCount += crnt.subresourceRange.layerCount;
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

	ANKI_TRACE_INC_COUNTER(GR_PIPELINE_BARRIERS, 1);

	m_imgBarrierCount = 0;
	m_buffBarrierCount = 0;
}

} // end namespace anki

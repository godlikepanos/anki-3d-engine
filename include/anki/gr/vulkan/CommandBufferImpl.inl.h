// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

//==============================================================================
inline void CommandBufferImpl::setViewport(
	U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	commandCommon();
	ANKI_ASSERT(minx < maxx && miny < maxy);
	VkViewport s;
	s.x = minx;
	s.y = miny;
	s.width = maxx - minx;
	s.height = maxy - miny;
	vkCmdSetViewport(m_handle, 0, 1, &s);

	VkRect2D scissor = {};
	scissor.extent.width = maxx - minx;
	scissor.extent.height = maxy - miny;
	scissor.offset.x = minx;
	scissor.offset.y = miny;
	vkCmdSetScissor(m_handle, 0, 1, &scissor);
}

//==============================================================================
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

	vkCmdPipelineBarrier(
		m_handle, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &inf);
}

//==============================================================================
inline void CommandBufferImpl::setImageBarrier(VkPipelineStageFlags srcStage,
	VkAccessFlags srcAccess,
	VkImageLayout prevLayout,
	VkPipelineStageFlags dstStage,
	VkAccessFlags dstAccess,
	VkImageLayout newLayout,
	TexturePtr tex,
	const VkImageSubresourceRange& range)
{
	setImageBarrier(srcStage,
		srcAccess,
		prevLayout,
		dstStage,
		dstAccess,
		newLayout,
		tex->getImplementation().m_imageHandle,
		range);

	m_texList.pushBack(m_alloc, tex);
}

//==============================================================================
inline void CommandBufferImpl::setImageBarrier(TexturePtr tex,
	TextureUsageBit prevUsage,
	TextureUsageBit nextUsage,
	const TextureSurfaceInfo& surf)
{
	const TextureImpl& impl = tex->getImplementation();
	tex->getImplementation().checkSurface(surf);
	Bool isDepthStencil = formatIsDepthStencil(impl.m_format);

	VkPipelineStageFlags srcStage;
	VkAccessFlags srcAccess;
	VkImageLayout oldLayout;
	VkPipelineStageFlags dstStage;
	VkAccessFlags dstAccess;
	VkImageLayout newLayout;
	computeBarrierInfo(prevUsage,
		nextUsage,
		isDepthStencil,
		surf.m_level,
		srcStage,
		srcAccess,
		dstStage,
		dstAccess);
	oldLayout = computeLayout(prevUsage, isDepthStencil, surf.m_level);
	newLayout = computeLayout(nextUsage, isDepthStencil, surf.m_level);

	VkImageSubresourceRange range;
	impl.computeSubResourceRange(surf, range);

	setImageBarrier(srcStage,
		srcAccess,
		oldLayout,
		dstStage,
		dstAccess,
		newLayout,
		tex,
		range);
}

//==============================================================================
inline void CommandBufferImpl::uploadTextureSurface(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const TransientMemoryToken& token)
{
	commandCommon();
	TextureImpl& impl = tex->getImplementation();
	impl.checkSurface(surf);

	VkImageSubresourceRange range;
	impl.computeSubResourceRange(surf, range);

	U width = impl.m_width >> surf.m_level;
	U height = impl.m_height >> surf.m_level;

	// Copy
	VkBufferImageCopy region;
	region.imageSubresource.aspectMask = impl.m_aspect;
	region.imageSubresource.baseArrayLayer = range.baseArrayLayer;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = surf.m_level;
	region.imageOffset = {0, 0, 0};
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;
	region.bufferOffset = token.m_offset;
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;

	vkCmdCopyBufferToImage(m_handle,
		getGrManagerImpl().getTransientMemoryManager().getBufferHandle(
			token.m_usage),
		impl.m_imageHandle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);

	m_texList.pushBack(m_alloc, tex);
}

} // end namespace anki

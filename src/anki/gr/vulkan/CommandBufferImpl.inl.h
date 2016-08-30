// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/vulkan/OcclusionQueryImpl.h>

namespace anki
{

inline void CommandBufferImpl::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	commandCommon();
	ANKI_ASSERT(minx < maxx && miny < maxy);
	VkViewport s;
	s.x = minx;
	s.y = miny;
	s.width = maxx - minx;
	s.height = maxy - miny;
	s.minDepth = 0.0;
	s.maxDepth = 1.0;
	vkCmdSetViewport(m_handle, 0, 1, &s);

	VkRect2D scissor = {};
	scissor.extent.width = maxx - minx;
	scissor.extent.height = maxy - miny;
	scissor.offset.x = minx;
	scissor.offset.y = miny;
	vkCmdSetScissor(m_handle, 0, 1, &scissor);
}

inline void CommandBufferImpl::setPolygonOffset(F32 factor, F32 units)
{
	commandCommon();
	vkCmdSetDepthBias(m_handle, units, 0.0, factor);
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

	vkCmdPipelineBarrier(m_handle, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &inf);
}

inline void CommandBufferImpl::setImageBarrier(VkPipelineStageFlags srcStage,
	VkAccessFlags srcAccess,
	VkImageLayout prevLayout,
	VkPipelineStageFlags dstStage,
	VkAccessFlags dstAccess,
	VkImageLayout newLayout,
	TexturePtr tex,
	const VkImageSubresourceRange& range)
{
	setImageBarrier(
		srcStage, srcAccess, prevLayout, dstStage, dstAccess, newLayout, tex->getImplementation().m_imageHandle, range);

	m_texList.pushBack(m_alloc, tex);
}

inline void CommandBufferImpl::setTextureBarrierInternal(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const VkImageSubresourceRange& range)
{
	const TextureImpl& impl = tex->getImplementation();
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

	setImageBarrier(srcStage, srcAccess, oldLayout, dstStage, dstAccess, newLayout, tex, range);
}

inline void CommandBufferImpl::setTextureSurfaceBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf)
{
	if(surf.m_level > 0)
	{
		ANKI_ASSERT(!(nextUsage & TextureUsageBit::GENERATE_MIPMAPS)
			&& "This transition happens inside CommandBufferImpl::generateMipmapsX");
	}

	const TextureImpl& impl = tex->getImplementation();
	impl.checkSurface(surf);

	VkImageSubresourceRange range;
	impl.computeSubResourceRange(surf, range);
	setTextureBarrierInternal(tex, prevUsage, nextUsage, range);
}

inline void CommandBufferImpl::setTextureVolumeBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol)
{
	if(vol.m_level > 0)
	{
		ANKI_ASSERT(!(nextUsage & TextureUsageBit::GENERATE_MIPMAPS)
			&& "This transition happens inside CommandBufferImpl::generateMipmaps");
	}

	const TextureImpl& impl = tex->getImplementation();
	impl.checkVolume(vol);

	VkImageSubresourceRange range;
	impl.computeSubResourceRange(vol, range);
	setTextureBarrierInternal(tex, prevUsage, nextUsage, range);
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

	vkCmdPipelineBarrier(m_handle, srcStage, dstStage, 0, 0, nullptr, 1, &b, 0, nullptr);
}

inline void CommandBufferImpl::setBufferBarrier(
	BufferPtr buff, BufferUsageBit before, BufferUsageBit after, PtrSize offset, PtrSize size)
{
	const BufferImpl& impl = buff->getImplementation();

	VkPipelineStageFlags srcStage;
	VkAccessFlags srcAccess;
	VkPipelineStageFlags dstStage;
	VkAccessFlags dstAccess;
	impl.computeBarrierInfo(before, after, srcStage, srcAccess, dstStage, dstAccess);

	setBufferBarrier(srcStage, srcAccess, dstStage, dstAccess, offset, size, impl.getHandle());

	m_bufferList.pushBack(m_alloc, buff);
}

inline void CommandBufferImpl::drawElements(
	U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	drawcallCommon();
	vkCmdDrawIndexed(m_handle, count, instanceCount, firstIndex, baseVertex, baseInstance);
}

inline void CommandBufferImpl::resetOcclusionQuery(OcclusionQueryPtr query)
{
	commandCommon();
	flushBarriers();

	VkQueryPool handle = query->getImplementation().m_handle.m_pool;
	U32 idx = query->getImplementation().m_handle.m_queryIndex;
	ANKI_ASSERT(handle);

	vkCmdResetQueryPool(m_handle, handle, idx, 0);

	m_queryList.pushBack(m_alloc, query);
}

inline void CommandBufferImpl::beginOcclusionQuery(OcclusionQueryPtr query)
{
	commandCommon();
	flushBarriers();

	VkQueryPool handle = query->getImplementation().m_handle.m_pool;
	U32 idx = query->getImplementation().m_handle.m_queryIndex;
	ANKI_ASSERT(handle);

	vkCmdBeginQuery(m_handle, handle, idx, 0);

	m_queryList.pushBack(m_alloc, query);
}

inline void CommandBufferImpl::endOcclusionQuery(OcclusionQueryPtr query)
{
	commandCommon();

	VkQueryPool handle = query->getImplementation().m_handle.m_pool;
	U32 idx = query->getImplementation().m_handle.m_queryIndex;
	ANKI_ASSERT(handle);

	vkCmdEndQuery(m_handle, handle, idx);

	m_queryList.pushBack(m_alloc, query);
}

inline void CommandBufferImpl::clearTextureInternal(
	TexturePtr tex, const ClearValue& clearValue, const VkImageSubresourceRange& range)
{
	commandCommon();
	flushBarriers();

	VkClearColorValue vclear;
	static_assert(sizeof(vclear) == sizeof(clearValue), "See file");
	memcpy(&vclear, &clearValue, sizeof(clearValue));

	const TextureImpl& impl = tex->getImplementation();
	if(impl.m_aspect == VK_IMAGE_ASPECT_COLOR_BIT)
	{
		vkCmdClearColorImage(m_handle, impl.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vclear, 1, &range);
	}
	else
	{
		ANKI_ASSERT(0 && "TODO");
	}

	m_texList.pushBack(m_alloc, tex);
}

inline void CommandBufferImpl::clearTexture(TexturePtr tex, const ClearValue& clearValue)
{
	VkImageSubresourceRange range;
	range.aspectMask = tex->getImplementation().m_aspect;
	range.baseMipLevel = 0;
	range.levelCount = VK_REMAINING_MIP_LEVELS;
	range.baseArrayLayer = 0;
	range.layerCount = VK_REMAINING_ARRAY_LAYERS;

	clearTextureInternal(tex, clearValue, range);
}

inline void CommandBufferImpl::clearTextureSurface(
	TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& clearValue)
{
	const TextureImpl& impl = tex->getImplementation();
	ANKI_ASSERT(impl.m_type != TextureType::_3D && "Not for 3D");

	VkImageSubresourceRange range;
	impl.computeSubResourceRange(surf, range);
	clearTextureInternal(tex, clearValue, range);
}

inline void CommandBufferImpl::clearTextureVolume(
	TexturePtr tex, const TextureVolumeInfo& vol, const ClearValue& clearValue)
{
	const TextureImpl& impl = tex->getImplementation();
	ANKI_ASSERT(impl.m_type == TextureType::_3D && "Only for 3D");

	VkImageSubresourceRange range;
	impl.computeSubResourceRange(vol, range);
	clearTextureInternal(tex, clearValue, range);
}

inline void CommandBufferImpl::uploadBuffer(BufferPtr buff, PtrSize offset, const TransientMemoryToken& token)
{
	commandCommon();
	flushBarriers();
	BufferImpl& impl = buff->getImplementation();

	VkBufferCopy region;
	region.srcOffset = token.m_offset;
	region.dstOffset = offset;
	region.size = token.m_range;

	ANKI_ASSERT(offset + token.m_range <= impl.getSize());

	vkCmdCopyBuffer(m_handle,
		getGrManagerImpl().getTransientMemoryManager().getBufferHandle(token.m_usage),
		impl.getHandle(),
		1,
		&region);

	m_bufferList.pushBack(m_alloc, buff);
}

inline void CommandBufferImpl::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	commandCommon();
	ANKI_ASSERT(insideRenderPass());
	ANKI_ASSERT(m_subpassContents == VK_SUBPASS_CONTENTS_MAX_ENUM
		|| m_subpassContents == VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
#if ANKI_ASSERTIONS
	m_subpassContents = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
#endif

	if(ANKI_UNLIKELY(m_rpCommandCount == 0))
	{
		beginRenderPassInternal();
	}

	cmdb->getImplementation().endRecordingInternal();

	vkCmdExecuteCommands(m_handle, 1, &cmdb->getImplementation().m_handle);

	++m_rpCommandCount;
	m_cmdbList.pushBack(m_alloc, cmdb);
}

inline void CommandBufferImpl::drawcallCommon()
{
	// Preconditions
	commandCommon();
	ANKI_ASSERT(insideRenderPass() || secondLevel());
	ANKI_ASSERT(m_subpassContents == VK_SUBPASS_CONTENTS_MAX_ENUM || m_subpassContents == VK_SUBPASS_CONTENTS_INLINE);
#if ANKI_ASSERTIONS
	m_subpassContents = VK_SUBPASS_CONTENTS_INLINE;
#endif

	if(ANKI_UNLIKELY(m_rpCommandCount == 0) && !secondLevel())
	{
		beginRenderPassInternal();
	}

	++m_rpCommandCount;
}

inline void CommandBufferImpl::commandCommon()
{
	ANKI_ASSERT(Thread::getCurrentThreadId() == m_tid
		&& "Commands must be recorder and flushed by the thread this command buffer was created");

	ANKI_ASSERT(!m_finalized);
	ANKI_ASSERT(m_handle);
	m_empty = false;
}

inline void CommandBufferImpl::fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
{
	commandCommon();
	ANKI_ASSERT(!insideRenderPass());
	BufferImpl& impl = buff->getImplementation();
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::FILL));

	ANKI_ASSERT(offset < impl.getSize());
	ANKI_ASSERT((offset % 4) == 0 && "Should be multiple of 4");

	size = (size == MAX_PTR_SIZE) ? (impl.getSize() - offset) : size;
	ANKI_ASSERT(offset + size <= impl.getSize());
	ANKI_ASSERT((size % 4) == 0 && "Should be multiple of 4");

	vkCmdFillBuffer(m_handle, impl.getHandle(), offset, size, value);
}

} // end namespace anki

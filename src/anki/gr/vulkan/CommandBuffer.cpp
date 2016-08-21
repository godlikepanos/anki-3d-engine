// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/Pipeline.h>
#include <anki/gr/ResourceGroup.h>

namespace anki
{

//==============================================================================
CommandBuffer::CommandBuffer(GrManager* manager, U64 hash)
	: GrObject(manager, CLASS_TYPE, hash)
{
}

//==============================================================================
CommandBuffer::~CommandBuffer()
{
}

//==============================================================================
void CommandBuffer::init(CommandBufferInitInfo& inf)
{
	m_impl.reset(getAllocator().newInstance<CommandBufferImpl>(&getManager()));

	if(m_impl->init(inf))
	{
		ANKI_LOGF("Cannot recover");
	}
}

//==============================================================================
CommandBufferInitHints CommandBuffer::computeInitHints() const
{
	// TODO
	CommandBufferInitHints hints;
	return hints;
}

//==============================================================================
void CommandBuffer::flush()
{
	m_impl->endRecording();
	m_impl->getGrManagerImpl().flushCommandBuffer(CommandBufferPtr(this),
		SemaphorePtr(),
		WeakArray<SemaphorePtr>(),
		WeakArray<VkPipelineStageFlags>());
}

//==============================================================================
void CommandBuffer::finish()
{
}

//==============================================================================
void CommandBuffer::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	m_impl->setViewport(minx, miny, maxx, maxy);
}

//==============================================================================
void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	m_impl->setPolygonOffset(factor, units);
}

//==============================================================================
void CommandBuffer::bindPipeline(PipelinePtr ppline)
{
	m_impl->bindPipeline(ppline);
}

//==============================================================================
void CommandBuffer::beginRenderPass(FramebufferPtr fb)
{
	m_impl->beginRenderPass(fb);
}

//==============================================================================
void CommandBuffer::endRenderPass()
{
	m_impl->endRenderPass();
}

//==============================================================================
void CommandBuffer::bindResourceGroup(
	ResourceGroupPtr rc, U slot, const TransientMemoryInfo* dynInfo)
{
	m_impl->bindResourceGroup(rc, slot, dynInfo);
}

//==============================================================================
void CommandBuffer::drawElements(U32 count,
	U32 instanceCount,
	U32 firstIndex,
	U32 baseVertex,
	U32 baseInstance)
{
	m_impl->drawElements(
		count, instanceCount, firstIndex, baseVertex, baseInstance);
}

//==============================================================================
void CommandBuffer::drawArrays(
	U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	m_impl->drawArrays(count, instanceCount, first, baseInstance);
}

//==============================================================================
void CommandBuffer::drawElementsConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 firstIndex,
	U32 baseVertex,
	U32 baseInstance)
{
	ANKI_ASSERT(0);
}

//==============================================================================
void CommandBuffer::drawArraysConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 first,
	U32 baseInstance)
{
	ANKI_ASSERT(0);
}

//==============================================================================
void CommandBuffer::dispatchCompute(
	U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	m_impl->dispatchCompute(groupCountX, groupCountY, groupCountZ);
}

//==============================================================================
void CommandBuffer::generateMipmaps2d(TexturePtr tex, U face, U layer)
{
	m_impl->generateMipmaps2d(tex, face, layer);
}

//==============================================================================
void CommandBuffer::generateMipmaps3d(TexturePtr tex)
{
	ANKI_ASSERT(0 && "TODO");
}

//==============================================================================
void CommandBuffer::copyTextureSurfaceToTextureSurface(TexturePtr src,
	const TextureSurfaceInfo& srcSurf,
	TexturePtr dest,
	const TextureSurfaceInfo& destSurf)
{
	ANKI_ASSERT(0 && "TODO");
}

//==============================================================================
void CommandBuffer::copyTextureVolumeToTextureVolume(TexturePtr src,
	const TextureVolumeInfo& srcVol,
	TexturePtr dest,
	const TextureVolumeInfo& destVol)
{
	ANKI_ASSERT(0 && "TODO");
}

//==============================================================================
void CommandBuffer::clearTexture(TexturePtr tex, const ClearValue& clearValue)
{
	m_impl->clearTexture(tex, clearValue);
}

//==============================================================================
void CommandBuffer::clearTextureSurface(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const ClearValue& clearValue)
{
	m_impl->clearTextureSurface(tex, surf, clearValue);
}

//==============================================================================
void CommandBuffer::clearTextureVolume(
	TexturePtr tex, const TextureVolumeInfo& vol, const ClearValue& clearValue)
{
	m_impl->clearTextureVolume(tex, vol, clearValue);
}

//==============================================================================
void CommandBuffer::uploadTextureSurface(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const TransientMemoryToken& token)
{
	m_impl->uploadTextureSurface(tex, surf, token);
}

//==============================================================================
void CommandBuffer::uploadTextureVolume(TexturePtr tex,
	const TextureVolumeInfo& vol,
	const TransientMemoryToken& token)
{
	m_impl->uploadTextureVolume(tex, vol, token);
}

//==============================================================================
void CommandBuffer::uploadBuffer(
	BufferPtr buff, PtrSize offset, const TransientMemoryToken& token)
{
	m_impl->uploadBuffer(buff, offset, token);
}

//==============================================================================
void CommandBuffer::setTextureSurfaceBarrier(TexturePtr tex,
	TextureUsageBit prevUsage,
	TextureUsageBit nextUsage,
	const TextureSurfaceInfo& surf)
{
	m_impl->setTextureSurfaceBarrier(tex, prevUsage, nextUsage, surf);
}

//==============================================================================
void CommandBuffer::setTextureVolumeBarrier(TexturePtr tex,
	TextureUsageBit prevUsage,
	TextureUsageBit nextUsage,
	const TextureVolumeInfo& vol)
{
	m_impl->setTextureVolumeBarrier(tex, prevUsage, nextUsage, vol);
}

//==============================================================================
void CommandBuffer::setBufferBarrier(
	BufferPtr buff, BufferUsageBit before, BufferUsageBit after)
{
}

//==============================================================================
void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->beginOcclusionQuery(query);
}

//==============================================================================
void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->endOcclusionQuery(query);
}

//==============================================================================
void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	m_impl->pushSecondLevelCommandBuffer(cmdb);
}

//==============================================================================
Bool CommandBuffer::isEmpty() const
{
	return m_impl->isEmpty();
}

} // end namespace anki

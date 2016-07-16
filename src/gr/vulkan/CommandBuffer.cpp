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
void CommandBuffer::setPolygonOffset(F32 offset, F32 units)
{
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
}

//==============================================================================
void CommandBuffer::drawArraysConditional(OcclusionQueryPtr query,
	U32 count,
	U32 instanceCount,
	U32 first,
	U32 baseInstance)
{
}

//==============================================================================
void CommandBuffer::dispatchCompute(
	U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
}

//==============================================================================
void CommandBuffer::generateMipmaps(TexturePtr tex, U depth, U face, U layer)
{
}

//==============================================================================
void CommandBuffer::copyTextureToTexture(TexturePtr src,
	const TextureSurfaceInfo& srcSurf,
	TexturePtr dest,
	const TextureSurfaceInfo& destSurf)
{
}

//==============================================================================
void CommandBuffer::clearTexture(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const ClearValue& clearValue)
{
}

//==============================================================================
void CommandBuffer::uploadTextureSurface(TexturePtr tex,
	const TextureSurfaceInfo& surf,
	const TransientMemoryToken& token)
{
	m_impl->uploadTextureSurface(tex, surf, token);
}

//==============================================================================
void CommandBuffer::uploadBuffer(
	BufferPtr buff, PtrSize offset, const TransientMemoryToken& token)
{
}

//==============================================================================
void CommandBuffer::setTextureBarrier(TexturePtr tex,
	TextureUsageBit prevUsage,
	TextureUsageBit nextUsage,
	const TextureSurfaceInfo& surf)
{
	m_impl->setImageBarrier(tex, prevUsage, nextUsage, surf);
}

//==============================================================================
void CommandBuffer::setPipelineBarrier(
	PipelineStageBit src, PipelineStageBit dst)
{
}

//==============================================================================
void CommandBuffer::setBufferMemoryBarrier(
	BufferPtr buff, ResourceAccessBit src, ResourceAccessBit dst)
{
}

//==============================================================================
void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
}

//==============================================================================
void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
}

//==============================================================================
void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
}

//==============================================================================
Bool CommandBuffer::isEmpty() const
{
	// TODO
	return false;
}

} // end namespace anki

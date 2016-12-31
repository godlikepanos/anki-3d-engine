// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

CommandBuffer::CommandBuffer(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::init(CommandBufferInitInfo& inf)
{
	m_impl.reset(getAllocator().newInstance<CommandBufferImpl>(&getManager()));

	if(m_impl->init(inf))
	{
		ANKI_LOGF("Cannot recover");
	}
}

CommandBufferInitHints CommandBuffer::computeInitHints() const
{
	// TODO
	CommandBufferInitHints hints;
	return hints;
}

void CommandBuffer::flush()
{
	m_impl->endRecording();

	if(!m_impl->isSecondLevel())
	{
		m_impl->getGrManagerImpl().flushCommandBuffer(CommandBufferPtr(this));
	}
}

void CommandBuffer::finish()
{
	m_impl->endRecording();

	if(!m_impl->isSecondLevel())
	{
		m_impl->getGrManagerImpl().finishCommandBuffer(CommandBufferPtr(this));
	}
}

void CommandBuffer::bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setPrimitiveRestart(Bool enable)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setViewport(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	m_impl->setViewport(minx, miny, maxx, maxy);
}

void CommandBuffer::setScissorTest(Bool enable)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setScissorRect(U16 minx, U16 miny, U16 maxx, U16 maxy)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setFillMode(FillMode mode)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setCullMode(FaceSelectionBit mode)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	m_impl->setPolygonOffset(factor, units);
}

void CommandBuffer::setStencilOperations(FaceSelectionBit face,
	StencilOperation stencilFail,
	StencilOperation stencilPassDepthFail,
	StencilOperation stencilPassDepthPass)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setStencilCompareMask(FaceSelectionBit face, U32 mask)
{
	m_impl->setStencilCompareMask(face, mask);
}

void CommandBuffer::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	m_impl->setStencilWriteMask(face, mask);
}

void CommandBuffer::setStencilReference(FaceSelectionBit face, U32 ref)
{
	m_impl->setStencilReference(face, ref);
}

void CommandBuffer::setDepthWrite(Bool enable)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setDepthCompareOperation(CompareOperation op)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setAlphaToCoverage(Bool enable)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setColorChannelWriteMask(U32 attachment, ColorBit mask)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setBlendFactors(
	U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindTexture(U32 set, U32 binding, TexturePtr tex, DepthStencilAspectBit aspect)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindTextureAndSampler(
	U32 set, U32 binding, TexturePtr tex, SamplerPtr sampler, DepthStencilAspectBit aspect)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindImage(U32 set, U32 binding, TexturePtr img, U32 level)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindShaderProgram(ShaderProgramPtr prog)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::beginRenderPass(FramebufferPtr fb)
{
	m_impl->beginRenderPass(fb);
}

void CommandBuffer::endRenderPass()
{
	m_impl->endRenderPass();
}

void CommandBuffer::drawElements(
	PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawArrays(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawArraysIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr buff)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawElementsIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr buff)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	m_impl->dispatchCompute(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::generateMipmaps2d(TexturePtr tex, U face, U layer)
{
	m_impl->generateMipmaps2d(tex, face, layer);
}

void CommandBuffer::generateMipmaps3d(TexturePtr tex)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyTextureSurfaceToTextureSurface(
	TexturePtr src, const TextureSurfaceInfo& srcSurf, TexturePtr dest, const TextureSurfaceInfo& destSurf)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyTextureVolumeToTextureVolume(
	TexturePtr src, const TextureVolumeInfo& srcVol, TexturePtr dest, const TextureVolumeInfo& destVol)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::clearTextureSurface(
	TexturePtr tex, const TextureSurfaceInfo& surf, const ClearValue& clearValue, DepthStencilAspectBit aspect)
{
	m_impl->clearTextureSurface(tex, surf, clearValue, aspect);
}

void CommandBuffer::clearTextureVolume(
	TexturePtr tex, const TextureVolumeInfo& vol, const ClearValue& clearValue, DepthStencilAspectBit aspect)
{
	m_impl->clearTextureVolume(tex, vol, clearValue, aspect);
}

void CommandBuffer::copyBufferToTextureSurface(
	BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureSurfaceInfo& surf)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyBufferToTextureVolume(
	BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureVolumeInfo& vol)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
{
	m_impl->fillBuffer(buff, offset, size, value);
}

void CommandBuffer::writeOcclusionQueryResultToBuffer(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
{
	m_impl->writeOcclusionQueryResultToBuffer(query, offset, buff);
}

void CommandBuffer::copyBufferToBuffer(
	BufferPtr src, PtrSize srcOffset, BufferPtr dst, PtrSize dstOffset, PtrSize range)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setTextureSurfaceBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf)
{
	m_impl->setTextureSurfaceBarrier(tex, prevUsage, nextUsage, surf);
}

void CommandBuffer::setTextureVolumeBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol)
{
	m_impl->setTextureVolumeBarrier(tex, prevUsage, nextUsage, vol);
}

void CommandBuffer::setBufferBarrier(
	BufferPtr buff, BufferUsageBit before, BufferUsageBit after, PtrSize offset, PtrSize size)
{
	m_impl->setBufferBarrier(buff, before, after, offset, size);
}

void CommandBuffer::resetOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->resetOcclusionQuery(query);
}

void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->beginOcclusionQuery(query);
}

void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
	m_impl->endOcclusionQuery(query);
}

void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	m_impl->pushSecondLevelCommandBuffer(cmdb);
}

Bool CommandBuffer::isEmpty() const
{
	return m_impl->isEmpty();
}

} // end namespace anki

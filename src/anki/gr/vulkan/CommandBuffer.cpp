// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

CommandBuffer* CommandBuffer::newInstance(GrManager* manager, const CommandBufferInitInfo& init)
{
	return CommandBufferImpl::newInstanceHelper(manager, init);
}

CommandBufferInitHints CommandBuffer::computeInitHints() const
{
	// TODO
	CommandBufferInitHints hints;
	return hints;
}

void CommandBuffer::flush(FencePtr* fence)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.endRecording();

	if(!self.isSecondLevel())
	{
		self.getGrManagerImpl().flushCommandBuffer(CommandBufferPtr(this), fence);
	}
	else
	{
		ANKI_ASSERT(fence == nullptr);
	}
}

void CommandBuffer::finish()
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.endRecording();

	if(!self.isSecondLevel())
	{
		self.getGrManagerImpl().finishCommandBuffer(CommandBufferPtr(this));
	}
}

void CommandBuffer::bindVertexBuffer(
	U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride, VertexStepRate stepRate)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindVertexBuffer(binding, buff, offset, stride, stepRate);
}

void CommandBuffer::setVertexAttribute(U32 location, U32 buffBinding, const PixelFormat& fmt, PtrSize relativeOffset)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setVertexAttribute(location, buffBinding, fmt, relativeOffset);
}

void CommandBuffer::bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindIndexBuffer(buff, offset, type);
}

void CommandBuffer::setPrimitiveRestart(Bool enable)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setPrimitiveRestart(enable);
}

void CommandBuffer::setViewport(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setViewport(minx, miny, width, height);
}

void CommandBuffer::setScissor(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setScissor(minx, miny, width, height);
}

void CommandBuffer::setFillMode(FillMode mode)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setFillMode(mode);
}

void CommandBuffer::setCullMode(FaceSelectionBit mode)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setCullMode(mode);
}

void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setPolygonOffset(factor, units);
}

void CommandBuffer::setStencilOperations(FaceSelectionBit face,
	StencilOperation stencilFail,
	StencilOperation stencilPassDepthFail,
	StencilOperation stencilPassDepthPass)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setStencilOperations(face, stencilFail, stencilPassDepthFail, stencilPassDepthPass);
}

void CommandBuffer::setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setStencilCompareOperation(face, comp);
}

void CommandBuffer::setStencilCompareMask(FaceSelectionBit face, U32 mask)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setStencilCompareMask(face, mask);
}

void CommandBuffer::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setStencilWriteMask(face, mask);
}

void CommandBuffer::setStencilReference(FaceSelectionBit face, U32 ref)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setStencilReference(face, ref);
}

void CommandBuffer::setDepthWrite(Bool enable)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setDepthWrite(enable);
}

void CommandBuffer::setDepthCompareOperation(CompareOperation op)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setDepthCompareOperation(op);
}

void CommandBuffer::setAlphaToCoverage(Bool enable)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setAlphaToCoverage(enable);
}

void CommandBuffer::setColorChannelWriteMask(U32 attachment, ColorBit mask)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setColorChannelWriteMask(attachment, mask);
}

void CommandBuffer::setBlendFactors(
	U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA);
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setBlendOperation(attachment, funcRgb, funcA);
}

void CommandBuffer::bindTextureAndSampler(
	U32 set, U32 binding, TexturePtr tex, SamplerPtr sampler, TextureUsageBit usage, DepthStencilAspectBit aspect)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindTextureAndSampler(set, binding, tex, sampler, usage, aspect);
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindUniformBuffer(set, binding, buff, offset, range);
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindStorageBuffer(set, binding, buff, offset, range);
}

void CommandBuffer::bindImage(U32 set, U32 binding, TextureViewPtr img)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindImage(set, binding, img);
}

void CommandBuffer::bindTextureBuffer(
	U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, PixelFormat fmt)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindShaderProgram(ShaderProgramPtr prog)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindShaderProgram(prog);
}

void CommandBuffer::beginRenderPass(FramebufferPtr fb,
	const Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS>& colorAttachmentUsages,
	TextureUsageBit depthStencilAttachmentUsage,
	U32 minx,
	U32 miny,
	U32 width,
	U32 height)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.beginRenderPass(fb, colorAttachmentUsages, depthStencilAttachmentUsage, minx, miny, width, height);
}

void CommandBuffer::endRenderPass()
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.endRenderPass();
}

void CommandBuffer::drawElements(
	PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.drawElements(topology, count, instanceCount, firstIndex, baseVertex, baseInstance);
}

void CommandBuffer::drawArrays(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.drawArrays(topology, count, instanceCount, first, baseInstance);
}

void CommandBuffer::drawArraysIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr buff)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.drawArraysIndirect(topology, drawCount, offset, buff);
}

void CommandBuffer::drawElementsIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, BufferPtr buff)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.drawElementsIndirect(topology, drawCount, offset, buff);
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.dispatchCompute(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::generateMipmaps2d(TexturePtr tex, U face, U layer)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.generateMipmaps2d(tex, face, layer);
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
	ANKI_VK_SELF(CommandBufferImpl);
	self.clearTextureSurface(tex, surf, clearValue, aspect);
}

void CommandBuffer::clearTextureVolume(
	TexturePtr tex, const TextureVolumeInfo& vol, const ClearValue& clearValue, DepthStencilAspectBit aspect)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.clearTextureVolume(tex, vol, clearValue, aspect);
}

void CommandBuffer::copyBufferToTextureSurface(
	BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureSurfaceInfo& surf)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.copyBufferToTextureSurface(buff, offset, range, tex, surf);
}

void CommandBuffer::copyBufferToTextureVolume(
	BufferPtr buff, PtrSize offset, PtrSize range, TexturePtr tex, const TextureVolumeInfo& vol)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.copyBufferToTextureVolume(buff, offset, range, tex, vol);
}

void CommandBuffer::fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.fillBuffer(buff, offset, size, value);
}

void CommandBuffer::writeOcclusionQueryResultToBuffer(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.writeOcclusionQueryResultToBuffer(query, offset, buff);
}

void CommandBuffer::copyBufferToBuffer(
	BufferPtr src, PtrSize srcOffset, BufferPtr dst, PtrSize dstOffset, PtrSize range)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.copyBufferToBuffer(src, srcOffset, dst, dstOffset, range);
}

void CommandBuffer::setTextureBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSubresourceInfo& subresource)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setTextureBarrier(tex, prevUsage, nextUsage, subresource);
}

void CommandBuffer::setTextureSurfaceBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureSurfaceInfo& surf)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setTextureSurfaceBarrier(tex, prevUsage, nextUsage, surf);
}

void CommandBuffer::setTextureVolumeBarrier(
	TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage, const TextureVolumeInfo& vol)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setTextureVolumeBarrier(tex, prevUsage, nextUsage, vol);
}

void CommandBuffer::setBufferBarrier(
	BufferPtr buff, BufferUsageBit before, BufferUsageBit after, PtrSize offset, PtrSize size)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setBufferBarrier(buff, before, after, offset, size);
}

void CommandBuffer::resetOcclusionQuery(OcclusionQueryPtr query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.resetOcclusionQuery(query);
}

void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.beginOcclusionQuery(query);
}

void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.endOcclusionQuery(query);
}

void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.pushSecondLevelCommandBuffer(cmdb);
}

Bool CommandBuffer::isEmpty() const
{
	ANKI_VK_SELF_CONST(CommandBufferImpl);
	return self.isEmpty();
}

} // end namespace anki

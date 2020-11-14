// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/AccelerationStructure.h>

namespace anki
{

CommandBuffer* CommandBuffer::newInstance(GrManager* manager, const CommandBufferInitInfo& init)
{
	CommandBufferImpl* impl = manager->getAllocator().newInstance<CommandBufferImpl>(manager, init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
	}
	return impl;
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

void CommandBuffer::bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride,
									 VertexStepRate stepRate)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindVertexBuffer(binding, buff, offset, stride, stepRate);
}

void CommandBuffer::setVertexAttribute(U32 location, U32 buffBinding, Format fmt, PtrSize relativeOffset)
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

void CommandBuffer::setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail,
										 StencilOperation stencilPassDepthFail, StencilOperation stencilPassDepthPass)
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

void CommandBuffer::setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA,
									BlendFactor dstA)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA);
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setBlendOperation(attachment, funcRgb, funcA);
}

void CommandBuffer::bindTextureAndSampler(U32 set, U32 binding, TextureViewPtr texView, SamplerPtr sampler,
										  TextureUsageBit usage, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindTextureAndSamplerInternal(set, binding, texView, sampler, usage, arrayIdx);
}

void CommandBuffer::bindTexture(U32 set, U32 binding, TextureViewPtr texView, TextureUsageBit usage, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindTextureInternal(set, binding, texView, usage, arrayIdx);
}

void CommandBuffer::bindSampler(U32 set, U32 binding, SamplerPtr sampler, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindSamplerInternal(set, binding, sampler, arrayIdx);
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindUniformBufferInternal(set, binding, buff, offset, range, arrayIdx);
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindStorageBufferInternal(set, binding, buff, offset, range, arrayIdx);
}

void CommandBuffer::bindImage(U32 set, U32 binding, TextureViewPtr img, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindImageInternal(set, binding, img, arrayIdx);
}

void CommandBuffer::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructurePtr as, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindAccelerationStructureInternal(set, binding, as, arrayIdx);
}

void CommandBuffer::bindTextureBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, Format fmt,
									  U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindAllBindless(U32 set)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindAllBindlessInternal(set);
}

void CommandBuffer::bindShaderProgram(ShaderProgramPtr prog)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.bindShaderProgram(prog);
}

void CommandBuffer::beginRenderPass(FramebufferPtr fb,
									const Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS>& colorAttachmentUsages,
									TextureUsageBit depthStencilAttachmentUsage, U32 minx, U32 miny, U32 width,
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

void CommandBuffer::drawElements(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex,
								 U32 baseVertex, U32 baseInstance)
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

void CommandBuffer::traceRays(BufferPtr sbtBuffer, PtrSize sbtBufferOffset, U32 sbtRecordSize,
							  U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width, U32 height, U32 depth)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.traceRaysInternal(sbtBuffer, sbtBufferOffset, sbtRecordSize, hitGroupSbtRecordCount, rayTypeCount, width,
						   height, depth);
}

void CommandBuffer::generateMipmaps2d(TextureViewPtr texView)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.generateMipmaps2d(texView);
}

void CommandBuffer::generateMipmaps3d(TextureViewPtr texView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::blitTextureViews(TextureViewPtr srcView, TextureViewPtr destView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::clearTextureView(TextureViewPtr texView, const ClearValue& clearValue)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.clearTextureView(texView, clearValue);
}

void CommandBuffer::copyBufferToTextureView(BufferPtr buff, PtrSize offset, PtrSize range, TextureViewPtr texView)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.copyBufferToTextureViewInternal(buff, offset, range, texView);
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

void CommandBuffer::copyBufferToBuffer(BufferPtr src, PtrSize srcOffset, BufferPtr dst, PtrSize dstOffset,
									   PtrSize range)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.copyBufferToBuffer(src, srcOffset, dst, dstOffset, range);
}

void CommandBuffer::buildAccelerationStructure(AccelerationStructurePtr as)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.buildAccelerationStructureInternal(as);
}

void CommandBuffer::setTextureBarrier(TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
									  const TextureSubresourceInfo& subresource)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setTextureBarrier(tex, prevUsage, nextUsage, subresource);
}

void CommandBuffer::setTextureSurfaceBarrier(TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
											 const TextureSurfaceInfo& surf)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setTextureSurfaceBarrier(tex, prevUsage, nextUsage, surf);
}

void CommandBuffer::setTextureVolumeBarrier(TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
											const TextureVolumeInfo& vol)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setTextureVolumeBarrier(tex, prevUsage, nextUsage, vol);
}

void CommandBuffer::setBufferBarrier(BufferPtr buff, BufferUsageBit before, BufferUsageBit after, PtrSize offset,
									 PtrSize size)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setBufferBarrier(buff, before, after, offset, size);
}

void CommandBuffer::setAccelerationStructureBarrier(AccelerationStructurePtr as,
													AccelerationStructureUsageBit prevUsage,
													AccelerationStructureUsageBit nextUsage)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setAccelerationStructureBarrierInternal(as, prevUsage, nextUsage);
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

void CommandBuffer::resetTimestampQuery(TimestampQueryPtr query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.resetTimestampQueryInternal(query);
}

void CommandBuffer::writeTimestamp(TimestampQueryPtr query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.writeTimestampInternal(query);
}

Bool CommandBuffer::isEmpty() const
{
	ANKI_VK_SELF_CONST(CommandBufferImpl);
	return self.isEmpty();
}

void CommandBuffer::setPushConstants(const void* data, U32 dataSize)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setPushConstants(data, dataSize);
}

void CommandBuffer::setRasterizationOrder(RasterizationOrder order)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setRasterizationOrder(order);
}

void CommandBuffer::setLineWidth(F32 width)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.setLineWidth(width);
}

void CommandBuffer::addReference(GrObjectPtr ptr)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.addReference(ptr);
}

} // end namespace anki

// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommandBuffer.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

CommandBuffer* CommandBuffer::newInstance(const CommandBufferInitInfo& init)
{
	ANKI_TRACE_SCOPED_EVENT(D3DNewCommandBuffer);
	CommandBufferImpl* impl = anki::newInstance<CommandBufferImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

void CommandBuffer::endRecording()
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindVertexBuffer(U32 binding, Buffer* buff, PtrSize offset, PtrSize stride, VertexStepRate stepRate)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setVertexAttribute(VertexAttribute attribute, U32 buffBinding, Format fmt, PtrSize relativeOffset)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindIndexBuffer(Buffer* buff, PtrSize offset, IndexType type)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setPrimitiveRestart(Bool enable)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setViewport(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setScissor(U32 minx, U32 miny, U32 width, U32 height)
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
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
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
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setStencilReference(FaceSelectionBit face, U32 ref)
{
	ANKI_ASSERT(!"TODO");
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

void CommandBuffer::setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindTexture(U32 set, U32 binding, TextureView* texView, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindSampler(U32 set, U32 binding, Sampler* sampler, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindStorageTexture(U32 set, U32 binding, TextureView* img, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructure* as, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindReadOnlyTexelBuffer(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, Format fmt, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindAllBindless(U32 set)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindShaderProgram(ShaderProgram* prog)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::beginRenderPass(Framebuffer* fb, const Array<TextureUsageBit, kMaxColorRenderTargets>& colorAttachmentUsages,
									TextureUsageBit depthStencilAttachmentUsage, U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::endRenderPass()
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setVrsRate(VrsRate rate)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndexed(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::draw(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, Buffer* buff)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndexedIndirectCount(PrimitiveTopology topology, Buffer* argBuffer, PtrSize argBufferOffset, U32 argBufferStride,
											 Buffer* countBuffer, PtrSize countBufferOffset, U32 maxDrawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndirectCount(PrimitiveTopology topology, Buffer* argBuffer, PtrSize argBufferOffset, U32 argBufferStride,
									  Buffer* countBuffer, PtrSize countBufferOffset, U32 maxDrawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndexedIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, Buffer* buff)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawMeshTasks(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawMeshTasksIndirect(Buffer* argBuffer, PtrSize argBufferOffset)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::dispatchComputeIndirect(Buffer* argBuffer, PtrSize argBufferOffset)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::traceRays(Buffer* sbtBuffer, PtrSize sbtBufferOffset, U32 sbtRecordSize, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width,
							  U32 height, U32 depth)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::generateMipmaps2d(TextureView* texView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::generateMipmaps3d([[maybe_unused]] TextureView* texView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::blitTextureViews([[maybe_unused]] TextureView* srcView, [[maybe_unused]] TextureView* destView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::clearTextureView(TextureView* texView, const ClearValue& clearValue)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyBufferToTextureView(Buffer* buff, PtrSize offset, PtrSize range, TextureView* texView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::fillBuffer(Buffer* buff, PtrSize offset, PtrSize size, U32 value)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::writeOcclusionQueriesResultToBuffer(ConstWeakArray<OcclusionQuery*> queries, PtrSize offset, Buffer* buff)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyBufferToBuffer(Buffer* src, Buffer* dst, ConstWeakArray<CopyBufferToBufferInfo> copies)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::buildAccelerationStructure(AccelerationStructure* as, Buffer* scratchBuffer, PtrSize scratchBufferOffset)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::upscale(GrUpscaler* upscaler, TextureView* inColor, TextureView* outUpscaledColor, TextureView* motionVectors, TextureView* depth,
							TextureView* exposure, Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& motionVectorsScale)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
									   ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::resetOcclusionQueries(ConstWeakArray<OcclusionQuery*> queries)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::beginOcclusionQuery(OcclusionQuery* query)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::endOcclusionQuery(OcclusionQuery* query)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::beginPipelineQuery(PipelineQuery* query)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::endPipelineQuery(PipelineQuery* query)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::resetTimestampQueries(ConstWeakArray<TimestampQuery*> queries)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::writeTimestamp(TimestampQuery* query)
{
	ANKI_ASSERT(!"TODO");
}

Bool CommandBuffer::isEmpty() const
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setPushConstants(const void* data, U32 dataSize)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setRasterizationOrder(RasterizationOrder order)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setLineWidth(F32 width)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::pushDebugMarker(CString name, Vec3 color)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::popDebugMarker()
{
	ANKI_ASSERT(!"TODO");
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki

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

void CommandBuffer::bindVertexBuffer(U32 binding, const BufferView& buff, PtrSize stride, VertexStepRate stepRate)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setVertexAttribute(VertexAttribute attribute, U32 buffBinding, Format fmt, PtrSize relativeOffset)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindIndexBuffer(const BufferView& buff, IndexType type)
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

void CommandBuffer::bindTexture(U32 set, U32 binding, const TextureView& texView, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindSampler(U32 set, U32 binding, Sampler* sampler, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, const BufferView& buff, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, const BufferView& buff, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindStorageTexture(U32 set, U32 binding, const TextureView& tex, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructure* as, U32 arrayIdx)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindReadOnlyTexelBuffer(U32 set, U32 binding, const BufferView& buff, Format fmt, U32 arrayIdx)
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

void CommandBuffer::beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, U32 minx, U32 miny, U32 width, U32 height,
									const TextureView& vrsRt, U8 vrsRtTexelSizeX, U8 vrsRtTexelSizeY)
{
	ANKI_D3D_SELF(CommandBufferImpl);

	Array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, kMaxColorRenderTargets> colorRtDescs;
	for(U32 i = 0; i < colorRts.getSize(); ++i)
	{
		const RenderTarget& rt = colorRts[i];
		D3D12_RENDER_PASS_RENDER_TARGET_DESC& desc = colorRtDescs[i];

		desc = {};
		// desc.cpuDescriptor = rt.m_view->
		desc.BeginningAccess.Type = convertLoadOp(rt.m_loadOperation);
		memcpy(&desc.BeginningAccess.Clear.ClearValue.Color, &rt.m_clearValue.m_colorf[0], sizeof(F32) * 4);
		desc.EndingAccess.Type = convertStoreOp(rt.m_storeOperation);
	}

	// self.m_cmdList->BeginRenderPass(colorRts.getSize(), )
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

void CommandBuffer::drawIndirect(PrimitiveTopology topology, const BufferView& buff, U32 drawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndexedIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride,
											 const BufferView& countBuffer, U32 maxDrawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride, const BufferView& countBuffer,
									  U32 maxDrawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndexedIndirect(PrimitiveTopology topology, const BufferView& buff, U32 drawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawMeshTasks(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawMeshTasksIndirect(const BufferView& argBuffer, U32 drawCount)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::dispatchComputeIndirect(const BufferView& argBuffer)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::traceRays(const BufferView& sbtBuffer, U32 sbtRecordSize, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width, U32 height,
							  U32 depth)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::generateMipmaps2d(const TextureView& texView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::blitTexture(const TextureView& srcView, const TextureView& destView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::clearTexture(const TextureView& texView, const ClearValue& clearValue)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyBufferToTexture(const BufferView& buff, const TextureView& texView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::fillBuffer(const BufferView& buff, U32 value)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::writeOcclusionQueriesResultToBuffer(ConstWeakArray<OcclusionQuery*> queries, const BufferView& buff)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::copyBufferToBuffer(Buffer* src, Buffer* dst, ConstWeakArray<CopyBufferToBufferInfo> copies)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::buildAccelerationStructure(AccelerationStructure* as, const BufferView& scratchBuffer)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::upscale(GrUpscaler* upscaler, const TextureView& inColor, const TextureView& outUpscaledColor, const TextureView& motionVectors,
							const TextureView& depth, const TextureView& exposure, Bool resetAccumulation, const Vec2& jitterOffset,
							const Vec2& motionVectorsScale)
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

CommandBufferImpl::~CommandBufferImpl()
{
	safeRelease(m_cmdList);
	safeRelease(m_cmdAllocator);
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	ANKI_D3D_CHECK(getDevice().CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator)));

	ANKI_D3D_CHECK(getDevice().CreateCommandList(
		0, !!(init.m_flags & CommandBufferFlag::kGeneralWork) ? D3D12_COMMAND_LIST_TYPE_DIRECT : D3D12_COMMAND_LIST_TYPE_COMPUTE, m_cmdAllocator,
		nullptr, IID_PPV_ARGS(&m_cmdList)));

	return Error::kNone;
}

} // end namespace anki

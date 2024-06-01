// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommandBuffer.h>
#include <AnKi/Gr/D3D/D3DTexture.h>
#include <AnKi/Gr/D3D/D3DSampler.h>
#include <AnKi/Gr/D3D/D3DShaderProgram.h>
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
	ANKI_D3D_SELF(CommandBufferImpl);

	self.m_cmdList->Close();
}

void CommandBuffer::bindVertexBuffer(U32 binding, const BufferView& buff, PtrSize stride, VertexStepRate stepRate)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setVertexAttribute(VertexAttributeSemantic attribute, U32 buffBinding, Format fmt, PtrSize relativeOffset)
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

void CommandBuffer::bindTexture(Register reg, const TextureView& texView)
{
	reg.validate();
	ANKI_D3D_SELF(CommandBufferImpl);

	const TextureImpl& impl = static_cast<const TextureImpl&>(texView.getTexture());

	if(reg.m_resourceType == HlslResourceType::kSrv)
	{
		const DescriptorHeapHandle handle = impl.getOrCreateSrv(texView.getSubresource());
		self.m_descriptors.bindSrv(reg.m_space, reg.m_bindPoint, handle);
	}
	else
	{
		const DescriptorHeapHandle handle = impl.getOrCreateUav(texView.getSubresource());
		self.m_descriptors.bindUav(reg.m_space, reg.m_bindPoint, handle);
	}
}

void CommandBuffer::bindSampler(Register reg, Sampler* sampler)
{
	reg.validate();
	ANKI_ASSERT(sampler);
	ANKI_D3D_SELF(CommandBufferImpl);

	const SamplerImpl& impl = static_cast<const SamplerImpl&>(*sampler);
	self.m_descriptors.bindSampler(reg.m_space, reg.m_bindPoint, impl.m_handle);

	self.m_mcmdb->pushObjectRef(sampler);
}

void CommandBuffer::bindUniformBuffer(Register reg, const BufferView& buff)
{
	reg.validate();
	ANKI_D3D_SELF(CommandBufferImpl);

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	self.m_descriptors.bindCbv(reg.m_space, reg.m_bindPoint, &impl.getD3DResource(), buff.getOffset(), buff.getRange());
}

void CommandBuffer::bindStorageBuffer(Register reg, const BufferView& buff)
{
	reg.validate();
	ANKI_D3D_SELF(CommandBufferImpl);

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	if(reg.m_resourceType == HlslResourceType::kUav)
	{
		self.m_descriptors.bindUav(reg.m_space, reg.m_bindPoint, &impl.getD3DResource(), buff.getOffset(), buff.getRange());
	}
	else
	{
		ANKI_ASSERT(reg.m_resourceType == HlslResourceType::kSrv);
		self.m_descriptors.bindSrv(reg.m_space, reg.m_bindPoint, &impl.getD3DResource(), buff.getOffset(), buff.getRange());
	}
}

void CommandBuffer::bindTexelBuffer(Register reg, const BufferView& buff, Format fmt)
{
	reg.validate();
	ANKI_D3D_SELF(CommandBufferImpl);

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	if(reg.m_resourceType == HlslResourceType::kUav)
	{
		self.m_descriptors.bindUav(reg.m_space, reg.m_bindPoint, &impl.getD3DResource(), buff.getOffset(), buff.getRange(), DXGI_FORMAT(fmt));
	}
	else
	{
		ANKI_ASSERT(reg.m_resourceType == HlslResourceType::kSrv);
		self.m_descriptors.bindSrv(reg.m_space, reg.m_bindPoint, &impl.getD3DResource(), buff.getOffset(), buff.getRange(), DXGI_FORMAT(fmt));
	}
}

void CommandBuffer::bindAccelerationStructure(Register reg, AccelerationStructure* as)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::bindShaderProgram(ShaderProgram* prog)
{
	ANKI_ASSERT(prog);
	ANKI_D3D_SELF(CommandBufferImpl);

	self.commandCommon();

	const ShaderProgramImpl& progImpl = static_cast<const ShaderProgramImpl&>(*prog);
	const Bool isCompute = !(progImpl.getShaderTypes() & ShaderTypeBit::kAllGraphics);

	self.m_descriptors.bindRootSignature(&progImpl.getRootSignature(), isCompute);

	self.m_mcmdb->pushObjectRef(prog);

	if(isCompute)
	{
		self.m_cmdList->SetPipelineState(&progImpl.getComputePipelineState());
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	// Shader program means descriptors so bind the descriptor heaps
	if(!self.m_descriptorHeapsBound)
	{
		self.m_descriptorHeapsBound = true;

		const Array<ID3D12DescriptorHeap*, 2> heaps = DescriptorFactory::getSingleton().getCpuGpuVisibleHeaps();
		self.m_cmdList->SetDescriptorHeaps(2, heaps.getBegin());
	}
}

void CommandBuffer::beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, U32 minx, U32 miny, U32 width, U32 height,
									const TextureView& vrsRt, U8 vrsRtTexelSizeX, U8 vrsRtTexelSizeY)
{
	ANKI_D3D_SELF(CommandBufferImpl);
	self.commandCommon();

	Array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, kMaxColorRenderTargets> colorRtDescs;
	for(U32 i = 0; i < colorRts.getSize(); ++i)
	{
		const RenderTarget& rt = colorRts[i];
		D3D12_RENDER_PASS_RENDER_TARGET_DESC& desc = colorRtDescs[i];

		desc = {};
		desc.cpuDescriptor =
			static_cast<const TextureImpl&>(rt.m_textureView.getTexture()).getOrCreateRtv(rt.m_textureView.getSubresource()).getCpuOffset();
		desc.BeginningAccess.Type = convertLoadOp(rt.m_loadOperation);
		memcpy(&desc.BeginningAccess.Clear.ClearValue.Color, &rt.m_clearValue.m_colorf[0], sizeof(F32) * 4);
		desc.EndingAccess.Type = convertStoreOp(rt.m_storeOperation);
	}

	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsDesc;
	if(depthStencilRt)
	{
		dsDesc = {};
		dsDesc.cpuDescriptor = static_cast<const TextureImpl&>(depthStencilRt->m_textureView.getTexture())
								   .getOrCreateDsv(depthStencilRt->m_textureView.getSubresource(), depthStencilRt->m_usage)
								   .getCpuOffset();

		dsDesc.DepthBeginningAccess.Type = convertLoadOp(depthStencilRt->m_loadOperation);
		dsDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth, depthStencilRt->m_clearValue.m_depthStencil.m_depth;
		dsDesc.DepthEndingAccess.Type = convertStoreOp(depthStencilRt->m_storeOperation);

		dsDesc.StencilBeginningAccess.Type = convertLoadOp(depthStencilRt->m_stencilLoadOperation);
		dsDesc.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = depthStencilRt->m_clearValue.m_depthStencil.m_stencil;
		dsDesc.StencilEndingAccess.Type = convertStoreOp(depthStencilRt->m_stencilStoreOperation);
	}

	self.m_cmdList->BeginRenderPass(colorRts.getSize(), colorRtDescs.getBegin(), (depthStencilRt) ? &dsDesc : nullptr,
									D3D12_RENDER_PASS_FLAG_ALLOW_UAV_WRITES);
}

void CommandBuffer::endRenderPass()
{
	ANKI_D3D_SELF(CommandBufferImpl);

	self.m_cmdList->EndRenderPass();
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
	ANKI_D3D_SELF(CommandBufferImpl);
	self.commandCommon();

	self.m_descriptors.flush(*self.m_cmdList);

	self.m_cmdList->Dispatch(groupCountX, groupCountY, groupCountZ);
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
	ANKI_D3D_SELF(CommandBufferImpl);

	self.commandCommon();

	const BufferImpl& buffImpl = static_cast<const BufferImpl&>(buff.getBuffer());
	const TextureImpl& texImpl = static_cast<const TextureImpl&>(texView.getTexture());

	const U32 width = texImpl.getWidth() >> texView.getFirstMipmap();
	const U32 height = texImpl.getHeight() >> texView.getFirstMipmap();
	const U32 depth = (texImpl.getTextureType() == TextureType::k3D) ? (texImpl.getDepth() >> texView.getFirstMipmap()) : 1u;
	ANKI_ASSERT(width && height && depth);

	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
	srcLocation.pResource = &buffImpl.getD3DResource();
	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcLocation.PlacedFootprint.Offset = buff.getOffset();
	srcLocation.PlacedFootprint.Footprint.Format = texImpl.getDxgiFormat();
	srcLocation.PlacedFootprint.Footprint.Width = width;
	srcLocation.PlacedFootprint.Footprint.Height = height;
	srcLocation.PlacedFootprint.Footprint.Depth = depth;
	srcLocation.PlacedFootprint.Footprint.RowPitch = width * getFormatInfo(texImpl.getFormat()).m_texelSize;

	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
	dstLocation.pResource = &texImpl.getD3DResource();
	dstLocation.SubresourceIndex = texImpl.calcD3DSubresourceIndex(texView.getSubresource());

	self.m_cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
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
	ANKI_D3D_SELF(CommandBufferImpl);
	self.commandCommon();

	DynamicArray<D3D12_RESOURCE_BARRIER, MemoryPoolPtrWrapper<StackMemoryPool>> resourceBarriers(self.m_fastPool);

	for(const TextureBarrierInfo& barrier : textures)
	{
		const TextureImpl& impl = static_cast<const TextureImpl&>(barrier.m_textureView.getTexture());

		D3D12_RESOURCE_BARRIER& d3dBarrier = *resourceBarriers.emplaceBack();
		d3dBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		d3dBarrier.Transition.pResource = &impl.getD3DResource();

		if(barrier.m_textureView.isAllSurfacesOrVolumes() && barrier.m_textureView.getDepthStencilAspect() == impl.getDepthStencilAspect())
		{
			d3dBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		}
		else
		{
			d3dBarrier.Transition.Subresource = impl.calcD3DSubresourceIndex(barrier.m_textureView.getSubresource());
		}

		impl.computeBarrierInfo(barrier.m_previousUsage, barrier.m_nextUsage, d3dBarrier.Transition.StateBefore, d3dBarrier.Transition.StateAfter);

		if(d3dBarrier.Transition.StateBefore & D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		   && d3dBarrier.Transition.StateAfter & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		{
			D3D12_RESOURCE_BARRIER& d3dBarrier = *resourceBarriers.emplaceBack();
			d3dBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			d3dBarrier.UAV.pResource = &impl.getD3DResource();
		}
	}

	ANKI_ASSERT(buffers.getSize() == 0 && "TODO");
	ANKI_ASSERT(accelerationStructures.getSize() == 0 && "TODO");

	ANKI_ASSERT(resourceBarriers.getSize() > 0);
	self.m_cmdList->ResourceBarrier(resourceBarriers.getSize(), resourceBarriers.getBegin());
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
	ANKI_D3D_SELF_CONST(CommandBufferImpl);
	return self.m_commandCount == 0;
}

void CommandBuffer::setPushConstants(const void* data, U32 dataSize)
{
	ANKI_D3D_SELF(CommandBufferImpl);

	self.m_descriptors.setRootConstants(data, dataSize);
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
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	ANKI_CHECK(CommandBufferFactory::getSingleton().newCommandBuffer(init.m_flags, m_mcmdb));

	m_cmdList = &m_mcmdb->getCmdList();
	m_fastPool = &m_mcmdb->getFastMemoryPool();

	m_descriptors.init(m_fastPool);

	return Error::kNone;
}

void CommandBufferImpl::commandCommon()
{
	++m_commandCount;
	if(m_commandCount >= kCommandBufferSmallBatchMaxCommands)
	{
		if((m_commandCount % 10) == 0) // Change the batch every 10 commands as an optimization
		{
			m_mcmdb->setBigBatch();
		}
	}
}

} // end namespace anki

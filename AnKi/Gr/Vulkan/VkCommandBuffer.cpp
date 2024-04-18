// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkCommandBuffer.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>
#include <AnKi/Gr/AccelerationStructure.h>
#include <AnKi/Gr/Vulkan/VkFence.h>
#include <AnKi/Gr/Vulkan/VkGrUpscaler.h>
#include <AnKi/Gr/Vulkan/VkOcclusionQuery.h>
#include <AnKi/Gr/Vulkan/VkTimestampQuery.h>

#if ANKI_DLSS
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx.h>
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx_helpers.h>
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx_vk.h>
#	include <ThirdParty/DlssSdk/sdk/include/nvsdk_ngx_helpers_vk.h>
#endif

namespace anki {

CommandBuffer* CommandBuffer::newInstance(const CommandBufferInitInfo& init)
{
	ANKI_TRACE_SCOPED_EVENT(VkNewCommandBuffer);
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
	ANKI_VK_SELF(CommandBufferImpl);
	self.endRecording();
}

void CommandBuffer::bindVertexBuffer(U32 binding, Buffer* buff, PtrSize offset, PtrSize stride, VertexStepRate stepRate)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.bindVertexBuffer(binding, stride, stepRate);
	const VkBuffer vkbuff = static_cast<const BufferImpl&>(*buff).getHandle();
	vkCmdBindVertexBuffers(self.m_handle, binding, 1, &vkbuff, &offset);
}

void CommandBuffer::setVertexAttribute(VertexAttribute attribute, U32 buffBinding, Format fmt, PtrSize relativeOffset)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setVertexAttribute(attribute, buffBinding, fmt, relativeOffset);
}

void CommandBuffer::bindIndexBuffer(Buffer* buff, PtrSize offset, IndexType type)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const BufferImpl& buffi = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(!!(buffi.getBufferUsage() & BufferUsageBit::kIndex));
	vkCmdBindIndexBuffer(self.m_handle, buffi.getHandle(), offset, convertIndexType(type));
}

void CommandBuffer::setPrimitiveRestart(Bool enable)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setPrimitiveRestart(enable);
}

void CommandBuffer::setViewport(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(width > 0 && height > 0);
	self.commandCommon();

	if(self.m_viewport[0] != minx || self.m_viewport[1] != miny || self.m_viewport[2] != width || self.m_viewport[3] != height)
	{
		self.m_viewportDirty = true;

		self.m_viewport[0] = minx;
		self.m_viewport[1] = miny;
		self.m_viewport[2] = width;
		self.m_viewport[3] = height;
	}
}

void CommandBuffer::setScissor(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(width > 0 && height > 0);
	self.commandCommon();

	if(self.m_scissor[0] != minx || self.m_scissor[1] != miny || self.m_scissor[2] != width || self.m_scissor[3] != height)
	{
		self.m_scissorDirty = true;

		self.m_scissor[0] = minx;
		self.m_scissor[1] = miny;
		self.m_scissor[2] = width;
		self.m_scissor[3] = height;
	}
}

void CommandBuffer::setFillMode(FillMode mode)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setFillMode(mode);
}

void CommandBuffer::setCullMode(FaceSelectionBit mode)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setCullMode(mode);
}

void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setPolygonOffset(factor, units);
	vkCmdSetDepthBias(self.m_handle, factor, 0.0f, units);
}

void CommandBuffer::setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
										 StencilOperation stencilPassDepthPass)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setStencilOperations(face, stencilFail, stencilPassDepthFail, stencilPassDepthPass);
}

void CommandBuffer::setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setStencilCompareOperation(face, comp);
}

void CommandBuffer::setStencilCompareMask(FaceSelectionBit face, U32 mask)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::kFront) && self.m_stencilCompareMasks[0] != mask)
	{
		self.m_stencilCompareMasks[0] = mask;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::kBack) && self.m_stencilCompareMasks[1] != mask)
	{
		self.m_stencilCompareMasks[1] = mask;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		vkCmdSetStencilCompareMask(self.m_handle, flags, mask);
	}
}

void CommandBuffer::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::kFront) && self.m_stencilWriteMasks[0] != mask)
	{
		self.m_stencilWriteMasks[0] = mask;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::kBack) && self.m_stencilWriteMasks[1] != mask)
	{
		self.m_stencilWriteMasks[1] = mask;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		vkCmdSetStencilWriteMask(self.m_handle, flags, mask);
	}
}

void CommandBuffer::setStencilReference(FaceSelectionBit face, U32 ref)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	VkStencilFaceFlags flags = 0;

	if(!!(face & FaceSelectionBit::kFront) && self.m_stencilReferenceMasks[0] != ref)
	{
		self.m_stencilReferenceMasks[0] = ref;
		flags = VK_STENCIL_FACE_FRONT_BIT;
	}

	if(!!(face & FaceSelectionBit::kBack) && self.m_stencilReferenceMasks[1] != ref)
	{
		self.m_stencilWriteMasks[1] = ref;
		flags |= VK_STENCIL_FACE_BACK_BIT;
	}

	if(flags)
	{
		vkCmdSetStencilReference(self.m_handle, flags, ref);
	}
}

void CommandBuffer::setDepthWrite(Bool enable)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setDepthWrite(enable);
}

void CommandBuffer::setDepthCompareOperation(CompareOperation op)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setDepthCompareOperation(op);
}

void CommandBuffer::setAlphaToCoverage(Bool enable)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setAlphaToCoverage(enable);
}

void CommandBuffer::setColorChannelWriteMask(U32 attachment, ColorBit mask)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setColorChannelWriteMask(attachment, mask);
}

void CommandBuffer::setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA);
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setBlendOperation(attachment, funcRgb, funcA);
}

void CommandBuffer::bindTexture(U32 set, U32 binding, TextureView* texView, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();
	ANKI_ASSERT(tex.isSubresourceGoodForSampling(view.getSubresource()));
	const VkImageLayout lay = tex.computeLayout(TextureUsageBit::kAllSampled & tex.getTextureUsage(), 0);

	self.m_dsetState[set].bindTexture(binding, arrayIdx, texView, lay);
}

void CommandBuffer::bindSampler(U32 set, U32 binding, Sampler* sampler, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindSampler(binding, arrayIdx, sampler);
	self.m_microCmdb->pushObjectRef(sampler);
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindUniformBuffer(binding, arrayIdx, buff, offset, range);
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindStorageBuffer(binding, arrayIdx, buff, offset, range);
}

void CommandBuffer::bindStorageTexture(U32 set, U32 binding, TextureView* img, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindStorageTexture(binding, arrayIdx, img);

	const Bool isPresentable = !!(static_cast<const TextureViewImpl&>(*img).getTextureImpl().getTextureUsage() & TextureUsageBit::kPresent);
	if(isPresentable)
	{
		self.m_renderedToDefaultFb = true;
	}
}

void CommandBuffer::bindAccelerationStructure(U32 set, U32 binding, AccelerationStructure* as, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindAccelerationStructure(binding, arrayIdx, as);
	self.m_microCmdb->pushObjectRef(as);
}

void CommandBuffer::bindReadOnlyTexelBuffer(U32 set, U32 binding, Buffer* buff, PtrSize offset, PtrSize range, Format fmt, U32 arrayIdx)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindReadOnlyTexelBuffer(binding, arrayIdx, buff, offset, range, fmt);
}

void CommandBuffer::bindAllBindless(U32 set)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_dsetState[set].bindBindlessDescriptorSet();
}

void CommandBuffer::bindShaderProgram(ShaderProgram* prog)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	ShaderProgramImpl& impl = static_cast<ShaderProgramImpl&>(*prog);

	if(impl.isGraphics())
	{
		self.m_graphicsProg = &impl;
		self.m_computeProg = nullptr; // Unbind the compute prog. Doesn't work like vulkan
		self.m_rtProg = nullptr; // See above
		self.m_state.bindShaderProgram(&impl);
	}
	else if(!!(impl.getStages() & ShaderTypeBit::kCompute))
	{
		self.m_computeProg = &impl;
		self.m_graphicsProg = nullptr; // See comment in the if()
		self.m_rtProg = nullptr; // See above

		// Bind the pipeline now
		vkCmdBindPipeline(self.m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, impl.getComputePipelineHandle());
	}
	else
	{
		ANKI_ASSERT(!!(impl.getStages() & ShaderTypeBit::kAllRayTracing));
		self.m_computeProg = nullptr;
		self.m_graphicsProg = nullptr;
		self.m_rtProg = &impl;

		// Bind now
		vkCmdBindPipeline(self.m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, impl.getRayTracingPipelineHandle());
	}

	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(impl.getReflectionInfo().m_descriptorSetMask.get(i))
		{
			self.m_dsetState[i].setLayout(&impl.getDescriptorSetLayout(i));
		}
		else
		{
			// According to the spec the bound DS may be disturbed if the ppline layout is not compatible. Play it safe and dirty the slot. That will
			// force rebind of the DS at drawcall time.
			self.m_dsetState[i].setLayoutDirty();
		}
	}

	self.m_microCmdb->pushObjectRef(prog);

#if ANKI_EXTRA_CHECKS
	self.m_setPushConstantsSize = 0;
#endif
}

void CommandBuffer::beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, U32 minx, U32 miny, U32 width, U32 height,
									TextureView* vrsRt, U8 vrsRtTexelSizeX, U8 vrsRtTexelSizeY)
{
	ANKI_VK_SELF(CommandBufferImpl);

	ANKI_ASSERT(!self.m_insideRenderpass);
#if ANKI_ASSERTIONS_ENABLED
	self.m_insideRenderpass = true;
#endif

	self.commandCommon();

	Array<VkRenderingAttachmentInfo, kMaxColorRenderTargets> vkColorAttachments;
	VkRenderingAttachmentInfo vkDepthAttachment;
	VkRenderingAttachmentInfo vkStencilAttachment;
	VkRenderingFragmentShadingRateAttachmentInfoKHR vkVrsAttachment;
	VkRenderingInfo info = {};
	Bool drawsToSwapchain = false;
	U32 fbWidth = 0;
	U32 fbHeight = 0;

	Array<Format, kMaxColorRenderTargets> colorFormats = {};
	Format dsFormat = Format::kNone;

	// Do color targets
	for(U32 i = 0; i < colorRts.getSize(); ++i)
	{
		ANKI_ASSERT(!colorRts[i].m_aspect);
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*colorRts[i].m_view);

		vkColorAttachments[i] = {};
		vkColorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		vkColorAttachments[i].imageView = view.getHandle();
		vkColorAttachments[i].imageLayout = view.getTextureImpl().computeLayout(colorRts[i].m_usage, 0);
		vkColorAttachments[i].loadOp = convertLoadOp(colorRts[i].m_loadOperation);
		vkColorAttachments[i].storeOp = convertStoreOp(colorRts[i].m_storeOperation);
		vkColorAttachments[i].clearValue.color.float32[0] = colorRts[i].m_clearValue.m_colorf[0];
		vkColorAttachments[i].clearValue.color.float32[1] = colorRts[i].m_clearValue.m_colorf[1];
		vkColorAttachments[i].clearValue.color.float32[2] = colorRts[i].m_clearValue.m_colorf[2];
		vkColorAttachments[i].clearValue.color.float32[3] = colorRts[i].m_clearValue.m_colorf[3];

		if(!!(view.getTextureImpl().getTextureUsage() & TextureUsageBit::kPresent))
		{
			drawsToSwapchain = true;
		}

		ANKI_ASSERT(fbWidth == 0 || (fbWidth == view.getTextureImpl().getWidth() >> view.getSubresource().m_firstMipmap));
		ANKI_ASSERT(fbHeight == 0 || (fbHeight == view.getTextureImpl().getHeight() >> view.getSubresource().m_firstMipmap));
		fbWidth = view.getTextureImpl().getWidth() >> view.getSubresource().m_firstMipmap;
		fbHeight = view.getTextureImpl().getHeight() >> view.getSubresource().m_firstMipmap;

		colorFormats[i] = view.getTextureImpl().getFormat();
	}

	if(colorRts.getSize())
	{
		info.colorAttachmentCount = colorRts.getSize();
		info.pColorAttachments = vkColorAttachments.getBegin();

		ANKI_ASSERT((!drawsToSwapchain || colorRts.getSize() == 1) && "Can't handle that");
	}

	// DS
	if(depthStencilRt)
	{
		ANKI_ASSERT(!!depthStencilRt->m_aspect);
		const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*depthStencilRt->m_view);

		if(!!(depthStencilRt->m_aspect & DepthStencilAspectBit::kDepth))
		{
			vkDepthAttachment = {};
			vkDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkDepthAttachment.imageView = view.getHandle();
			vkDepthAttachment.imageLayout = view.getTextureImpl().computeLayout(depthStencilRt->m_usage, 0);
			vkDepthAttachment.loadOp = convertLoadOp(depthStencilRt->m_loadOperation);
			vkDepthAttachment.storeOp = convertStoreOp(depthStencilRt->m_storeOperation);
			vkDepthAttachment.clearValue.depthStencil.depth = depthStencilRt->m_clearValue.m_depthStencil.m_depth;
			vkDepthAttachment.clearValue.depthStencil.stencil = depthStencilRt->m_clearValue.m_depthStencil.m_stencil;

			info.pDepthAttachment = &vkDepthAttachment;
		}

		if(!!(depthStencilRt->m_aspect & DepthStencilAspectBit::kStencil) && getFormatInfo(view.getTextureImpl().getFormat()).isStencil())
		{
			vkStencilAttachment = {};
			vkStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkStencilAttachment.imageView = view.getHandle();
			vkStencilAttachment.imageLayout = view.getTextureImpl().computeLayout(depthStencilRt->m_usage, 0);
			vkStencilAttachment.loadOp = convertLoadOp(depthStencilRt->m_stencilLoadOperation);
			vkStencilAttachment.storeOp = convertStoreOp(depthStencilRt->m_stencilStoreOperation);
			vkStencilAttachment.clearValue.depthStencil.depth = depthStencilRt->m_clearValue.m_depthStencil.m_depth;
			vkStencilAttachment.clearValue.depthStencil.stencil = depthStencilRt->m_clearValue.m_depthStencil.m_stencil;

			info.pStencilAttachment = &vkStencilAttachment;
		}

		ANKI_ASSERT(fbWidth == 0 || (fbWidth == view.getTextureImpl().getWidth() >> view.getSubresource().m_firstMipmap));
		ANKI_ASSERT(fbHeight == 0 || (fbHeight == view.getTextureImpl().getHeight() >> view.getSubresource().m_firstMipmap));
		fbWidth = view.getTextureImpl().getWidth() >> view.getSubresource().m_firstMipmap;
		fbHeight = view.getTextureImpl().getHeight() >> view.getSubresource().m_firstMipmap;

		dsFormat = view.getTextureImpl().getFormat();
	}

	if(vrsRt)
	{
		vkVrsAttachment = {};
		vkVrsAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
		vkVrsAttachment.imageView = static_cast<const TextureViewImpl&>(*vrsRt).getHandle();
		// Technically it's possible for SRI to be in other layout. Don't bother though
		vkVrsAttachment.imageLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
		vkVrsAttachment.shadingRateAttachmentTexelSize.width = vrsRtTexelSizeX;
		vkVrsAttachment.shadingRateAttachmentTexelSize.height = vrsRtTexelSizeY;

		appendPNextList(info, &vkVrsAttachment);
	}

	info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	info.layerCount = 1;

	// Set the render area
	ANKI_ASSERT(minx < fbWidth && miny < fbHeight);

	const Bool flipViewport = drawsToSwapchain;
	const U32 maxx = min<U32>(minx + width, fbWidth);
	const U32 maxy = min<U32>(miny + height, fbHeight);
	width = maxx - minx;
	height = maxy - miny;
	ANKI_ASSERT(minx + width <= fbWidth && miny + height <= fbHeight);

	info.renderArea.offset.x = minx;
	if(flipViewport)
	{
		ANKI_ASSERT(height <= fbHeight);
	}
	info.renderArea.offset.y = (flipViewport) ? fbHeight - (miny + height) : miny;
	info.renderArea.extent.width = width;
	info.renderArea.extent.height = height;

	// State bookkeeping
	self.m_state.beginRenderPass({colorFormats.getBegin(), colorRts.getSize()}, dsFormat, flipViewport);

	// Re-set the viewport and scissor because sometimes they are set clamped
	self.m_viewportDirty = true;
	self.m_scissorDirty = true;

	self.m_renderpassDrawsToDefaultFb = drawsToSwapchain;
	if(drawsToSwapchain)
	{
		self.m_renderedToDefaultFb = true;
	}

	self.m_renderpassWidth = fbWidth;
	self.m_renderpassHeight = fbHeight;

	// Finaly
	vkCmdBeginRenderingKHR(self.m_handle, &info);
}

void CommandBuffer::endRenderPass()
{
	ANKI_VK_SELF(CommandBufferImpl);

	ANKI_ASSERT(self.m_insideRenderpass);
#if ANKI_ASSERTIONS_ENABLED
	self.m_insideRenderpass = false;
#endif

	self.commandCommon();
	self.m_state.endRenderPass();
	vkCmdEndRenderingKHR(self.m_handle);
}

void CommandBuffer::setVrsRate(VrsRate rate)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_vrs);
	ANKI_ASSERT(rate < VrsRate::kCount);
	self.commandCommon();

	if(self.m_vrsRate != rate)
	{
		self.m_vrsRate = rate;
		self.m_vrsRateDirty = true;
	}
}

void CommandBuffer::drawIndexed(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();
	vkCmdDrawIndexed(self.m_handle, count, instanceCount, firstIndex, baseVertex, baseInstance);
}

void CommandBuffer::draw(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();
	vkCmdDraw(self.m_handle, count, instanceCount, first, baseInstance);
}

void CommandBuffer::drawIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, Buffer* buff)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(DrawIndirectArgs) * drawCount) <= impl.getSize());

	vkCmdDrawIndirect(self.m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawIndirectArgs));
}

void CommandBuffer::drawIndexedIndirectCount(PrimitiveTopology topology, Buffer* argBuffer, PtrSize argBufferOffset, U32 argBufferStride,
											 Buffer* countBuffer, PtrSize countBufferOffset, U32 maxDrawCount)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();
	const BufferImpl& argBufferImpl = static_cast<const BufferImpl&>(*argBuffer);
	ANKI_ASSERT(argBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((argBufferOffset % 4) == 0);
	ANKI_ASSERT(argBufferStride >= sizeof(DrawIndexedIndirectArgs));
	ANKI_ASSERT(argBufferOffset + maxDrawCount * argBufferStride <= argBuffer->getSize());

	const BufferImpl& countBufferImpl = static_cast<const BufferImpl&>(*countBuffer);
	ANKI_ASSERT(countBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((countBufferOffset % 4) == 0);
	ANKI_ASSERT(countBufferOffset + sizeof(U32) <= countBuffer->getSize());

	ANKI_ASSERT(maxDrawCount > 0 && maxDrawCount <= getGrManagerImpl().getDeviceCapabilities().m_maxDrawIndirectCount);

	vkCmdDrawIndexedIndirectCountKHR(self.m_handle, argBufferImpl.getHandle(), argBufferOffset, countBufferImpl.getHandle(), countBufferOffset,
									 maxDrawCount, argBufferStride);
}

void CommandBuffer::drawIndirectCount(PrimitiveTopology topology, Buffer* argBuffer, PtrSize argBufferOffset, U32 argBufferStride,
									  Buffer* countBuffer, PtrSize countBufferOffset, U32 maxDrawCount)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();
	const BufferImpl& argBufferImpl = static_cast<const BufferImpl&>(*argBuffer);
	ANKI_ASSERT(argBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((argBufferOffset % 4) == 0);
	ANKI_ASSERT(argBufferStride >= sizeof(DrawIndirectArgs));
	ANKI_ASSERT(argBufferOffset + maxDrawCount * argBufferStride <= argBuffer->getSize());

	const BufferImpl& countBufferImpl = static_cast<const BufferImpl&>(*countBuffer);
	ANKI_ASSERT(countBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((countBufferOffset % 4) == 0);
	ANKI_ASSERT(countBufferOffset + maxDrawCount * sizeof(U32) <= countBuffer->getSize());

	ANKI_ASSERT(maxDrawCount > 0 && maxDrawCount <= getGrManagerImpl().getDeviceCapabilities().m_maxDrawIndirectCount);

	vkCmdDrawIndirectCountKHR(self.m_handle, argBufferImpl.getHandle(), argBufferOffset, countBufferImpl.getHandle(), countBufferOffset, maxDrawCount,
							  argBufferStride);
}

void CommandBuffer::drawIndexedIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset, Buffer* buff)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((offset % 4) == 0);
	ANKI_ASSERT((offset + sizeof(DrawIndexedIndirectArgs) * drawCount) <= impl.getSize());

	vkCmdDrawIndexedIndirect(self.m_handle, impl.getHandle(), offset, drawCount, sizeof(DrawIndexedIndirectArgs));
}

void CommandBuffer::drawMeshTasks(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));
	self.drawcallCommon();
	vkCmdDrawMeshTasksEXT(self.m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::drawMeshTasksIndirect(Buffer* argBuffer, PtrSize argBufferOffset)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));
	ANKI_ASSERT((argBufferOffset % 4) == 0);
	const BufferImpl& impl = static_cast<const BufferImpl&>(*argBuffer);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((argBufferOffset + sizeof(DispatchIndirectArgs)) <= impl.getSize());

	self.m_state.setPrimitiveTopology(PrimitiveTopology::kTriangles); // Not sure if that's needed
	self.drawcallCommon();
	vkCmdDrawMeshTasksIndirectEXT(self.m_handle, impl.getHandle(), argBufferOffset, 1, sizeof(DispatchIndirectArgs));
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(groupCountX > 0 && groupCountY > 0 && groupCountZ > 0);
	self.dispatchCommon();
	vkCmdDispatch(self.m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::dispatchComputeIndirect(Buffer* argBuffer, PtrSize argBufferOffset)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(argBuffer);
	ANKI_ASSERT(argBufferOffset + sizeof(U32) * 2 < argBuffer->getSize());
	ANKI_ASSERT(argBufferOffset % 4 == 0);
	self.dispatchCommon();
	vkCmdDispatchIndirect(self.m_handle, static_cast<BufferImpl*>(argBuffer)->getHandle(), argBufferOffset);
}

void CommandBuffer::traceRays(Buffer* sbtBuffer, PtrSize sbtBufferOffset, U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount,
							  U32 width, U32 height, U32 depth)
{
	ANKI_VK_SELF(CommandBufferImpl);
	const PtrSize sbtRecordSize = sbtRecordSize32;
	ANKI_ASSERT(hitGroupSbtRecordCount > 0);
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	ANKI_ASSERT(self.m_rtProg);
	const ShaderProgramImpl& sprog = static_cast<const ShaderProgramImpl&>(*self.m_rtProg);
	ANKI_ASSERT(sprog.getReflectionInfo().m_pushConstantsSize == self.m_setPushConstantsSize && "Forgot to set pushConstants");

	ANKI_ASSERT(rayTypeCount == sprog.getMissShaderCount() && "All the miss shaders should be in use");
	ANKI_ASSERT((hitGroupSbtRecordCount % rayTypeCount) == 0);
	const PtrSize sbtRecordCount = 1 + rayTypeCount + hitGroupSbtRecordCount;
	[[maybe_unused]] const PtrSize sbtBufferSize = sbtRecordCount * sbtRecordSize;
	ANKI_ASSERT(sbtBufferSize + sbtBufferOffset <= sbtBuffer->getSize());
	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_sbtRecordAlignment, sbtBufferOffset));

	self.commandCommon();

	// Bind descriptors
	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(sprog.getReflectionInfo().m_descriptorSetMask.get(i))
		{
			VkDescriptorSet dset;
			if(self.m_dsetState[i].flush(self.m_microCmdb->getDSAllocator(), dset))
			{
				vkCmdBindDescriptorSets(self.m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, sprog.getPipelineLayout().getHandle(), i, 1, &dset, 0,
										nullptr);
			}
		}
	}

	Array<VkStridedDeviceAddressRegionKHR, 4> regions;
	const U64 stbBufferAddress = sbtBuffer->getGpuAddress() + sbtBufferOffset;
	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_sbtRecordAlignment, stbBufferAddress));

	// Rgen
	regions[0].deviceAddress = stbBufferAddress;
	regions[0].stride = sbtRecordSize;
	regions[0].size = sbtRecordSize;

	// Miss
	regions[1].deviceAddress = regions[0].deviceAddress + regions[0].size;
	regions[1].stride = sbtRecordSize;
	regions[1].size = sbtRecordSize * rayTypeCount;

	// Hit
	regions[2].deviceAddress = regions[1].deviceAddress + regions[1].size;
	regions[2].stride = sbtRecordSize * rayTypeCount;
	regions[2].size = sbtRecordSize * hitGroupSbtRecordCount;

	// Callable, nothing for now
	regions[3] = VkStridedDeviceAddressRegionKHR();

	vkCmdTraceRaysKHR(self.m_handle, &regions[0], &regions[1], &regions[2], &regions[3], width, height, depth);
}

void CommandBuffer::generateMipmaps2d(TextureView* texView)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();
	ANKI_ASSERT(tex.getTextureType() != TextureType::k3D && "Not for 3D");
	ANKI_ASSERT(tex.isSubresourceGoodForMipmapGeneration(view.getSubresource()));

	const U32 blitCount = tex.getMipmapCount() - 1u;
	if(blitCount == 0)
	{
		// Nothing to be done, flush the previous commands though because you may batch (and sort) things you shouldn't
		return;
	}

	const DepthStencilAspectBit aspect = view.getSubresource().m_depthStencilAspect;
	const U32 face = view.getSubresource().m_firstFace;
	const U32 layer = view.getSubresource().m_firstLayer;

	for(U32 i = 0; i < blitCount; ++i)
	{
		// Transition source
		// OPT: Combine the 2 barriers
		if(i > 0)
		{
			VkImageSubresourceRange range;
			tex.computeVkImageSubresourceRange(TextureSubresourceInfo(TextureSurfaceInfo(i, face, layer), aspect), range);

			self.setImageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex.m_imageHandle,
								 range);
		}

		// Transition destination
		{
			VkImageSubresourceRange range;
			tex.computeVkImageSubresourceRange(TextureSubresourceInfo(TextureSurfaceInfo(i + 1, face, layer), aspect), range);

			self.setImageBarrier(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, tex.m_imageHandle, range);
		}

		// Setup the blit struct
		I32 srcWidth = tex.getWidth() >> i;
		I32 srcHeight = tex.getHeight() >> i;

		I32 dstWidth = tex.getWidth() >> (i + 1);
		I32 dstHeight = tex.getHeight() >> (i + 1);

		ANKI_ASSERT(srcWidth > 0 && srcHeight > 0 && dstWidth > 0 && dstHeight > 0);

		U32 vkLayer = 0;
		switch(tex.getTextureType())
		{
		case TextureType::k2D:
		case TextureType::k2DArray:
			break;
		case TextureType::kCube:
			vkLayer = face;
			break;
		case TextureType::kCubeArray:
			vkLayer = layer * 6 + face;
			break;
		default:
			ANKI_ASSERT(0);
			break;
		}

		VkImageBlit blit;
		blit.srcSubresource.aspectMask = convertImageAspect(aspect);
		blit.srcSubresource.baseArrayLayer = vkLayer;
		blit.srcSubresource.layerCount = 1;
		blit.srcSubresource.mipLevel = i;
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {srcWidth, srcHeight, 1};

		blit.dstSubresource.aspectMask = convertImageAspect(aspect);
		blit.dstSubresource.baseArrayLayer = vkLayer;
		blit.dstSubresource.layerCount = 1;
		blit.dstSubresource.mipLevel = i + 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {dstWidth, dstHeight, 1};

		vkCmdBlitImage(self.m_handle, tex.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex.m_imageHandle,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, (!!aspect) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);
	}
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
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();

	VkClearColorValue vclear;
	static_assert(sizeof(vclear) == sizeof(clearValue), "See file");
	memcpy(&vclear, &clearValue, sizeof(clearValue));

	if(!view.getSubresource().m_depthStencilAspect)
	{
		VkImageSubresourceRange vkRange = view.getVkImageSubresourceRange();
		vkCmdClearColorImage(self.m_handle, tex.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vclear, 1, &vkRange);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

void CommandBuffer::copyBufferToTextureView(Buffer* buff, PtrSize offset, [[maybe_unused]] PtrSize range, TextureView* texView)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*texView);
	const TextureImpl& tex = view.getTextureImpl();
	ANKI_ASSERT(tex.usageValid(TextureUsageBit::kTransferDestination));
	ANKI_ASSERT(tex.isSubresourceGoodForCopyFromBuffer(view.getSubresource()));
	const VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	const Bool is3D = tex.getTextureType() == TextureType::k3D;
	const VkImageAspectFlags aspect = convertImageAspect(view.getSubresource().m_depthStencilAspect);

	const TextureSurfaceInfo surf(view.getSubresource().m_firstMipmap, view.getSubresource().m_firstFace, view.getSubresource().m_firstLayer);
	const TextureVolumeInfo vol(view.getSubresource().m_firstMipmap);

	// Compute the sizes of the mip
	const U32 width = tex.getWidth() >> surf.m_level;
	const U32 height = tex.getHeight() >> surf.m_level;
	ANKI_ASSERT(width && height);
	const U32 depth = (is3D) ? (tex.getDepth() >> surf.m_level) : 1u;

	if(!is3D)
	{
		ANKI_ASSERT(range == computeSurfaceSize(width, height, tex.getFormat()));
	}
	else
	{
		ANKI_ASSERT(range == computeVolumeSize(width, height, depth, tex.getFormat()));
	}

	// Copy
	VkBufferImageCopy region;
	region.imageSubresource.aspectMask = aspect;
	region.imageSubresource.baseArrayLayer = (is3D) ? tex.computeVkArrayLayer(vol) : tex.computeVkArrayLayer(surf);
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = surf.m_level;
	region.imageOffset = {0, 0, 0};
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = depth;
	region.bufferOffset = offset;
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;

	vkCmdCopyBufferToImage(self.m_handle, static_cast<const BufferImpl&>(*buff).getHandle(), tex.m_imageHandle, layout, 1, &region);
}

void CommandBuffer::fillBuffer(Buffer* buff, PtrSize offset, PtrSize size, U32 value)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	ANKI_ASSERT(!self.m_insideRenderpass);
	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kTransferDestination));

	ANKI_ASSERT(offset < impl.getSize());
	ANKI_ASSERT((offset % 4) == 0 && "Should be multiple of 4");

	size = (size == kMaxPtrSize) ? (impl.getActualSize() - offset) : size;
	alignRoundUp(4, size); // Needs to be multiple of 4
	ANKI_ASSERT(offset + size <= impl.getActualSize());
	ANKI_ASSERT((size % 4) == 0 && "Should be multiple of 4");

	vkCmdFillBuffer(self.m_handle, impl.getHandle(), offset, size, value);
}

void CommandBuffer::writeOcclusionQueriesResultToBuffer(ConstWeakArray<OcclusionQuery*> queries, PtrSize offset, Buffer* buff)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(queries.getSize() > 0);
	self.commandCommon();
	ANKI_ASSERT(!self.m_insideRenderpass);

	const BufferImpl& impl = static_cast<const BufferImpl&>(*buff);
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kTransferDestination));

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		ANKI_ASSERT(queries[i]);
		ANKI_ASSERT((offset % 4) == 0);
		ANKI_ASSERT((offset + sizeof(U32)) <= impl.getSize());

		OcclusionQueryImpl* q = static_cast<OcclusionQueryImpl*>(queries[i]);

		vkCmdCopyQueryPoolResults(self.m_handle, q->m_handle.getQueryPool(), q->m_handle.getQueryIndex(), 1, impl.getHandle(), offset, sizeof(U32),
								  VK_QUERY_RESULT_PARTIAL_BIT);

		offset += sizeof(U32);
		self.m_microCmdb->pushObjectRef(q);
	}
}

void CommandBuffer::copyBufferToBuffer(Buffer* src, Buffer* dst, ConstWeakArray<CopyBufferToBufferInfo> copies)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(static_cast<const BufferImpl&>(*src).usageValid(BufferUsageBit::kTransferSource));
	ANKI_ASSERT(static_cast<const BufferImpl&>(*dst).usageValid(BufferUsageBit::kTransferDestination));
	ANKI_ASSERT(copies.getSize() > 0);

	self.commandCommon();

	static_assert(sizeof(CopyBufferToBufferInfo) == sizeof(VkBufferCopy));
	const VkBufferCopy* vkCopies = reinterpret_cast<const VkBufferCopy*>(&copies[0]);

	vkCmdCopyBuffer(self.m_handle, static_cast<const BufferImpl&>(*src).getHandle(), static_cast<const BufferImpl&>(*dst).getHandle(),
					copies.getSize(), &vkCopies[0]);
}

void CommandBuffer::buildAccelerationStructure(AccelerationStructure* as, Buffer* scratchBuffer, PtrSize scratchBufferOffset)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(as && scratchBuffer);
	ANKI_ASSERT(as->getBuildScratchBufferSize() + scratchBufferOffset <= scratchBuffer->getSize());

	self.commandCommon();

	// Get objects
	const AccelerationStructureImpl& asImpl = static_cast<AccelerationStructureImpl&>(*as);

	// Create the build info
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo;
	VkAccelerationStructureBuildRangeInfoKHR rangeInfo;
	asImpl.generateBuildInfo(scratchBuffer->getGpuAddress() + scratchBufferOffset, buildInfo, rangeInfo);

	// Run the command
	Array<const VkAccelerationStructureBuildRangeInfoKHR*, 1> pRangeInfos = {&rangeInfo};
	vkCmdBuildAccelerationStructuresKHR(self.m_handle, 1, &buildInfo, &pRangeInfos[0]);
}

#if ANKI_DLSS
/// Utility function to get the NGX's resource structure for a texture
/// @param[in] tex the texture to generate the NVSDK_NGX_Resource_VK from
static NVSDK_NGX_Resource_VK getNGXResourceFromAnkiTexture(const TextureViewImpl& view)
{
	const TextureImpl& tex = view.getTextureImpl();

	const VkImageView imageView = view.getHandle();
	const VkFormat format = tex.m_vkFormat;
	const VkImage image = tex.m_imageHandle;
	const VkImageSubresourceRange subresourceRange = view.getVkImageSubresourceRange();
	const Bool isUAV = !!(tex.m_vkUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT);

	// TODO Not sure if I should pass the width,height of the image or the view
	return NVSDK_NGX_Create_ImageView_Resource_VK(imageView, image, subresourceRange, format, tex.getWidth(), tex.getHeight(), isUAV);
}
#endif

void CommandBuffer::upscale(GrUpscaler* upscaler, TextureView* inColor, TextureView* outUpscaledColor, TextureView* motionVectors, TextureView* depth,
							TextureView* exposure, Bool resetAccumulation, const Vec2& jitterOffset, const Vec2& motionVectorsScale)
{
#if ANKI_DLSS
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_dlss);
	ANKI_ASSERT(upscaler->getUpscalerType() == GrUpscalerType::kDlss2);

	self.commandCommon();

	const GrUpscalerImpl& upscalerImpl = static_cast<const GrUpscalerImpl&>(*upscaler);

	const TextureViewImpl& srcViewImpl = static_cast<const TextureViewImpl&>(*inColor);
	const TextureViewImpl& dstViewImpl = static_cast<const TextureViewImpl&>(*outUpscaledColor);
	const TextureViewImpl& mvViewImpl = static_cast<const TextureViewImpl&>(*motionVectors);
	const TextureViewImpl& depthViewImpl = static_cast<const TextureViewImpl&>(*depth);
	const TextureViewImpl& exposureViewImpl = static_cast<const TextureViewImpl&>(*exposure);

	NVSDK_NGX_Resource_VK srcResVk = getNGXResourceFromAnkiTexture(srcViewImpl);
	NVSDK_NGX_Resource_VK dstResVk = getNGXResourceFromAnkiTexture(dstViewImpl);
	NVSDK_NGX_Resource_VK mvResVk = getNGXResourceFromAnkiTexture(mvViewImpl);
	NVSDK_NGX_Resource_VK depthResVk = getNGXResourceFromAnkiTexture(depthViewImpl);
	NVSDK_NGX_Resource_VK exposureResVk = getNGXResourceFromAnkiTexture(exposureViewImpl);

	const U32 mipLevel = srcViewImpl.getSubresource().m_firstMipmap;
	const NVSDK_NGX_Coordinates renderingOffset = {0, 0};
	const NVSDK_NGX_Dimensions renderingSize = {srcViewImpl.getTextureImpl().getWidth() >> mipLevel,
												srcViewImpl.getTextureImpl().getHeight() >> mipLevel};

	NVSDK_NGX_VK_DLSS_Eval_Params vkDlssEvalParams;
	memset(&vkDlssEvalParams, 0, sizeof(vkDlssEvalParams));
	vkDlssEvalParams.Feature.pInColor = &srcResVk;
	vkDlssEvalParams.Feature.pInOutput = &dstResVk;
	vkDlssEvalParams.pInDepth = &depthResVk;
	vkDlssEvalParams.pInMotionVectors = &mvResVk;
	vkDlssEvalParams.pInExposureTexture = &exposureResVk;
	vkDlssEvalParams.InJitterOffsetX = jitterOffset.x();
	vkDlssEvalParams.InJitterOffsetY = jitterOffset.y();
	vkDlssEvalParams.InReset = resetAccumulation;
	vkDlssEvalParams.InMVScaleX = motionVectorsScale.x();
	vkDlssEvalParams.InMVScaleY = motionVectorsScale.y();
	vkDlssEvalParams.InColorSubrectBase = renderingOffset;
	vkDlssEvalParams.InDepthSubrectBase = renderingOffset;
	vkDlssEvalParams.InTranslucencySubrectBase = renderingOffset;
	vkDlssEvalParams.InMVSubrectBase = renderingOffset;
	vkDlssEvalParams.InRenderSubrectDimensions = renderingSize;

	NVSDK_NGX_Parameter* dlssParameters = &upscalerImpl.getParameters();
	NVSDK_NGX_Handle* dlssFeature = &upscalerImpl.getFeature();
	const NVSDK_NGX_Result result = NGX_VULKAN_EVALUATE_DLSS_EXT(self.m_handle, dlssFeature, dlssParameters, &vkDlssEvalParams);

	if(NVSDK_NGX_FAILED(result))
	{
		ANKI_VK_LOGF("Failed to NVSDK_NGX_VULKAN_EvaluateFeature for DLSS, code = 0x%08x, info: %ls", result, GetNGXResultAsString(result));
	}
#else
	ANKI_ASSERT(0 && "Not supported");
	(void)upscaler;
	(void)inColor;
	(void)outUpscaledColor;
	(void)motionVectors;
	(void)depth;
	(void)exposure;
	(void)resetAccumulation;
	(void)jitterOffset;
	(void)motionVectorsScale;
#endif
}

void CommandBuffer::setPipelineBarrier(ConstWeakArray<TextureBarrierInfo> textures, ConstWeakArray<BufferBarrierInfo> buffers,
									   ConstWeakArray<AccelerationStructureBarrierInfo> accelerationStructures)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	DynamicArray<VkImageMemoryBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> imageBarriers(self.m_pool);
	DynamicArray<VkBufferMemoryBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> bufferBarriers(self.m_pool);
	DynamicArray<VkMemoryBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> genericBarriers(self.m_pool);
	VkPipelineStageFlags srcStageMask = 0;
	VkPipelineStageFlags dstStageMask = 0;

	for(const TextureBarrierInfo& barrier : textures)
	{
		ANKI_ASSERT(barrier.m_texture);
		const TextureImpl& impl = static_cast<const TextureImpl&>(*barrier.m_texture);
		const TextureUsageBit nextUsage = barrier.m_nextUsage;
		const TextureUsageBit prevUsage = barrier.m_previousUsage;
		TextureSubresourceInfo subresource = barrier.m_subresource;

		ANKI_ASSERT(impl.usageValid(prevUsage));
		ANKI_ASSERT(impl.usageValid(nextUsage));
		ANKI_ASSERT(((nextUsage & TextureUsageBit::kGenerateMipmaps) == TextureUsageBit::kGenerateMipmaps
					 || (nextUsage & TextureUsageBit::kGenerateMipmaps) == TextureUsageBit::kNone)
					&& "GENERATE_MIPMAPS should be alone");
		ANKI_ASSERT(impl.isSubresourceValid(subresource));

		if(subresource.m_firstMipmap > 0 && nextUsage == TextureUsageBit::kGenerateMipmaps) [[unlikely]]
		{
			// This transition happens inside CommandBufferImpl::generateMipmapsX. No need to do something
			continue;
		}

		if(nextUsage == TextureUsageBit::kGenerateMipmaps) [[unlikely]]
		{
			// The transition of the non zero mip levels happens inside CommandBufferImpl::generateMipmapsX so limit the subresource

			ANKI_ASSERT(subresource.m_firstMipmap == 0 && subresource.m_mipmapCount == 1);
		}

		VkImageMemoryBarrier& inf = *imageBarriers.emplaceBack();
		inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		inf.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		inf.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		inf.image = impl.m_imageHandle;

		impl.computeVkImageSubresourceRange(subresource, inf.subresourceRange);

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		impl.computeBarrierInfo(prevUsage, nextUsage, inf.subresourceRange.baseMipLevel, srcStage, inf.srcAccessMask, dstStage, inf.dstAccessMask);
		inf.oldLayout = impl.computeLayout(prevUsage, inf.subresourceRange.baseMipLevel);
		inf.newLayout = impl.computeLayout(nextUsage, inf.subresourceRange.baseMipLevel);

		srcStageMask |= srcStage;
		dstStageMask |= dstStage;
	}

	for(const BufferBarrierInfo& barrier : buffers)
	{
		ANKI_ASSERT(barrier.m_buffer);
		const BufferImpl& impl = static_cast<const BufferImpl&>(*barrier.m_buffer);

		const VkBuffer handle = impl.getHandle();
		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		VkAccessFlags srcAccessMask;
		VkAccessFlags dstAccessMask;
		impl.computeBarrierInfo(barrier.m_previousUsage, barrier.m_nextUsage, srcStage, srcAccessMask, dstStage, dstAccessMask);

		srcStageMask |= srcStage;
		dstStageMask |= dstStage;

		if(bufferBarriers.getSize() && bufferBarriers.getBack().buffer == handle)
		{
			// Merge barriers
			bufferBarriers.getBack().srcAccessMask |= srcAccessMask;
			bufferBarriers.getBack().dstAccessMask |= dstAccessMask;
		}
		else
		{
			// Create a new buffer barrier
			VkBufferMemoryBarrier& inf = *bufferBarriers.emplaceBack();
			inf = {};
			inf.buffer = handle;
			inf.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			inf.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			inf.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			inf.srcAccessMask = srcAccessMask;
			inf.dstAccessMask = dstAccessMask;
			inf.offset = 0;
			inf.size = VK_WHOLE_SIZE; // All size because we don't care
		}
	}

	for(const AccelerationStructureBarrierInfo& barrier : accelerationStructures)
	{
		ANKI_ASSERT(barrier.m_as);

		VkMemoryBarrier& inf = *genericBarriers.emplaceBack();
		inf = {};
		inf.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		AccelerationStructureImpl::computeBarrierInfo(barrier.m_previousUsage, barrier.m_nextUsage, srcStage, inf.srcAccessMask, dstStage,
													  inf.dstAccessMask);

		srcStageMask |= srcStage;
		dstStageMask |= dstStage;

		self.m_microCmdb->pushObjectRef(barrier.m_as);
	}

	vkCmdPipelineBarrier(self.m_handle, srcStageMask, dstStageMask, 0, genericBarriers.getSize(),
						 (genericBarriers.getSize()) ? &genericBarriers[0] : nullptr, bufferBarriers.getSize(),
						 (bufferBarriers.getSize()) ? &bufferBarriers[0] : nullptr, imageBarriers.getSize(),
						 (imageBarriers.getSize()) ? &imageBarriers[0] : nullptr);

	ANKI_TRACE_INC_COUNTER(VkBarrier, 1);
}

void CommandBuffer::resetOcclusionQueries(ConstWeakArray<OcclusionQuery*> queries)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(queries.getSize() > 0);

	self.commandCommon();

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		OcclusionQuery* query = queries[i];
		ANKI_ASSERT(query);
		const VkQueryPool poolHandle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
		const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();

		vkCmdResetQueryPool(self.m_handle, poolHandle, idx, 1);
		self.m_microCmdb->pushObjectRef(query);
	}
}

void CommandBuffer::beginOcclusionQuery(OcclusionQuery* query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);

	vkCmdBeginQuery(self.m_handle, handle, idx, 0);

	self.m_microCmdb->pushObjectRef(query);
}

void CommandBuffer::endOcclusionQuery(OcclusionQuery* query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);

	vkCmdEndQuery(self.m_handle, handle, idx);

	self.m_microCmdb->pushObjectRef(query);
}

void CommandBuffer::beginPipelineQuery(PipelineQuery* query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const VkQueryPool handle = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);
	vkCmdBeginQuery(self.m_handle, handle, idx, 0);
	self.m_microCmdb->pushObjectRef(query);
}

void CommandBuffer::endPipelineQuery(PipelineQuery* query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const VkQueryPool handle = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);
	vkCmdEndQuery(self.m_handle, handle, idx);
	self.m_microCmdb->pushObjectRef(query);
}

void CommandBuffer::resetTimestampQueries(ConstWeakArray<TimestampQuery*> queries)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(queries.getSize() > 0);

	self.commandCommon();

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		TimestampQuery* query = queries[i];
		ANKI_ASSERT(query);
		const VkQueryPool poolHandle = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryPool();
		const U32 idx = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryIndex();

		vkCmdResetQueryPool(self.m_handle, poolHandle, idx, 1);
		self.m_microCmdb->pushObjectRef(query);
	}
}

void CommandBuffer::writeTimestamp(TimestampQuery* query)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkQueryPool handle = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryIndex();

	vkCmdWriteTimestamp(self.m_handle, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, handle, idx);

	self.m_microCmdb->pushObjectRef(query);
}

Bool CommandBuffer::isEmpty() const
{
	ANKI_VK_SELF_CONST(CommandBufferImpl);
	return self.isEmpty();
}

void CommandBuffer::setPushConstants(const void* data, U32 dataSize)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(data && dataSize && dataSize % 16 == 0);
	const ShaderProgramImpl& prog = self.getBoundProgram();
	ANKI_ASSERT(prog.getReflectionInfo().m_pushConstantsSize == dataSize
				&& "The bound program should have push constants equal to the \"dataSize\" parameter");

	self.commandCommon();

	vkCmdPushConstants(self.m_handle, prog.getPipelineLayout().getHandle(), VK_SHADER_STAGE_ALL, 0, dataSize, data);

#if ANKI_EXTRA_CHECKS
	self.m_setPushConstantsSize = dataSize;
#endif
}

void CommandBuffer::setRasterizationOrder(RasterizationOrder order)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kAMD_rasterization_order))
	{
		self.m_state.setRasterizationOrder(order);
	}
}

void CommandBuffer::setLineWidth(F32 width)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	vkCmdSetLineWidth(self.m_handle, width);

#if ANKI_ASSERTIONS_ENABLED
	self.m_lineWidthSet = true;
#endif
}

void CommandBuffer::pushDebugMarker(CString name, Vec3 color)
{
	ANKI_VK_SELF(CommandBufferImpl);
	if(self.m_debugMarkers) [[unlikely]]
	{
		self.commandCommon();

		VkDebugUtilsLabelEXT label = {};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pLabelName = name.cstr();
		label.color[0] = color[0];
		label.color[1] = color[1];
		label.color[2] = color[2];
		label.color[3] = 1.0f;
		vkCmdBeginDebugUtilsLabelEXT(self.m_handle, &label);
	}

#if ANKI_EXTRA_CHECKS
	++self.m_debugMarkersPushed;
#endif
}

void CommandBuffer::popDebugMarker()
{
	ANKI_VK_SELF(CommandBufferImpl);
	if(self.m_debugMarkers) [[unlikely]]
	{
		self.commandCommon();
		vkCmdEndDebugUtilsLabelEXT(self.m_handle);
	}

#if ANKI_EXTRA_CHECKS
	ANKI_ASSERT(self.m_debugMarkersPushed > 0);
	--self.m_debugMarkersPushed;
#endif
}

CommandBufferImpl::~CommandBufferImpl()
{
	if(m_empty)
	{
		ANKI_VK_LOGW("Command buffer was empty");
	}

	if(!m_finalized)
	{
		ANKI_VK_LOGW("Command buffer was not flushed");
	}

#if ANKI_EXTRA_CHECKS
	ANKI_ASSERT(m_debugMarkersPushed == 0);

	if(!m_submitted)
	{
		ANKI_VK_LOGW("Command buffer not submitted");
	}
#endif
}

Error CommandBufferImpl::init(const CommandBufferInitInfo& init)
{
	m_tid = Thread::getCurrentThreadId();
	m_flags = init.m_flags;

	ANKI_CHECK(CommandBufferFactory::getSingleton().newCommandBuffer(m_tid, m_flags, m_microCmdb));
	m_handle = m_microCmdb->getHandle();

	m_pool = &m_microCmdb->getFastMemoryPool();

	for(DSStateTracker& state : m_dsetState)
	{
		state.init(m_pool);
	}

	m_state.setVrsCapable(getGrManagerImpl().getDeviceCapabilities().m_vrs);

	m_debugMarkers = !!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_debug_utils);

	return Error::kNone;
}

void CommandBufferImpl::setImageBarrier(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkImageLayout prevLayout,
										VkPipelineStageFlags dstStage, VkAccessFlags dstAccess, VkImageLayout newLayout, VkImage img,
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
	ANKI_TRACE_INC_COUNTER(VkBarrier, 1);
}

void CommandBufferImpl::beginRecording()
{
	// Do the begin
	VkCommandBufferInheritanceInfo inheritance = {};
	inheritance.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	VkCommandBufferBeginInfo begin = {};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin.pInheritanceInfo = &inheritance;

	vkBeginCommandBuffer(m_handle, &begin);

	// Stats
	if(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kKHR_pipeline_executable_properties))
	{
		m_state.setEnablePipelineStatistics(true);
	}
}

void CommandBufferImpl::endRecording()
{
	commandCommon();

	ANKI_ASSERT(!m_finalized);
	ANKI_ASSERT(!m_empty);

	ANKI_VK_CHECKF(vkEndCommandBuffer(m_handle));
	m_finalized = true;

#if ANKI_EXTRA_CHECKS
	static Atomic<U32> messagePrintCount(0);
	constexpr U32 MAX_PRINT_COUNT = 10;

	CString message;
	if(!!(m_flags & CommandBufferFlag::kSmallBatch))
	{
		if(m_commandCount > kCommandBufferSmallBatchMaxCommands * 4)
		{
			message = "Command buffer has too many commands%s: %u";
		}
	}
	else
	{
		if(m_commandCount <= kCommandBufferSmallBatchMaxCommands / 4)
		{
			message = "Command buffer has too few commands%s: %u";
		}
	}

	if(!message.isEmpty())
	{
		const U32 count = messagePrintCount.fetchAdd(1) + 1;
		if(count < MAX_PRINT_COUNT)
		{
			ANKI_VK_LOGW(message.cstr(), "", m_commandCount);
		}
		else if(count == MAX_PRINT_COUNT)
		{
			ANKI_VK_LOGW(message.cstr(), " (will ignore further warnings)", m_commandCount);
		}
	}
#endif
}

void CommandBufferImpl::drawcallCommon()
{
	// Preconditions
	commandCommon();
	ANKI_ASSERT(m_graphicsProg);
	ANKI_ASSERT(m_insideRenderpass);
	ANKI_ASSERT(m_graphicsProg->getReflectionInfo().m_pushConstantsSize == m_setPushConstantsSize && "Forgot to set pushConstants");

	// Get or create ppline
	Pipeline ppline;
	Bool stateDirty;
	m_graphicsProg->getPipelineFactory().getOrCreatePipeline(m_state, ppline, stateDirty);

	if(stateDirty)
	{
		vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ppline.getHandle());
	}

	// Bind dsets
	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(m_graphicsProg->getReflectionInfo().m_descriptorSetMask.get(i))
		{
			VkDescriptorSet dset;
			if(m_dsetState[i].flush(m_microCmdb->getDSAllocator(), dset))
			{
				vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsProg->getPipelineLayout().getHandle(), i, 1, &dset, 0,
										nullptr);
			}
		}
	}

	// Flush viewport
	if(m_viewportDirty) [[unlikely]]
	{
		const Bool flipvp = m_renderpassDrawsToDefaultFb;
		VkViewport vp = computeViewport(&m_viewport[0], m_renderpassWidth, m_renderpassHeight, flipvp);

		// Additional optimization
		if(memcmp(&vp, &m_lastViewport, sizeof(vp)) != 0)
		{
			vkCmdSetViewport(m_handle, 0, 1, &vp);
			m_lastViewport = vp;
		}

		m_viewportDirty = false;
	}

	// Flush scissor
	if(m_scissorDirty) [[unlikely]]
	{
		const Bool flipvp = m_renderpassDrawsToDefaultFb;
		VkRect2D scissor = computeScissor(&m_scissor[0], m_renderpassWidth, m_renderpassHeight, flipvp);

		// Additional optimization
		if(memcmp(&scissor, &m_lastScissor, sizeof(scissor)) != 0)
		{
			vkCmdSetScissor(m_handle, 0, 1, &scissor);
			m_lastScissor = scissor;
		}

		m_scissorDirty = false;
	}

	// VRS
	if(getGrManagerImpl().getDeviceCapabilities().m_vrs && m_vrsRateDirty)
	{
		const VkExtent2D extend = convertVrsShadingRate(m_vrsRate);
		Array<VkFragmentShadingRateCombinerOpKHR, 2> combiner;
		combiner[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR; // Keep pipeline rating over primitive
		combiner[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR; // Max of attachment and pipeline rates
		vkCmdSetFragmentShadingRateKHR(m_handle, &extend, &combiner[0]);

		m_vrsRateDirty = false;
	}

	// Some checks
#if ANKI_ASSERTIONS_ENABLED
	if(m_state.getPrimitiveTopology() == PrimitiveTopology::kLines || m_state.getPrimitiveTopology() == PrimitiveTopology::kLineStip)
	{
		ANKI_ASSERT(m_lineWidthSet == true);
	}
#endif

	ANKI_TRACE_INC_COUNTER(VkDrawcall, 1);
}

ANKI_FORCE_INLINE void CommandBufferImpl::dispatchCommon()
{
	ANKI_ASSERT(m_computeProg);
	ANKI_ASSERT(m_computeProg->getReflectionInfo().m_pushConstantsSize == m_setPushConstantsSize && "Forgot to set pushConstants");

	commandCommon();

	// Bind descriptors
	for(U32 i = 0; i < kMaxDescriptorSets; ++i)
	{
		if(m_computeProg->getReflectionInfo().m_descriptorSetMask.get(i))
		{
			VkDescriptorSet dset;
			if(m_dsetState[i].flush(m_microCmdb->getDSAllocator(), dset))
			{
				vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, m_computeProg->getPipelineLayout().getHandle(), i, 1, &dset, 0,
										nullptr);
			}
		}
	}
}

} // end namespace anki

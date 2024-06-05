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
#include <AnKi/Gr/Vulkan/VkSampler.h>
#include <AnKi/Gr/Vulkan/VkAccelerationStructure.h>

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

void CommandBuffer::bindVertexBuffer(U32 binding, const BufferView& buff, U32 stride, VertexStepRate stepRate)
{
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.bindVertexBuffer(binding, stride, stepRate);
	const VkBuffer vkbuff = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();
	vkCmdBindVertexBuffers(self.m_handle, binding, 1, &vkbuff, &buff.getOffset());
}

void CommandBuffer::setVertexAttribute(VertexAttributeSemantic attribute, U32 buffBinding, Format fmt, U32 relativeOffset)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_state.setVertexAttribute(attribute, buffBinding, fmt, relativeOffset);
}

void CommandBuffer::bindIndexBuffer(const BufferView& buff, IndexType type)
{
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const BufferImpl& buffi = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(!!(buffi.getBufferUsage() & BufferUsageBit::kIndex));
	vkCmdBindIndexBuffer(self.m_handle, buffi.getHandle(), buff.getOffset(), convertIndexType(type));
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

void CommandBuffer::bindTexture(Register reg, const TextureView& texView)
{
	reg.validate();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());

	if(reg.m_resourceType == HlslResourceType::kSrv)
	{
		ANKI_ASSERT(texView.isGoodForSampling());
		const VkImageLayout lay = tex.computeLayout(TextureUsageBit::kAllSampled & tex.getTextureUsage(), 0);
		self.m_descriptorState.bindSampledTexture(reg.m_space, reg.m_bindPoint, tex.getImageView(texView.getSubresource()), lay);
	}
	else
	{
		ANKI_ASSERT(texView.isGoodForStorage());
		self.m_descriptorState.bindStorageTexture(reg.m_space, reg.m_bindPoint, tex.getImageView(texView.getSubresource()));

		const Bool isPresentable = !!(tex.getTextureUsage() & TextureUsageBit::kPresent);
		if(isPresentable)
		{
			self.m_renderedToDefaultFb = true;
		}
	}
}

void CommandBuffer::bindSampler(Register reg, Sampler* sampler)
{
	reg.validate();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkSampler handle = static_cast<const SamplerImpl&>(*sampler).m_sampler->getHandle();
	self.m_descriptorState.bindSampler(reg.m_space, reg.m_bindPoint, handle);
	self.m_microCmdb->pushObjectRef(sampler);
}

void CommandBuffer::bindUniformBuffer(Register reg, const BufferView& buff)
{
	reg.validate();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkBuffer handle = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();
	self.m_descriptorState.bindUniformBuffer(reg.m_space, reg.m_bindPoint, handle, buff.getOffset(), buff.getRange());
}

void CommandBuffer::bindStorageBuffer(Register reg, const BufferView& buff)
{
	reg.validate();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkBuffer handle = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();
	if(reg.m_resourceType == HlslResourceType::kSrv)
	{
		self.m_descriptorState.bindReadStorageBuffer(reg.m_space, reg.m_bindPoint, handle, buff.getOffset(), buff.getRange());
	}
	else
	{
		self.m_descriptorState.bindReadWriteStorageBuffer(reg.m_space, reg.m_bindPoint, handle, buff.getOffset(), buff.getRange());
	}
}

void CommandBuffer::bindAccelerationStructure(Register reg, AccelerationStructure* as)
{
	reg.validate();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkAccelerationStructureKHR& handle = static_cast<const AccelerationStructureImpl&>(*as).getHandle();
	self.m_descriptorState.bindAccelerationStructure(reg.m_space, reg.m_bindPoint, &handle);
	self.m_microCmdb->pushObjectRef(as);
}

void CommandBuffer::bindTexelBuffer(Register reg, const BufferView& buff, Format fmt)
{
	reg.validate();
	ANKI_ASSERT(fmt != Format::kNone);
	reg.validate();

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkBufferView view = static_cast<const BufferImpl&>(buff.getBuffer()).getOrCreateBufferView(fmt, buff.getOffset(), buff.getRange());

	if(reg.m_resourceType == HlslResourceType::kSrv)
	{
		self.m_descriptorState.bindReadTexelBuffer(reg.m_space, reg.m_bindPoint, view);
	}
	else
	{
		ANKI_ASSERT(reg.m_resourceType == HlslResourceType::kUav);
		self.m_descriptorState.bindReadWriteTexelBuffer(reg.m_space, reg.m_bindPoint, view);
	}
}

void CommandBuffer::bindShaderProgram(ShaderProgram* prog)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	ShaderProgramImpl& impl = static_cast<ShaderProgramImpl&>(*prog);
	VkPipelineBindPoint bindPoint;

	if(impl.isGraphics())
	{
		self.m_graphicsProg = &impl;
		self.m_computeProg = nullptr; // Unbind the compute prog. Doesn't work like vulkan
		self.m_rtProg = nullptr; // See above
		self.m_state.bindShaderProgram(&impl);

		bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}
	else if(!!(impl.getStages() & ShaderTypeBit::kCompute))
	{
		self.m_computeProg = &impl;
		self.m_graphicsProg = nullptr; // See comment in the if()
		self.m_rtProg = nullptr; // See above

		// Bind the pipeline now
		vkCmdBindPipeline(self.m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, impl.getComputePipelineHandle());

		bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	}
	else
	{
		ANKI_ASSERT(!!(impl.getStages() & ShaderTypeBit::kAllRayTracing));
		self.m_computeProg = nullptr;
		self.m_graphicsProg = nullptr;
		self.m_rtProg = &impl;

		// Bind now
		vkCmdBindPipeline(self.m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, impl.getRayTracingPipelineHandle());

		bindPoint = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
	}

	self.m_descriptorState.setPipelineLayout(&impl.getPipelineLayout(), bindPoint);

	self.m_microCmdb->pushObjectRef(prog);
}

void CommandBuffer::beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, U32 minx, U32 miny, U32 width, U32 height,
									const TextureView& vrsRt, U8 vrsRtTexelSizeX, U8 vrsRtTexelSizeY)
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
		const TextureView& view = colorRts[i].m_textureView;
		ANKI_ASSERT(!view.getDepthStencilAspect());
		const TextureImpl& tex = static_cast<const TextureImpl&>(view.getTexture());

		vkColorAttachments[i] = {};
		vkColorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		vkColorAttachments[i].imageView = tex.getImageView(view.getSubresource());
		vkColorAttachments[i].imageLayout = tex.computeLayout(colorRts[i].m_usage, 0);
		vkColorAttachments[i].loadOp = convertLoadOp(colorRts[i].m_loadOperation);
		vkColorAttachments[i].storeOp = convertStoreOp(colorRts[i].m_storeOperation);
		vkColorAttachments[i].clearValue.color.float32[0] = colorRts[i].m_clearValue.m_colorf[0];
		vkColorAttachments[i].clearValue.color.float32[1] = colorRts[i].m_clearValue.m_colorf[1];
		vkColorAttachments[i].clearValue.color.float32[2] = colorRts[i].m_clearValue.m_colorf[2];
		vkColorAttachments[i].clearValue.color.float32[3] = colorRts[i].m_clearValue.m_colorf[3];

		if(!!(tex.getTextureUsage() & TextureUsageBit::kPresent))
		{
			drawsToSwapchain = true;
		}

		ANKI_ASSERT(fbWidth == 0 || (fbWidth == tex.getWidth() >> view.getFirstMipmap()));
		ANKI_ASSERT(fbHeight == 0 || (fbHeight == tex.getHeight() >> view.getFirstMipmap()));
		fbWidth = tex.getWidth() >> view.getFirstMipmap();
		fbHeight = tex.getHeight() >> view.getFirstMipmap();

		colorFormats[i] = tex.getFormat();
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
		const TextureView& view = depthStencilRt->m_textureView;
		ANKI_ASSERT(!!view.getDepthStencilAspect());
		const TextureImpl& tex = static_cast<const TextureImpl&>(view.getTexture());

		if(!!(view.getDepthStencilAspect() & DepthStencilAspectBit::kDepth))
		{
			vkDepthAttachment = {};
			vkDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkDepthAttachment.imageView = tex.getImageView(view.getSubresource());
			vkDepthAttachment.imageLayout = tex.computeLayout(depthStencilRt->m_usage, 0);
			vkDepthAttachment.loadOp = convertLoadOp(depthStencilRt->m_loadOperation);
			vkDepthAttachment.storeOp = convertStoreOp(depthStencilRt->m_storeOperation);
			vkDepthAttachment.clearValue.depthStencil.depth = depthStencilRt->m_clearValue.m_depthStencil.m_depth;
			vkDepthAttachment.clearValue.depthStencil.stencil = depthStencilRt->m_clearValue.m_depthStencil.m_stencil;

			info.pDepthAttachment = &vkDepthAttachment;
		}

		if(!!(view.getDepthStencilAspect() & DepthStencilAspectBit::kStencil) && getFormatInfo(tex.getFormat()).isStencil())
		{
			vkStencilAttachment = {};
			vkStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vkStencilAttachment.imageView = tex.getImageView(view.getSubresource());
			vkStencilAttachment.imageLayout = tex.computeLayout(depthStencilRt->m_usage, 0);
			vkStencilAttachment.loadOp = convertLoadOp(depthStencilRt->m_stencilLoadOperation);
			vkStencilAttachment.storeOp = convertStoreOp(depthStencilRt->m_stencilStoreOperation);
			vkStencilAttachment.clearValue.depthStencil.depth = depthStencilRt->m_clearValue.m_depthStencil.m_depth;
			vkStencilAttachment.clearValue.depthStencil.stencil = depthStencilRt->m_clearValue.m_depthStencil.m_stencil;

			info.pStencilAttachment = &vkStencilAttachment;
		}

		ANKI_ASSERT(fbWidth == 0 || (fbWidth == tex.getWidth() >> view.getFirstMipmap()));
		ANKI_ASSERT(fbHeight == 0 || (fbHeight == tex.getHeight() >> view.getFirstMipmap()));
		fbWidth = tex.getWidth() >> view.getFirstMipmap();
		fbHeight = tex.getHeight() >> view.getFirstMipmap();

		dsFormat = tex.getFormat();
	}

	if(vrsRt.isValid())
	{
		const TextureImpl& tex = static_cast<const TextureImpl&>(vrsRt.getTexture());

		vkVrsAttachment = {};
		vkVrsAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
		vkVrsAttachment.imageView = tex.getImageView(vrsRt.getSubresource());
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

void CommandBuffer::drawIndirect(PrimitiveTopology topology, const BufferView& buff, U32 drawCount)
{
	ANKI_ASSERT(buff.isValid());
	ANKI_ASSERT(drawCount > 0);

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((buff.getOffset() % 4) == 0);
	ANKI_ASSERT((buff.getRange() % sizeof(DrawIndirectArgs)) == 0);
	ANKI_ASSERT(sizeof(DrawIndirectArgs) * drawCount == buff.getRange());

	vkCmdDrawIndirect(self.m_handle, impl.getHandle(), buff.getOffset(), drawCount, sizeof(DrawIndirectArgs));
}

void CommandBuffer::drawIndexedIndirect(PrimitiveTopology topology, const BufferView& buff, U32 drawCount)
{
	ANKI_ASSERT(buff.isValid());
	ANKI_ASSERT(drawCount > 0);

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((buff.getOffset() % 4) == 0);
	ANKI_ASSERT(sizeof(DrawIndexedIndirectArgs) * drawCount == buff.getRange());

	vkCmdDrawIndexedIndirect(self.m_handle, impl.getHandle(), buff.getOffset(), drawCount, sizeof(DrawIndexedIndirectArgs));
}

void CommandBuffer::drawIndexedIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride,
											 const BufferView& countBuffer, U32 maxDrawCount)
{
	ANKI_ASSERT(argBuffer.isValid());
	ANKI_ASSERT(countBuffer.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();

	ANKI_ASSERT(argBufferStride >= sizeof(DrawIndexedIndirectArgs));

	const BufferImpl& argBufferImpl = static_cast<const BufferImpl&>(argBuffer.getBuffer());
	ANKI_ASSERT(argBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((argBuffer.getOffset() % 4) == 0);
	ANKI_ASSERT((argBuffer.getRange() % argBufferStride) == 0);
	ANKI_ASSERT(argBufferStride * maxDrawCount == argBuffer.getRange());

	const BufferImpl& countBufferImpl = static_cast<const BufferImpl&>(countBuffer.getBuffer());
	ANKI_ASSERT(countBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((countBuffer.getOffset() % 4) == 0);
	ANKI_ASSERT(countBuffer.getRange() == sizeof(U32));

	ANKI_ASSERT(maxDrawCount > 0 && maxDrawCount <= getGrManagerImpl().getDeviceCapabilities().m_maxDrawIndirectCount);

	vkCmdDrawIndexedIndirectCountKHR(self.m_handle, argBufferImpl.getHandle(), argBuffer.getOffset(), countBufferImpl.getHandle(),
									 countBuffer.getOffset(), maxDrawCount, argBufferStride);
}

void CommandBuffer::drawIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride, const BufferView& countBuffer,
									  U32 maxDrawCount)
{
	ANKI_ASSERT(argBuffer.isValid());
	ANKI_ASSERT(countBuffer.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_state.setPrimitiveTopology(topology);
	self.drawcallCommon();

	ANKI_ASSERT(argBufferStride >= sizeof(DrawIndirectArgs));

	const BufferImpl& argBufferImpl = static_cast<const BufferImpl&>(argBuffer.getBuffer());
	ANKI_ASSERT(argBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((argBuffer.getOffset() % 4) == 0);
	ANKI_ASSERT(maxDrawCount * argBufferStride == argBuffer.getRange());

	const BufferImpl& countBufferImpl = static_cast<const BufferImpl&>(countBuffer.getBuffer());
	ANKI_ASSERT(countBufferImpl.usageValid(BufferUsageBit::kIndirectDraw));
	ANKI_ASSERT((countBuffer.getOffset() % 4) == 0);
	ANKI_ASSERT(countBuffer.getRange() == sizeof(U32));

	ANKI_ASSERT(maxDrawCount > 0 && maxDrawCount <= getGrManagerImpl().getDeviceCapabilities().m_maxDrawIndirectCount);

	vkCmdDrawIndirectCountKHR(self.m_handle, argBufferImpl.getHandle(), argBuffer.getOffset(), countBufferImpl.getHandle(), countBuffer.getOffset(),
							  maxDrawCount, argBufferStride);
}

void CommandBuffer::drawMeshTasks(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));
	self.drawcallCommon();
	vkCmdDrawMeshTasksEXT(self.m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::drawMeshTasksIndirect(const BufferView& argBuffer, U32 drawCount)
{
	ANKI_ASSERT(argBuffer.isValid());
	ANKI_ASSERT(drawCount > 0);
	ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));

	ANKI_ASSERT((argBuffer.getOffset() % 4) == 0);
	ANKI_ASSERT(drawCount * sizeof(DispatchIndirectArgs) == argBuffer.getRange());
	const BufferImpl& impl = static_cast<const BufferImpl&>(argBuffer.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));

	ANKI_VK_SELF(CommandBufferImpl);

	self.m_state.setPrimitiveTopology(PrimitiveTopology::kTriangles); // Not sure if that's needed
	self.drawcallCommon();
	vkCmdDrawMeshTasksIndirectEXT(self.m_handle, impl.getHandle(), argBuffer.getOffset(), drawCount, sizeof(DispatchIndirectArgs));
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(groupCountX > 0 && groupCountY > 0 && groupCountZ > 0);
	self.dispatchCommon();
	vkCmdDispatch(self.m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::dispatchComputeIndirect(const BufferView& argBuffer)
{
	ANKI_ASSERT(argBuffer.isValid());

	ANKI_ASSERT(sizeof(DispatchIndirectArgs) == argBuffer.getRange());
	ANKI_ASSERT(argBuffer.getOffset() % 4 == 0);

	ANKI_VK_SELF(CommandBufferImpl);
	self.dispatchCommon();
	vkCmdDispatchIndirect(self.m_handle, static_cast<BufferImpl&>(argBuffer.getBuffer()).getHandle(), argBuffer.getOffset());
}

void CommandBuffer::traceRays(const BufferView& sbtBuffer, U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width, U32 height,
							  U32 depth)
{
	ANKI_ASSERT(sbtBuffer.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	const PtrSize sbtRecordSize = sbtRecordSize32;
	ANKI_ASSERT(hitGroupSbtRecordCount > 0);
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	ANKI_ASSERT(self.m_rtProg);
	const ShaderProgramImpl& sprog = static_cast<const ShaderProgramImpl&>(*self.m_rtProg);

	ANKI_ASSERT(rayTypeCount == sprog.getMissShaderCount() && "All the miss shaders should be in use");
	ANKI_ASSERT((hitGroupSbtRecordCount % rayTypeCount) == 0);
	const PtrSize sbtRecordCount = 1 + rayTypeCount + hitGroupSbtRecordCount;
	[[maybe_unused]] const PtrSize sbtBufferSize = sbtRecordCount * sbtRecordSize;
	ANKI_ASSERT(sbtBufferSize <= sbtBuffer.getRange());
	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_sbtRecordAlignment, sbtBuffer.getOffset()));

	self.commandCommon();

	// Bind descriptors
	self.m_descriptorState.flush(self.m_handle, self.m_microCmdb->getDSAllocator());

	Array<VkStridedDeviceAddressRegionKHR, 4> regions;
	const U64 stbBufferAddress = sbtBuffer.getBuffer().getGpuAddress() + sbtBuffer.getOffset();
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

void CommandBuffer::generateMipmaps2d(const TextureView& texView)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());
	ANKI_ASSERT(tex.getTextureType() != TextureType::k3D && "Not for 3D");
	ANKI_ASSERT(texView.getFirstMipmap() == 0 && texView.getMipmapCount() == 1);

	const U32 blitCount = tex.getMipmapCount() - 1u;
	if(blitCount == 0)
	{
		// Nothing to be done, flush the previous commands though because you may batch (and sort) things you shouldn't
		return;
	}

	const DepthStencilAspectBit aspect = texView.getDepthStencilAspect();
	const U32 face = texView.getFirstFace();
	const U32 layer = texView.getFirstLayer();

	for(U32 i = 0; i < blitCount; ++i)
	{
		// Transition source
		// OPT: Combine the 2 barriers
		if(i > 0)
		{
			const VkImageSubresourceRange range = tex.computeVkImageSubresourceRange(TextureSubresourceDescriptor::surface(i, face, layer, aspect));

			self.setImageBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex.m_imageHandle,
								 range);
		}

		// Transition destination
		{
			const VkImageSubresourceRange range =
				tex.computeVkImageSubresourceRange(TextureSubresourceDescriptor::surface(i + 1, face, layer, aspect));

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

void CommandBuffer::blitTexture([[maybe_unused]] const TextureView& srcView, [[maybe_unused]] const TextureView& destView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::clearTexture(const TextureView& texView, const ClearValue& clearValue)
{
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());

	VkClearColorValue vclear;
	static_assert(sizeof(vclear) == sizeof(clearValue), "See file");
	memcpy(&vclear, &clearValue, sizeof(clearValue));

	if(!texView.getDepthStencilAspect())
	{
		const VkImageSubresourceRange vkRange = tex.computeVkImageSubresourceRange(texView.getSubresource());
		vkCmdClearColorImage(self.m_handle, tex.m_imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vclear, 1, &vkRange);
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

void CommandBuffer::copyBufferToTexture(const BufferView& buff, const TextureView& texView)
{
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());
	ANKI_ASSERT(tex.usageValid(TextureUsageBit::kTransferDestination));
	ANKI_ASSERT(texView.isGoodForCopyBufferToTexture());
	const VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	const VkImageSubresourceRange range = tex.computeVkImageSubresourceRange(texView.getSubresource());

	// Compute the sizes of the mip
	const U32 width = tex.getWidth() >> range.baseMipLevel;
	const U32 height = tex.getHeight() >> range.baseMipLevel;
	ANKI_ASSERT(width && height);
	const U32 depth = (tex.getTextureType() == TextureType::k3D) ? (tex.getDepth() >> range.baseMipLevel) : 1u;

	if(tex.getTextureType() != TextureType::k3D)
	{
		ANKI_ASSERT(buff.getRange() == computeSurfaceSize(width, height, tex.getFormat()));
	}
	else
	{
		ANKI_ASSERT(buff.getRange() == computeVolumeSize(width, height, depth, tex.getFormat()));
	}

	// Copy
	VkBufferImageCopy region;
	region.imageSubresource.aspectMask = range.aspectMask;
	region.imageSubresource.baseArrayLayer = range.baseArrayLayer;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = range.baseMipLevel;
	region.imageOffset = {0, 0, 0};
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = depth;
	region.bufferOffset = buff.getOffset();
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;

	vkCmdCopyBufferToImage(self.m_handle, static_cast<const BufferImpl&>(buff.getBuffer()).getHandle(), tex.m_imageHandle, layout, 1, &region);
}

void CommandBuffer::fillBuffer(const BufferView& buff, U32 value)
{
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	ANKI_ASSERT(!self.m_insideRenderpass);
	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kTransferDestination));

	ANKI_ASSERT((buff.getOffset() % 4) == 0 && "Should be multiple of 4");
	ANKI_ASSERT((buff.getRange() % 4) == 0 && "Should be multiple of 4");

	vkCmdFillBuffer(self.m_handle, impl.getHandle(), buff.getOffset(), buff.getRange(), value);
}

void CommandBuffer::writeOcclusionQueriesResultToBuffer(ConstWeakArray<OcclusionQuery*> queries, const BufferView& buff)
{
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(queries.getSize() > 0);
	self.commandCommon();
	ANKI_ASSERT(!self.m_insideRenderpass);

	ANKI_ASSERT(sizeof(U32) * queries.getSize() <= buff.getRange());
	ANKI_ASSERT((buff.getOffset() % 4) == 0);

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kTransferDestination));

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		ANKI_ASSERT(queries[i]);

		OcclusionQueryImpl* q = static_cast<OcclusionQueryImpl*>(queries[i]);

		vkCmdCopyQueryPoolResults(self.m_handle, q->m_handle.getQueryPool(), q->m_handle.getQueryIndex(), 1, impl.getHandle(),
								  buff.getOffset() * sizeof(U32) * i, sizeof(U32), VK_QUERY_RESULT_PARTIAL_BIT);

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

void CommandBuffer::buildAccelerationStructure(AccelerationStructure* as, const BufferView& scratchBuffer)
{
	ANKI_ASSERT(scratchBuffer.isValid());
	ANKI_ASSERT(as);
	ANKI_ASSERT(as->getBuildScratchBufferSize() <= scratchBuffer.getRange());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	// Get objects
	const AccelerationStructureImpl& asImpl = static_cast<AccelerationStructureImpl&>(*as);

	// Create the build info
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo;
	VkAccelerationStructureBuildRangeInfoKHR rangeInfo;
	asImpl.generateBuildInfo(scratchBuffer.getBuffer().getGpuAddress() + scratchBuffer.getOffset(), buildInfo, rangeInfo);

	// Run the command
	Array<const VkAccelerationStructureBuildRangeInfoKHR*, 1> pRangeInfos = {&rangeInfo};
	vkCmdBuildAccelerationStructuresKHR(self.m_handle, 1, &buildInfo, &pRangeInfos[0]);
}

#if ANKI_DLSS
/// Utility function to get the NGX's resource structure for a texture
/// @param[in] tex the texture to generate the NVSDK_NGX_Resource_VK from
static NVSDK_NGX_Resource_VK getNGXResourceFromAnkiTexture(const TextureView& view)
{
	const TextureImpl& tex = static_cast<const TextureImpl&>(view.getTexture());

	const VkImageView imageView = tex.getImageView(view.getSubresource());
	const VkFormat format = tex.m_vkFormat;
	const VkImage image = tex.m_imageHandle;
	const VkImageSubresourceRange subresourceRange = tex.computeVkImageSubresourceRange(view.getSubresource());
	const Bool isUAV = !!(tex.m_vkUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT);

	// TODO Not sure if I should pass the width,height of the image or the view
	return NVSDK_NGX_Create_ImageView_Resource_VK(imageView, image, subresourceRange, format, tex.getWidth(), tex.getHeight(), isUAV);
}
#endif

void CommandBuffer::upscale(GrUpscaler* upscaler, const TextureView& inColor, const TextureView& outUpscaledColor, const TextureView& motionVectors,
							const TextureView& depth, const TextureView& exposure, Bool resetAccumulation, const Vec2& jitterOffset,
							const Vec2& motionVectorsScale)
{
#if ANKI_DLSS
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_dlss);
	ANKI_ASSERT(upscaler->getUpscalerType() == GrUpscalerType::kDlss2);

	self.commandCommon();

	const GrUpscalerImpl& upscalerImpl = static_cast<const GrUpscalerImpl&>(*upscaler);

	NVSDK_NGX_Resource_VK srcResVk = getNGXResourceFromAnkiTexture(inColor);
	NVSDK_NGX_Resource_VK dstResVk = getNGXResourceFromAnkiTexture(outUpscaledColor);
	NVSDK_NGX_Resource_VK mvResVk = getNGXResourceFromAnkiTexture(motionVectors);
	NVSDK_NGX_Resource_VK depthResVk = getNGXResourceFromAnkiTexture(depth);
	NVSDK_NGX_Resource_VK exposureResVk = getNGXResourceFromAnkiTexture(exposure);

	const U32 mipLevel = inColor.getFirstMipmap();
	const NVSDK_NGX_Coordinates renderingOffset = {0, 0};
	const NVSDK_NGX_Dimensions renderingSize = {inColor.getTexture().getWidth() >> mipLevel, inColor.getTexture().getHeight() >> mipLevel};

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
		const TextureImpl& impl = static_cast<const TextureImpl&>(barrier.m_textureView.getTexture());
		const TextureUsageBit nextUsage = barrier.m_nextUsage;
		const TextureUsageBit prevUsage = barrier.m_previousUsage;

		ANKI_ASSERT(impl.usageValid(prevUsage));
		ANKI_ASSERT(impl.usageValid(nextUsage));
		ANKI_ASSERT(((nextUsage & TextureUsageBit::kGenerateMipmaps) == TextureUsageBit::kGenerateMipmaps
					 || (nextUsage & TextureUsageBit::kGenerateMipmaps) == TextureUsageBit::kNone)
					&& "GENERATE_MIPMAPS should be alone");

		if(barrier.m_textureView.getFirstMipmap() > 0 && nextUsage == TextureUsageBit::kGenerateMipmaps) [[unlikely]]
		{
			// This transition happens inside CommandBufferImpl::generateMipmapsX. No need to do something
			continue;
		}

		if(nextUsage == TextureUsageBit::kGenerateMipmaps) [[unlikely]]
		{
			// The transition of the non zero mip levels happens inside CommandBufferImpl::generateMipmapsX so limit the subresource

			ANKI_ASSERT(barrier.m_textureView.getFirstMipmap() == 0 && barrier.m_textureView.getMipmapCount() == 1);
		}

		VkImageMemoryBarrier& inf = *imageBarriers.emplaceBack();
		inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		inf.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		inf.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		inf.image = impl.m_imageHandle;
		inf.subresourceRange = impl.computeVkImageSubresourceRange(barrier.m_textureView.getSubresource());

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
		ANKI_ASSERT(barrier.m_bufferView.isValid());
		const BufferImpl& impl = static_cast<const BufferImpl&>(barrier.m_bufferView.getBuffer());

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
	ANKI_ASSERT(prog.getReflectionInfo().m_descriptor.m_pushConstantsSize == dataSize
				&& "The bound program should have push constants equal to the \"dataSize\" parameter");

	self.commandCommon();
	self.m_descriptorState.setPushConstants(data, dataSize);
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

	m_descriptorState.init(m_pool);

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

	// Get or create ppline
	Pipeline ppline;
	Bool stateDirty;
	m_graphicsProg->getPipelineFactory().getOrCreatePipeline(m_state, ppline, stateDirty);

	if(stateDirty)
	{
		vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ppline.getHandle());
	}

	// Bind dsets
	m_descriptorState.flush(m_handle, m_microCmdb->getDSAllocator());

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

	commandCommon();

	// Bind descriptors
	m_descriptorState.flush(m_handle, m_microCmdb->getDSAllocator());
}

} // end namespace anki

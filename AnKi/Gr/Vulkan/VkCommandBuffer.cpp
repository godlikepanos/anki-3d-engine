// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Gr/Vulkan/VkShaderProgram.h>

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
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.bindVertexBuffer(binding, stepRate, stride);

	const VkBuffer vkbuff = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();
	vkCmdBindVertexBuffers(self.m_handle, binding, 1, &vkbuff, &buff.getOffset());
}

void CommandBuffer::setVertexAttribute(VertexAttributeSemantic attribute, U32 buffBinding, Format fmt, U32 relativeOffset)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setVertexAttribute(attribute, buffBinding, fmt, relativeOffset);
}

void CommandBuffer::bindIndexBuffer(const BufferView& buff, IndexType type)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const BufferImpl& buffi = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(!!(buffi.getBufferUsage() & BufferUsageBit::kVertexOrIndex));
	vkCmdBindIndexBuffer(self.m_handle, buffi.getHandle(), buff.getOffset(), convertIndexType(type));
}

void CommandBuffer::setPrimitiveRestart(Bool enable)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setPrimitiveRestart(enable);
}

void CommandBuffer::setViewport(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(width > 0 && height > 0);
	self.commandCommon();
	self.m_graphicsState.setViewport(minx, miny, width, height);
}

void CommandBuffer::setScissor(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(width > 0 && height > 0);
	self.commandCommon();
	self.m_graphicsState.setScissor(minx, miny, width, height);
}

void CommandBuffer::setFillMode(FillMode mode)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setFillMode(mode);
}

void CommandBuffer::setCullMode(FaceSelectionBit mode)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setCullMode(mode);
}

void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setPolygonOffset(factor, units);
	vkCmdSetDepthBias(self.m_handle, factor, 0.0f, units);
}

void CommandBuffer::setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
										 StencilOperation stencilPassDepthPass)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setStencilOperations(face, stencilFail, stencilPassDepthFail, stencilPassDepthPass);
}

void CommandBuffer::setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setStencilCompareOperation(face, comp);
}

void CommandBuffer::setStencilCompareMask(FaceSelectionBit face, U32 mask)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setStencilCompareMask(face, mask);
}

void CommandBuffer::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setStencilWriteMask(face, mask);
}

void CommandBuffer::setStencilReference(FaceSelectionBit face, U32 ref)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setStencilReference(face, ref);
}

void CommandBuffer::setDepthWrite(Bool enable)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setDepthWrite(enable);
}

void CommandBuffer::setDepthCompareOperation(CompareOperation op)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setDepthCompareOperation(op);
}

void CommandBuffer::setAlphaToCoverage(Bool enable)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setAlphaToCoverage(enable);
}

void CommandBuffer::setColorChannelWriteMask(U32 attachment, ColorBit mask)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setColorChannelWriteMask(attachment, mask);
}

void CommandBuffer::setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA, BlendFactor dstA)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA);
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setBlendOperation(attachment, funcRgb, funcA);
}

void CommandBuffer::bindSrv(U32 reg, U32 space, const TextureView& texView)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());

	ANKI_ASSERT(texView.isGoodForSampling());
	const VkImageLayout lay = tex.computeLayout(TextureUsageBit::kAllSrv & tex.getTextureUsage());
	self.m_descriptorState.bindSampledTexture(space, reg, tex.getImageView(texView.getSubresource()), lay);
}

void CommandBuffer::bindUav(U32 reg, U32 space, const TextureView& texView)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());

	ANKI_ASSERT(texView.isGoodForStorage());
	self.m_descriptorState.bindStorageTexture(space, reg, tex.getImageView(texView.getSubresource()));

	const Bool isPresentable = !!(tex.getTextureUsage() & TextureUsageBit::kPresent);
	if(isPresentable)
	{
		self.m_renderedToDefaultFb = true;
	}
}

void CommandBuffer::bindSampler(U32 reg, U32 space, Sampler* sampler)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkSampler handle = static_cast<const SamplerImpl&>(*sampler).m_sampler->getHandle();
	self.m_descriptorState.bindSampler(space, reg, handle);
}

void CommandBuffer::bindConstantBuffer(U32 reg, U32 space, const BufferView& buff)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkBuffer handle = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();
	self.m_descriptorState.bindUniformBuffer(space, reg, handle, buff.getOffset(), buff.getRange());
}

void CommandBuffer::bindSrv(U32 reg, U32 space, const BufferView& buff, Format fmt)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkBuffer handle = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();

	if(fmt == Format::kNone)
	{
		self.m_descriptorState.bindReadStorageBuffer(space, reg, handle, buff.getOffset(), buff.getRange());
	}
	else
	{
		const VkBufferView view = static_cast<const BufferImpl&>(buff.getBuffer()).getOrCreateBufferView(fmt, buff.getOffset(), buff.getRange());
		self.m_descriptorState.bindReadTexelBuffer(space, reg, view);
	}
}

void CommandBuffer::bindUav(U32 reg, U32 space, const BufferView& buff, Format fmt)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkBuffer handle = static_cast<const BufferImpl&>(buff.getBuffer()).getHandle();

	if(fmt == Format::kNone)
	{
		self.m_descriptorState.bindReadWriteStorageBuffer(space, reg, handle, buff.getOffset(), buff.getRange());
	}
	else
	{
		const VkBufferView view = static_cast<const BufferImpl&>(buff.getBuffer()).getOrCreateBufferView(fmt, buff.getOffset(), buff.getRange());
		self.m_descriptorState.bindReadWriteTexelBuffer(space, reg, view);
	}
}

void CommandBuffer::bindSrv(U32 reg, U32 space, AccelerationStructure* as)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkAccelerationStructureKHR& handle = static_cast<const AccelerationStructureImpl&>(*as).getHandle();
	self.m_descriptorState.bindAccelerationStructure(space, reg, &handle);
}

void CommandBuffer::bindShaderProgram(ShaderProgram* prog)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	ShaderProgramImpl& impl = static_cast<ShaderProgramImpl&>(*prog);
	VkPipelineBindPoint bindPoint;

	if(impl.isGraphics())
	{
		self.m_graphicsProg = &impl;
		self.m_computeProg = nullptr; // Unbind the compute prog. Doesn't work like vulkan
		self.m_rtProg = nullptr; // See above
		self.m_graphicsState.bindShaderProgram(&impl);

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

	self.m_descriptorState.setShaderProgram(&impl.getPipelineLayout(), impl.getUuid(), bindPoint);
}

void CommandBuffer::beginRenderPass(ConstWeakArray<RenderTarget> colorRts, RenderTarget* depthStencilRt, const TextureView& vrsRt, U8 vrsRtTexelSizeX,
									U8 vrsRtTexelSizeY)
{
	ANKI_TRACE_FUNCTION();
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
		vkColorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		vkColorAttachments[i].imageView = tex.getImageView(view.getSubresource());
		vkColorAttachments[i].imageLayout = tex.computeLayout(colorRts[i].m_usage);
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
			vkDepthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			vkDepthAttachment.imageView = tex.getImageView(view.getSubresource());
			vkDepthAttachment.imageLayout = tex.computeLayout(depthStencilRt->m_usage);
			vkDepthAttachment.loadOp = convertLoadOp(depthStencilRt->m_loadOperation);
			vkDepthAttachment.storeOp = convertStoreOp(depthStencilRt->m_storeOperation);
			vkDepthAttachment.clearValue.depthStencil.depth = depthStencilRt->m_clearValue.m_depthStencil.m_depth;
			vkDepthAttachment.clearValue.depthStencil.stencil = depthStencilRt->m_clearValue.m_depthStencil.m_stencil;

			info.pDepthAttachment = &vkDepthAttachment;
		}

		if(!!(view.getDepthStencilAspect() & DepthStencilAspectBit::kStencil) && getFormatInfo(tex.getFormat()).isStencil())
		{
			vkStencilAttachment = {};
			vkStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			vkStencilAttachment.imageView = tex.getImageView(view.getSubresource());
			vkStencilAttachment.imageLayout = tex.computeLayout(depthStencilRt->m_usage);
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
	info.renderArea.offset.x = 0;
	info.renderArea.offset.y = 0;
	info.renderArea.extent.width = fbWidth;
	info.renderArea.extent.height = fbHeight;

	// State bookkeeping
	self.m_graphicsState.beginRenderPass({colorFormats.getBegin(), colorRts.getSize()}, dsFormat, UVec2(fbWidth, fbHeight));
	if(drawsToSwapchain)
	{
		self.m_renderedToDefaultFb = true;
	}

	// Finaly
	vkCmdBeginRendering(self.m_handle, &info);
}

void CommandBuffer::endRenderPass()
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);

	ANKI_ASSERT(self.m_insideRenderpass);
#if ANKI_ASSERTIONS_ENABLED
	self.m_insideRenderpass = false;
#endif

	self.commandCommon();
	vkCmdEndRendering(self.m_handle);
}

void CommandBuffer::setVrsRate([[maybe_unused]] VrsRate rate)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(getGrManagerImpl().getDeviceCapabilities().m_vrs);
	ANKI_ASSERT(rate < VrsRate::kCount);
	self.commandCommon();

	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::drawIndexed(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex, U32 baseVertex, U32 baseInstance)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_graphicsState.setPrimitiveTopology(topology);
	self.drawcallCommon();
	vkCmdDrawIndexed(self.m_handle, count, instanceCount, firstIndex, baseVertex, baseInstance);
}

void CommandBuffer::draw(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.m_graphicsState.setPrimitiveTopology(topology);
	self.drawcallCommon();
	vkCmdDraw(self.m_handle, count, instanceCount, first, baseInstance);
}

void CommandBuffer::drawIndirect(PrimitiveTopology topology, const BufferView& buff, U32 drawCount)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());
	ANKI_ASSERT(drawCount > 0);

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_graphicsState.setPrimitiveTopology(topology);
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
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());
	ANKI_ASSERT(drawCount > 0);

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_graphicsState.setPrimitiveTopology(topology);
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
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(argBuffer.isValid());
	ANKI_ASSERT(countBuffer.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_graphicsState.setPrimitiveTopology(topology);
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

	vkCmdDrawIndexedIndirectCount(self.m_handle, argBufferImpl.getHandle(), argBuffer.getOffset(), countBufferImpl.getHandle(),
								  countBuffer.getOffset(), maxDrawCount, argBufferStride);
}

void CommandBuffer::drawIndirectCount(PrimitiveTopology topology, const BufferView& argBuffer, U32 argBufferStride, const BufferView& countBuffer,
									  U32 maxDrawCount)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(argBuffer.isValid());
	ANKI_ASSERT(countBuffer.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.m_graphicsState.setPrimitiveTopology(topology);
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

	vkCmdDrawIndirectCount(self.m_handle, argBufferImpl.getHandle(), argBuffer.getOffset(), countBufferImpl.getHandle(), countBuffer.getOffset(),
						   maxDrawCount, argBufferStride);
}

void CommandBuffer::drawMeshTasks(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));
	self.drawcallCommon();
	vkCmdDrawMeshTasksEXT(self.m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::drawMeshTasksIndirect(const BufferView& argBuffer, U32 drawCount)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(argBuffer.isValid());
	ANKI_ASSERT(drawCount > 0);
	ANKI_ASSERT(!!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_mesh_shader));

	ANKI_ASSERT((argBuffer.getOffset() % 4) == 0);
	ANKI_ASSERT(drawCount * sizeof(DispatchIndirectArgs) == argBuffer.getRange());
	const BufferImpl& impl = static_cast<const BufferImpl&>(argBuffer.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kIndirectDraw));

	ANKI_VK_SELF(CommandBufferImpl);

	self.m_graphicsState.setPrimitiveTopology(PrimitiveTopology::kTriangles); // Not sure if that's needed
	self.drawcallCommon();
	vkCmdDrawMeshTasksIndirectEXT(self.m_handle, impl.getHandle(), argBuffer.getOffset(), drawCount, sizeof(DispatchIndirectArgs));
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(groupCountX > 0 && groupCountY > 0 && groupCountZ > 0);
	self.dispatchCommon();
	vkCmdDispatch(self.m_handle, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::dispatchComputeIndirect(const BufferView& argBuffer)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(argBuffer.isValid());

	ANKI_ASSERT(sizeof(DispatchIndirectArgs) == argBuffer.getRange());
	ANKI_ASSERT(argBuffer.getOffset() % 4 == 0);

	ANKI_VK_SELF(CommandBufferImpl);
	self.dispatchCommon();
	vkCmdDispatchIndirect(self.m_handle, static_cast<BufferImpl&>(argBuffer.getBuffer()).getHandle(), argBuffer.getOffset());
}

void CommandBuffer::dispatchGraph([[maybe_unused]] const BufferView& scratchBuffer, [[maybe_unused]] const void* records,
								  [[maybe_unused]] U32 recordCount, [[maybe_unused]] U32 recordStride)
{
	ANKI_ASSERT(!"Not supported");
}

void CommandBuffer::dispatchRays(const BufferView& sbtBuffer, U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width,
								 U32 height, U32 depth)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.traceRaysInternal(sbtBuffer, sbtRecordSize32, hitGroupSbtRecordCount, rayTypeCount, width, height, depth, {});
}

void CommandBuffer::dispatchRaysIndirect(const BufferView& sbtBuffer, U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount,
										 BufferView argsBuffer)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.traceRaysInternal(sbtBuffer, sbtRecordSize32, hitGroupSbtRecordCount, rayTypeCount, 0, 0, 0, argsBuffer);
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

void CommandBuffer::copyBufferToTexture(const BufferView& buff, const TextureView& texView, const TextureRect& rect)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const TextureImpl& tex = static_cast<const TextureImpl&>(texView.getTexture());
	ANKI_ASSERT(tex.usageValid(TextureUsageBit::kCopyDestination));
	ANKI_ASSERT(texView.isGoodForCopyBufferToTexture());
	const VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	const VkImageSubresourceRange range = tex.computeVkImageSubresourceRange(texView.getSubresource());

	// Compute the sizes of the mip
	const U32 width = (rect.m_width != kMaxU32) ? rect.m_width : tex.getWidth() >> range.baseMipLevel;
	const U32 height = (rect.m_height != kMaxU32) ? rect.m_height : tex.getHeight() >> range.baseMipLevel;
	const U32 depth =
		(tex.getTextureType() == TextureType::k3D) ? ((rect.m_depth != kMaxU32) ? rect.m_depth : tex.getDepth() >> range.baseMipLevel) : 1u;

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
	region.imageOffset = {I32(rect.m_offsetX), I32(rect.m_offsetY), I32(rect.m_offsetZ)};
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = depth;
	region.bufferOffset = buff.getOffset();
	region.bufferImageHeight = 0;
	region.bufferRowLength = 0;

	vkCmdCopyBufferToImage(self.m_handle, static_cast<const BufferImpl&>(buff.getBuffer()).getHandle(), tex.m_imageHandle, layout, 1, &region);
}

void CommandBuffer::zeroBuffer(const BufferView& buff)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	ANKI_ASSERT(!self.m_insideRenderpass);
	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kCopyDestination));

	ANKI_ASSERT((buff.getOffset() % 4) == 0 && "Should be multiple of 4");
	ANKI_ASSERT((buff.getRange() % 4) == 0 && "Should be multiple of 4");

	vkCmdFillBuffer(self.m_handle, impl.getHandle(), buff.getOffset(), buff.getRange(), 0);
}

void CommandBuffer::writeOcclusionQueriesResultToBuffer(ConstWeakArray<OcclusionQuery*> queries, const BufferView& buff)
{
	ANKI_TRACE_FUNCTION();
	ANKI_ASSERT(buff.isValid());

	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(queries.getSize() > 0);
	self.commandCommon();
	ANKI_ASSERT(!self.m_insideRenderpass);

	ANKI_ASSERT(sizeof(U32) * queries.getSize() <= buff.getRange());
	ANKI_ASSERT((buff.getOffset() % 4) == 0);

	const BufferImpl& impl = static_cast<const BufferImpl&>(buff.getBuffer());
	ANKI_ASSERT(impl.usageValid(BufferUsageBit::kCopyDestination));

	for(U32 i = 0; i < queries.getSize(); ++i)
	{
		ANKI_ASSERT(queries[i]);

		OcclusionQueryImpl* q = static_cast<OcclusionQueryImpl*>(queries[i]);

		vkCmdCopyQueryPoolResults(self.m_handle, q->m_handle.getQueryPool(), q->m_handle.getQueryIndex(), 1, impl.getHandle(),
								  buff.getOffset() * sizeof(U32) * i, sizeof(U32), VK_QUERY_RESULT_PARTIAL_BIT);
	}
}

void CommandBuffer::copyBufferToBuffer(Buffer* src, Buffer* dst, ConstWeakArray<CopyBufferToBufferInfo> copies)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(static_cast<const BufferImpl&>(*src).usageValid(BufferUsageBit::kCopySource));
	ANKI_ASSERT(static_cast<const BufferImpl&>(*dst).usageValid(BufferUsageBit::kCopyDestination));
	ANKI_ASSERT(copies.getSize() > 0);

	self.commandCommon();

	static_assert(sizeof(CopyBufferToBufferInfo) == sizeof(VkBufferCopy));
	const VkBufferCopy* vkCopies = reinterpret_cast<const VkBufferCopy*>(&copies[0]);

	vkCmdCopyBuffer(self.m_handle, static_cast<const BufferImpl&>(*src).getHandle(), static_cast<const BufferImpl&>(*dst).getHandle(),
					copies.getSize(), &vkCopies[0]);
}

void CommandBuffer::buildAccelerationStructure(AccelerationStructure* as, const BufferView& scratchBuffer)
{
	ANKI_TRACE_FUNCTION();
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
	ANKI_TRACE_FUNCTION();
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
	vkDlssEvalParams.InJitterOffsetX = jitterOffset.x;
	vkDlssEvalParams.InJitterOffsetY = jitterOffset.y;
	vkDlssEvalParams.InReset = resetAccumulation;
	vkDlssEvalParams.InMVScaleX = motionVectorsScale.x;
	vkDlssEvalParams.InMVScaleY = motionVectorsScale.y;
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
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	DynamicArray<VkImageMemoryBarrier, MemoryPoolPtrWrapper<StackMemoryPool>> imageBarriers(&self.m_pool);
	VkMemoryBarrier genericBarrier = {};
	genericBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	VkPipelineStageFlags srcStageMask = 0;
	VkPipelineStageFlags dstStageMask = 0;

	for(const TextureBarrierInfo& barrier : textures)
	{
		const TextureImpl& impl = static_cast<const TextureImpl&>(barrier.m_textureView.getTexture());

		imageBarriers.emplaceBack(impl.computeBarrierInfo(barrier.m_previousUsage, barrier.m_nextUsage, barrier.m_textureView.getSubresource(),
														  srcStageMask, dstStageMask));
	}

	for(const BufferBarrierInfo& barrier : buffers)
	{
		ANKI_ASSERT(barrier.m_bufferView.isValid());
		const BufferImpl& impl = static_cast<const BufferImpl&>(barrier.m_bufferView.getBuffer());
		const VkBufferMemoryBarrier akBarrier = impl.computeBarrierInfo(barrier.m_previousUsage, barrier.m_nextUsage, srcStageMask, dstStageMask);

		genericBarrier.srcAccessMask |= akBarrier.srcAccessMask;
		genericBarrier.dstAccessMask |= akBarrier.dstAccessMask;
	}

	for(const AccelerationStructureBarrierInfo& barrier : accelerationStructures)
	{
		ANKI_ASSERT(barrier.m_as);

		const VkMemoryBarrier memBarrier =
			AccelerationStructureImpl::computeBarrierInfo(barrier.m_previousUsage, barrier.m_nextUsage, srcStageMask, dstStageMask);

		genericBarrier.srcAccessMask |= memBarrier.srcAccessMask;
		genericBarrier.dstAccessMask |= memBarrier.dstAccessMask;
	}

	const Bool genericBarrierSet = genericBarrier.srcAccessMask != 0 && genericBarrier.dstAccessMask != 0;

	vkCmdPipelineBarrier(self.m_handle, srcStageMask, dstStageMask, 0, (genericBarrierSet) ? 1 : 0, (genericBarrierSet) ? &genericBarrier : nullptr,
						 0, nullptr, imageBarriers.getSize(), (imageBarriers.getSize()) ? &imageBarriers[0] : nullptr);

	ANKI_TRACE_INC_COUNTER(VkBarrier, 1);
}

void CommandBuffer::beginOcclusionQuery(OcclusionQuery* query)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);

	vkCmdBeginQuery(self.m_handle, handle, idx, 0);
}

void CommandBuffer::endOcclusionQuery(OcclusionQuery* query)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkQueryPool handle = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const OcclusionQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);

	vkCmdEndQuery(self.m_handle, handle, idx);
}

void CommandBuffer::beginPipelineQuery(PipelineQuery* query)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const VkQueryPool handle = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);
	vkCmdBeginQuery(self.m_handle, handle, idx, 0);
}

void CommandBuffer::endPipelineQuery(PipelineQuery* query)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	const VkQueryPool handle = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const PipelineQueryImpl&>(*query).m_handle.getQueryIndex();
	ANKI_ASSERT(handle);
	vkCmdEndQuery(self.m_handle, handle, idx);
}

void CommandBuffer::writeTimestamp(TimestampQuery* query)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();

	const VkQueryPool handle = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryPool();
	const U32 idx = static_cast<const TimestampQueryImpl&>(*query).m_handle.getQueryIndex();

	vkCmdWriteTimestamp(self.m_handle, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, handle, idx);
}

Bool CommandBuffer::isEmpty() const
{
	ANKI_VK_SELF_CONST(CommandBufferImpl);
	return self.isEmpty();
}

void CommandBuffer::setFastConstants(const void* data, U32 dataSize)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	ANKI_ASSERT(data && dataSize && dataSize % 16 == 0);
	// ANKI_ASSERT(static_cast<const ShaderProgramImpl&>(self.getBoundProgram()).getReflection().m_descriptor.m_fastConstantsSize == dataSize
	//		&& "The bound program should have push constants equal to the \"dataSize\" parameter");

	self.commandCommon();
	self.m_descriptorState.setFastConstants(data, dataSize);
}

void CommandBuffer::setLineWidth(F32 width)
{
	ANKI_TRACE_FUNCTION();
	ANKI_VK_SELF(CommandBufferImpl);
	self.commandCommon();
	self.m_graphicsState.setLineWidth(width);
}

void CommandBuffer::pushDebugMarker(CString name, Vec3 color)
{
	ANKI_TRACE_FUNCTION();
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
	ANKI_TRACE_FUNCTION();
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

	m_pool.init(GrMemoryPool::getSingleton().getAllocationCallback(), GrMemoryPool::getSingleton().getAllocationCallbackUserData(), 256_KB, 2.0f);

	m_descriptorState.init(&m_pool);

	m_debugMarkers = !!(getGrManagerImpl().getExtensions() & VulkanExtensions::kEXT_debug_utils);

	return Error::kNone;
}

void CommandBufferImpl::setImageBarrier(VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, VkImageLayout prevLayout,
										VkPipelineStageFlags dstStage, VkAccessFlags dstAccess, VkImageLayout newLayout, VkImage img,
										const VkImageSubresourceRange& range)
{
	ANKI_TRACE_FUNCTION();
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
	ANKI_TRACE_FUNCTION();
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
		m_graphicsState.setEnablePipelineStatistics(true);
	}
}

void CommandBufferImpl::endRecording()
{
	ANKI_TRACE_FUNCTION();
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
	ANKI_TRACE_FUNCTION();

	// Preconditions
	commandCommon();
	ANKI_ASSERT(m_graphicsProg);
	ANKI_ASSERT(m_insideRenderpass);

	// Get or create ppline
	m_graphicsProg->getGraphicsPipelineFactory().flushState(m_graphicsState, m_handle);

	// Bind dsets
	m_descriptorState.flush(m_handle, m_microCmdb->getDSAllocator());

	ANKI_TRACE_INC_COUNTER(VkDrawcall, 1);
}

ANKI_FORCE_INLINE void CommandBufferImpl::dispatchCommon()
{
	ANKI_ASSERT(m_computeProg);

	commandCommon();

	// Bind descriptors
	m_descriptorState.flush(m_handle, m_microCmdb->getDSAllocator());
}

void CommandBufferImpl::traceRaysInternal(const BufferView& sbtBuffer, U32 sbtRecordSize32, U32 hitGroupSbtRecordCount, U32 rayTypeCount, U32 width,
										  U32 height, U32 depth, BufferView argsBuff)
{
	ANKI_ASSERT(sbtBuffer.isValid());
	ANKI_ASSERT(rayTypeCount > 0);

	const PtrSize sbtRecordSize = sbtRecordSize32;
	ANKI_ASSERT(m_rtProg);

	ANKI_ASSERT((hitGroupSbtRecordCount % rayTypeCount) == 0);
	const PtrSize sbtRecordCount = 1 + rayTypeCount + hitGroupSbtRecordCount;
	[[maybe_unused]] const PtrSize sbtBufferSize = sbtRecordCount * sbtRecordSize;
	ANKI_ASSERT(sbtBufferSize <= sbtBuffer.getRange());
	ANKI_ASSERT(isAligned(getGrManagerImpl().getDeviceCapabilities().m_sbtRecordAlignment, sbtBuffer.getOffset()));

	commandCommon();

	// Bind descriptors
	m_descriptorState.flush(m_handle, m_microCmdb->getDSAllocator());

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

	if(!argsBuff.isValid())
	{
		ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
		vkCmdTraceRaysKHR(m_handle, &regions[0], &regions[1], &regions[2], &regions[3], width, height, depth);
	}
	else
	{
		ANKI_ASSERT(argsBuff.getRange() == sizeof(DispatchIndirectArgs));
		vkCmdTraceRaysIndirectKHR(m_handle, &regions[0], &regions[1], &regions[2], &regions[3],
								  argsBuff.getBuffer().getGpuAddress() + argsBuff.getOffset());
	}
}

} // end namespace anki

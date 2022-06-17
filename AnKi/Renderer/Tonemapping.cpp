// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/Renderer.h>

namespace anki {

Error Tonemapping::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize tonemapping");
	}

	return err;
}

Error Tonemapping::initInternal()
{
	m_inputTexMip = m_r->getDownscaleBlur().getMipmapCount() - 2;
	const U32 width = m_r->getDownscaleBlur().getPassWidth(m_inputTexMip);
	const U32 height = m_r->getDownscaleBlur().getPassHeight(m_inputTexMip);

	ANKI_R_LOGV("Initializing tonemapping. Resolution %ux%u", width, height);

	// Create program
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/TonemappingAverageLuminance.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addConstant("INPUT_TEX_SIZE", UVec2(width, height));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variantInitInfo, variant);
	m_grProg = variant->getProgram();

	// Create buffer
	m_luminanceBuff = getGrManager().newBuffer(BufferInitInfo(
		sizeof(Vec4), BufferUsageBit::ALL_STORAGE | BufferUsageBit::ALL_UNIFORM | BufferUsageBit::TRANSFER_DESTINATION,
		BufferMapAccessBit::NONE, "AvgLum"));

	// Create exposure texture
	TextureUsageBit usage =
		TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::SAMPLED_COMPUTE | TextureUsageBit::IMAGE_COMPUTE_WRITE;
	TextureInitInfo texinit = m_r->create2DRenderTargetInitInfo(1, 1, Format::R32_SFLOAT, usage, "Exposure 1x1");
	m_exposure1x1 = m_r->createAndClearRenderTarget(texinit, TextureUsageBit::TRANSFER_DESTINATION);

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::GENERAL_WORK;
	CommandBufferPtr cmdb = getGrManager().newCommandBuffer(cmdbinit);

	TransferGpuAllocatorHandle handle;
	ANKI_CHECK(m_r->getResourceManager().getTransferGpuAllocator().allocate(sizeof(Vec4), handle));
	void* data = handle.getMappedMemory();

	*static_cast<Vec4*>(data) = Vec4(0.5);
	cmdb->copyBufferToBuffer(handle.getBuffer(), handle.getOffset(), m_luminanceBuff, 0, handle.getRange());

	// TODO: Review
	TextureSubresourceInfo subresource;
	subresource = TextureSubresourceInfo(TextureSurfaceInfo(0, 0, 0, 0));
	TextureViewPtr tmpView =
		getGrManager().newTextureView(TextureViewInitInfo(m_exposure1x1, subresource, "ExposureTmpView"));
	cmdb->copyBufferToTextureView(handle.getBuffer(), handle.getOffset(), sizeof(F32), tmpView);

	FencePtr fence;
	cmdb->flush({}, &fence);

	m_r->getResourceManager().getTransferGpuAllocator().release(handle, fence);

	return Error::NONE;
}

void Tonemapping::importRenderTargets(RenderingContext& ctx)
{
	// Computation of the AVG luminance will run first in the frame and it will use the m_luminanceBuff as storage
	// read/write. To skip the barrier import it as read/write as well.
	m_runCtx.m_buffHandle = ctx.m_renderGraphDescr.importBuffer(
		m_luminanceBuff, BufferUsageBit::STORAGE_COMPUTE_READ | BufferUsageBit::STORAGE_COMPUTE_WRITE);
	m_runCtx.m_exposureHandle = ctx.m_renderGraphDescr.importRenderTarget(
		m_exposure1x1, TextureUsageBit::IMAGE_COMPUTE_READ | TextureUsageBit::IMAGE_COMPUTE_WRITE);
}

void Tonemapping::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create the pass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Avg lum");

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

		cmdb->bindShaderProgram(m_grProg);
		rgraphCtx.bindStorageBuffer(0, 1, m_runCtx.m_buffHandle);
		rgraphCtx.bindImage(0, 2, m_runCtx.m_exposureHandle);

		TextureSubresourceInfo inputTexSubresource;
		inputTexSubresource.m_firstMipmap = m_inputTexMip;
		rgraphCtx.bindTexture(0, 0, m_r->getDownscaleBlur().getRt(), inputTexSubresource);

		cmdb->dispatchCompute(1, 1, 1);
	});

	pass.newDependency(
		{m_runCtx.m_buffHandle, BufferUsageBit::STORAGE_COMPUTE_READ | BufferUsageBit::STORAGE_COMPUTE_WRITE});

	TextureSubresourceInfo inputTexSubresource;
	inputTexSubresource.m_firstMipmap = m_inputTexMip;
	pass.newDependency({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE, inputTexSubresource});
}

} // end namespace anki

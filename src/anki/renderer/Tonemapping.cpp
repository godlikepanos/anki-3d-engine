// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Tonemapping.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Renderer.h>

namespace anki
{

Error Tonemapping::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing tonemapping");
	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize tonemapping");
	}

	return err;
}

Error Tonemapping::initInternal(const ConfigSet& initializer)
{
	m_inputTexMip = m_r->getDownscaleBlur().getMipmapCount() - 2;

	// Create program
	ANKI_CHECK(getResourceManager().loadResource("shaders/TonemappingAverageLuminance.glslp", m_prog));

	ShaderProgramResourceConstantValueInitList<1> consts(m_prog);
	consts.add("INPUT_TEX_SIZE",
		UVec2(
			m_r->getDownscaleBlur().getPassWidth(m_inputTexMip), m_r->getDownscaleBlur().getPassHeight(m_inputTexMip)));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(consts.get(), variant);
	m_grProg = variant->getProgram();

	// Create buffer
	m_luminanceBuff = getGrManager().newBuffer(BufferInitInfo(sizeof(Vec4),
		BufferUsageBit::STORAGE_ALL | BufferUsageBit::UNIFORM_ALL | BufferUsageBit::BUFFER_UPLOAD_DESTINATION,
		BufferMapAccessBit::NONE,
		"AvgLum"));

	CommandBufferInitInfo cmdbinit;
	cmdbinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = getGrManager().newCommandBuffer(cmdbinit);

	TransferGpuAllocatorHandle handle;
	ANKI_CHECK(m_r->getResourceManager().getTransferGpuAllocator().allocate(sizeof(Vec4), handle));
	void* data = handle.getMappedMemory();

	*static_cast<Vec4*>(data) = Vec4(0.5);
	cmdb->copyBufferToBuffer(handle.getBuffer(), handle.getOffset(), m_luminanceBuff, 0, handle.getRange());

	FencePtr fence;
	cmdb->flush(&fence);

	m_r->getResourceManager().getTransferGpuAllocator().release(handle, fence);

	return Error::NONE;
}

void Tonemapping::importRenderTargets(RenderingContext& ctx)
{
	// Computation of the AVG luminance will run first in the frame and it will use the m_luminanceBuff as storage
	// read/write. To skip the barrier import it as read/write as well.
	m_runCtx.m_buffHandle =
		ctx.m_renderGraphDescr.importBuffer(m_luminanceBuff, BufferUsageBit::STORAGE_COMPUTE_READ_WRITE);
}

void Tonemapping::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create the pass
	ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("Avg lum");

	pass.setWork(runCallback, this, 0);

	pass.newDependency({m_runCtx.m_buffHandle, BufferUsageBit::STORAGE_COMPUTE_READ_WRITE});

	TextureSubresourceInfo inputTexSubresource;
	inputTexSubresource.m_firstMipmap = m_inputTexMip;
	pass.newDependency({m_r->getDownscaleBlur().getRt(), TextureUsageBit::SAMPLED_COMPUTE, inputTexSubresource});
}

void Tonemapping::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);
	rgraphCtx.bindStorageBuffer(0, 0, m_runCtx.m_buffHandle);

	TextureSubresourceInfo inputTexSubresource;
	inputTexSubresource.m_firstMipmap = m_inputTexMip;
	rgraphCtx.bindTextureAndSampler(
		0, 0, m_r->getDownscaleBlur().getRt(), inputTexSubresource, m_r->getLinearSampler());

	cmdb->dispatchCompute(1, 1, 1);
}

} // end namespace anki

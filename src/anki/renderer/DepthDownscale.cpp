// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>

namespace anki
{

DepthDownscale::~DepthDownscale()
{
	m_passes.destroy(getAllocator());

	if(m_copyToBuff.m_buffAddr)
	{
		m_copyToBuff.m_buff->unmap();
	}
}

Error DepthDownscale::initInternal(const ConfigSet&)
{
	const U width = m_r->getWidth() / 2;
	const U height = m_r->getHeight() / 2;

	const U mipCount = computeMaxMipmapCount2d(width, height, HIERARCHICAL_Z_MIN_HEIGHT);

	const U lastMipWidth = width >> (mipCount - 1);
	const U lastMipHeight = height >> (mipCount - 1);

	ANKI_R_LOGI("Initializing HiZ. Mip count %u, last mip size %ux%u", mipCount, lastMipWidth, lastMipHeight);

	m_passes.create(getAllocator(), mipCount);

	// Create RT descrs
	m_depthRtDescr =
		m_r->create2DRenderTargetDescription(width, height, GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT, "Half depth");
	m_depthRtDescr.bake();

	m_hizRtDescr = m_r->create2DRenderTargetDescription(width, height, Format::R32_SFLOAT, "HiZ");
	m_hizRtDescr.m_mipmapCount = mipCount;
	m_hizRtDescr.bake();

	// Create FB descr
	m_passes[0].m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_passes[0].m_fbDescr.m_colorAttachmentCount = 1;
	m_passes[0].m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_passes[0].m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_passes[0].m_fbDescr.bake();

	for(U i = 1; i < m_passes.getSize(); ++i)
	{
		m_passes[i].m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_passes[i].m_fbDescr.m_colorAttachments[0].m_surface.m_level = i;
		m_passes[i].m_fbDescr.m_colorAttachmentCount = 1;
		m_passes[i].m_fbDescr.bake();
	}

	// Progs
	ANKI_CHECK(getResourceManager().loadResource("programs/DepthDownscale.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<2> mutations(m_prog);
	mutations.add("TYPE", 0).add("SAMPLE_RESOLVE_TYPE", 1);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), variant);
	m_passes[0].m_grProg = variant->getProgram();

	for(U i = 1; i < m_passes.getSize(); ++i)
	{
		mutations[0].m_value = 1;

		m_prog->getOrCreateVariant(mutations.get(), variant);
		m_passes[i].m_grProg = variant->getProgram();
	}

	// Copy to buffer
	{
		m_copyToBuff.m_lastMipWidth = lastMipWidth;
		m_copyToBuff.m_lastMipHeight = lastMipHeight;

		// Create program
		ANKI_CHECK(getResourceManager().loadResource("programs/HiZCopyToClient.ankiprog", m_copyToBuff.m_prog));

		ShaderProgramResourceConstantValueInitList<1> consts(m_copyToBuff.m_prog);
		consts.add("HIZ_MIP_SIZE", UVec2(lastMipWidth, lastMipHeight));

		const ShaderProgramResourceVariant* variant;
		m_copyToBuff.m_prog->getOrCreateVariant(consts.get(), variant);

		m_copyToBuff.m_grProg = variant->getProgram();

		// Create buffer
		BufferInitInfo buffInit("HiZ Client");
		buffInit.m_access = BufferMapAccessBit::READ;
		buffInit.m_size = lastMipHeight * lastMipWidth * sizeof(F32);
		buffInit.m_usage = BufferUsageBit::STORAGE_COMPUTE_WRITE;
		m_copyToBuff.m_buff = getGrManager().newBuffer(buffInit);

		m_copyToBuff.m_buffAddr = m_copyToBuff.m_buff->map(0, buffInit.m_size, BufferMapAccessBit::READ);
	}

	return Error::NONE;
}

Error DepthDownscale::init(const ConfigSet& cfg)
{
	ANKI_R_LOGI("Initializing depth downscale passes");

	Error err = initInternal(cfg);
	if(err)
	{
		ANKI_R_LOGE("Failed to initialize depth downscale passes");
	}

	return err;
}

void DepthDownscale::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_pass = 0;

	static const Array<CString, 5> passNames = {{"HiZ #0", "HiZ #1", "HiZ #2", "HiZ #3", "HiZ #4"}};

	// Create render targets
	m_runCtx.m_halfDepthRt = rgraph.newRenderTarget(m_depthRtDescr);
	m_runCtx.m_hizRt = rgraph.newRenderTarget(m_hizRtDescr);

	// First render pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[0]);

		pass.setFramebufferInfo(m_passes[0].m_fbDescr, {{m_runCtx.m_hizRt}}, m_runCtx.m_halfDepthRt);
		pass.setWork(runCallback, this, 0);

		TextureSubresourceInfo subresource = TextureSubresourceInfo(DepthStencilAspectBit::DEPTH); // First mip

		pass.newConsumer({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT, subresource});
		pass.newConsumer({m_runCtx.m_halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresource});

		pass.newProducer({m_runCtx.m_halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, subresource});
		pass.newProducer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}

	// Rest of the passes
	for(U i = 1; i < m_passes.getSize(); ++i)
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass(passNames[i]);

		pass.setFramebufferInfo(m_passes[i].m_fbDescr, {{m_runCtx.m_hizRt}}, {});
		pass.setWork(runCallback, this, 0);

		TextureSubresourceInfo subresourceRead, subresourceWrite;
		subresourceRead.m_firstMipmap = i - 1;
		subresourceWrite.m_firstMipmap = i;

		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::SAMPLED_FRAGMENT, subresourceRead});
		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresourceWrite});

		pass.newProducer({m_runCtx.m_hizRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE, subresourceWrite});
	}

	// Copy to buffer pass
	{
		ComputeRenderPassDescription& pass = rgraph.newComputeRenderPass("HiZ Copy");

		pass.setWork(runCopyToBufferCallback, this, 0);

		TextureSubresourceInfo subresource;
		subresource.m_firstMipmap = getMipmapCount() - 1;

		pass.newConsumer({m_runCtx.m_hizRt, TextureUsageBit::SAMPLED_COMPUTE, subresource});
	}
}

void DepthDownscale::run(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	const U passIdx = m_runCtx.m_pass++;

	cmdb->setViewport(0, 0, m_r->getWidth() >> (passIdx + 1), m_r->getHeight() >> (passIdx + 1));

	if(passIdx == 0)
	{
		rgraphCtx.bindTextureAndSampler(0,
			0,
			m_r->getGBuffer().getDepthRt(),
			TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
			m_r->getNearestSampler());

		cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
	}
	else
	{
		TextureSubresourceInfo sampleSubresource;
		sampleSubresource.m_firstMipmap = passIdx - 1;

		rgraphCtx.bindTextureAndSampler(0, 0, m_runCtx.m_hizRt, sampleSubresource, m_r->getNearestSampler());
	}

	cmdb->bindShaderProgram(m_passes[passIdx].m_grProg);

	drawQuad(cmdb);

	// Restore state
	if(passIdx == 0)
	{
		cmdb->setDepthCompareOperation(CompareOperation::LESS);
	}
}

void DepthDownscale::runCopyToBuffer(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	TextureSubresourceInfo sampleSubresource;
	sampleSubresource.m_firstMipmap = getMipmapCount() - 1;

	rgraphCtx.bindTextureAndSampler(0, 0, m_runCtx.m_hizRt, sampleSubresource, m_r->getNearestSampler());

	cmdb->bindStorageBuffer(0, 0, m_copyToBuff.m_buff, 0, m_copyToBuff.m_buff->getSize());

	dispatchPPCompute(cmdb, 8, 8, m_copyToBuff.m_lastMipWidth, m_copyToBuff.m_lastMipHeight);
}

} // end namespace anki

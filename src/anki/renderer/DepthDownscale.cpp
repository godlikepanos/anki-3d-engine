// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
}

Error DepthDownscale::initHalf(const ConfigSet&)
{
	U width = m_r->getWidth() / 2;
	U height = m_r->getHeight() / 2;

	// Create RT descrs
	m_half.m_depthRtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		"Half depth");
	m_half.m_depthRtDescr.bake();

	m_half.m_colorRtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		PixelFormat(ComponentFormat::R32, TransformFormat::FLOAT),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		"Half depth col");
	m_half.m_colorRtDescr.bake();

	// Create FB descr
	m_half.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_half.m_fbDescr.m_colorAttachmentCount = 1;
	m_half.m_fbDescr.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_half.m_fbDescr.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_half.m_fbDescr.bake();

	// Prog
	ANKI_CHECK(getResourceManager().loadResource("programs/DepthDownscale.ankiprog", m_half.m_prog));

	ShaderProgramResourceMutationInitList<2> mutations(m_half.m_prog);
	mutations.add("TYPE", 0).add("SAMPLE_RESOLVE_TYPE", 0);

	const ShaderProgramResourceVariant* variant;
	m_half.m_prog->getOrCreateVariant(mutations.get(), variant);
	m_half.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error DepthDownscale::initQuarter(const ConfigSet&)
{
	U width = m_r->getWidth() / 4;
	U height = m_r->getHeight() / 4;

	// RT descr
	m_quarter.m_colorRtDescr = m_r->create2DRenderTargetDescription(width,
		height,
		PixelFormat(ComponentFormat::R32, TransformFormat::FLOAT),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		"quarterdepth");
	m_quarter.m_colorRtDescr.bake();

	// FB descr
	m_quarter.m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_quarter.m_fbDescr.m_colorAttachmentCount = 1;
	m_quarter.m_fbDescr.bake();

	// Prog
	ANKI_CHECK(getResourceManager().loadResource("programs/DepthDownscale.ankiprog", m_quarter.m_prog));

	ShaderProgramResourceMutationInitList<2> mutations(m_quarter.m_prog);
	mutations.add("TYPE", 1).add("SAMPLE_RESOLVE_TYPE", 0);

	const ShaderProgramResourceVariant* variant;
	m_quarter.m_prog->getOrCreateVariant(mutations.get(), variant);
	m_quarter.m_grProg = variant->getProgram();

	return Error::NONE;
}

Error DepthDownscale::initInternal(const ConfigSet& cfg)
{
	ANKI_CHECK(initHalf(cfg));
	ANKI_CHECK(initQuarter(cfg));
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

void DepthDownscale::runHalf(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_half.m_grProg);
	rgraphCtx.bindTexture(0, 0, m_r->getGBuffer().getDepthRt());

	cmdb->setViewport(0, 0, m_r->getWidth() / 2, m_r->getHeight() / 2);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);

	drawQuad(cmdb);

	// Restore state
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
}

void DepthDownscale::runQuarter(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->bindShaderProgram(m_quarter.m_grProg);
	rgraphCtx.bindTexture(0, 0, m_runCtx.m_halfColorRt);
	cmdb->setViewport(0, 0, m_r->getWidth() / 4, m_r->getHeight() / 4);

	drawQuad(cmdb);
}

void DepthDownscale::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create render targets
	m_runCtx.m_halfDepthRt = rgraph.newRenderTarget(m_half.m_depthRtDescr);
	m_runCtx.m_halfColorRt = rgraph.newRenderTarget(m_half.m_colorRtDescr);
	m_runCtx.m_quarterRt = rgraph.newRenderTarget(m_quarter.m_colorRtDescr);

	// Create half depth render pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Half depth");

		pass.setFramebufferInfo(m_half.m_fbDescr, {{m_runCtx.m_halfColorRt}}, m_runCtx.m_halfDepthRt);
		pass.setWork(runHalfCallback, this, 0);

		pass.newConsumer(
			{m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT, DepthStencilAspectBit::DEPTH});
		pass.newConsumer({m_runCtx.m_halfColorRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newConsumer(
			{m_runCtx.m_halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, DepthStencilAspectBit::DEPTH});
		pass.newProducer({m_runCtx.m_halfColorRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer(
			{m_runCtx.m_halfDepthRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE, DepthStencilAspectBit::DEPTH});
	}

	// Create quarter depth render pass
	{
		GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Quarter depth");

		pass.setFramebufferInfo(m_quarter.m_fbDescr, {{m_runCtx.m_quarterRt}}, {});
		pass.setWork(runQuarterCallback, this, 0);

		pass.newConsumer({m_runCtx.m_halfColorRt, TextureUsageBit::SAMPLED_FRAGMENT});
		pass.newConsumer({m_runCtx.m_quarterRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newProducer({m_runCtx.m_quarterRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	}
}

} // end namespace anki

// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>

namespace anki
{

HalfDepth::~HalfDepth()
{
}

Error HalfDepth::init(const ConfigSet&)
{
	GrManager& gr = getGrManager();
	U width = m_r->getWidth() / 2;
	U height = m_r->getHeight() / 2;

	// Create RTs
	m_depthRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(width,
		height,
		GBUFFER_DEPTH_ATTACHMENT_PIXEL_FORMAT,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		SamplingFilter::LINEAR,
		1,
		"halfdepth"));

	m_colorRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(width,
		height,
		PixelFormat(ComponentFormat::R32, TransformFormat::FLOAT),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		1,
		"halfdepthcol"));

	// Create FB
	FramebufferInitInfo fbInit("halfdepth");
	fbInit.m_colorAttachments[0].m_texture = m_colorRt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachmentCount = 1;
	fbInit.m_depthStencilAttachment.m_texture = m_depthRt;
	fbInit.m_depthStencilAttachment.m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_depthStencilAttachment.m_aspect = DepthStencilAspectBit::DEPTH;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// Prog
	ANKI_CHECK(getResourceManager().loadResource("programs/DepthDownscale.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<2> mutations(m_prog);
	mutations.add("TYPE", 0).add("SAMPLE_RESOLVE_TYPE", 0);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void HalfDepth::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_depthRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));

	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_colorRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void HalfDepth::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_colorRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void HalfDepth::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_grProg);
	cmdb->bindTexture(0, 0, m_r->getGBuffer().m_depthRt);

	cmdb->setViewport(0, 0, m_r->getWidth() / 2, m_r->getHeight() / 2);
	cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();

	// Restore state
	cmdb->setDepthCompareOperation(CompareOperation::LESS);
}

QuarterDepth::~QuarterDepth()
{
}

Error QuarterDepth::init(const ConfigSet&)
{
	GrManager& gr = getGrManager();
	U width = m_r->getWidth() / 4;
	U height = m_r->getHeight() / 4;

	m_colorRt = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(width,
		height,
		PixelFormat(ComponentFormat::R32, TransformFormat::FLOAT),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE | TextureUsageBit::SAMPLED_FRAGMENT,
		SamplingFilter::LINEAR,
		1,
		"quarterdepth"));

	FramebufferInitInfo fbInit("quarterdepth");
	fbInit.m_colorAttachments[0].m_texture = m_colorRt;
	fbInit.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	fbInit.m_colorAttachmentCount = 1;
	m_fb = gr.newInstance<Framebuffer>(fbInit);

	// Prog
	ANKI_CHECK(getResourceManager().loadResource("programs/DepthDownscale.ankiprog", m_prog));

	ShaderProgramResourceMutationInitList<2> mutations(m_prog);
	mutations.add("TYPE", 1).add("SAMPLE_RESOLVE_TYPE", 0);

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), variant);
	m_grProg = variant->getProgram();

	return Error::NONE;
}

void QuarterDepth::setPreRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_colorRt,
		TextureUsageBit::NONE,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void QuarterDepth::setPostRunBarriers(RenderingContext& ctx)
{
	ctx.m_commandBuffer->setTextureSurfaceBarrier(m_colorRt,
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));
}

void QuarterDepth::run(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->beginRenderPass(m_fb);
	cmdb->bindShaderProgram(m_grProg);
	cmdb->bindTexture(0, 0, m_parent->m_hd.m_colorRt);

	cmdb->setViewport(0, 0, m_r->getWidth() / 4, m_r->getHeight() / 4);

	m_r->drawQuad(cmdb);

	cmdb->endRenderPass();
}

DepthDownscale::~DepthDownscale()
{
}

Error DepthDownscale::initInternal(const ConfigSet& cfg)
{
	ANKI_CHECK(m_hd.init(cfg));
	ANKI_CHECK(m_qd.init(cfg));
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

} // end namespace anki

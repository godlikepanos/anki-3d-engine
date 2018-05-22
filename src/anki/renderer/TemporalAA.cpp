// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/Tonemapping.h>

namespace anki
{

TemporalAA::TemporalAA(Renderer* r)
	: RendererObject(r)
{
}

TemporalAA::~TemporalAA()
{
}

Error TemporalAA::init(const ConfigSet& config)
{
	ANKI_R_LOGI("Initializing TAA");
	Error err = initInternal(config);

	if(err)
	{
		ANKI_R_LOGE("Failed to init TAA");
	}

	return Error::NONE;
}

Error TemporalAA::initInternal(const ConfigSet& config)
{
	ANKI_CHECK(m_r->getResourceManager().loadResource("programs/TemporalAAResolve.ankiprog", m_prog));

	for(U i = 0; i < 2; ++i)
	{
		ShaderProgramResourceConstantValueInitList<2> consts(m_prog);
		consts.add("VARIANCE_CLIPPING_GAMMA", 1.7f).add("BLEND_FACTOR", 1.0f / 16.0f);

		ShaderProgramResourceMutationInitList<4> mutations(m_prog);
		mutations.add("SHARPEN", i + 1).add("VARIANCE_CLIPPING", 1).add("TONEMAP_FIX", 1).add("YCBCR", 0);

		const ShaderProgramResourceVariant* variant;
		m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
		m_grProgs[i] = variant->getProgram();
	}

	for(U i = 0; i < 2; ++i)
	{
		m_rtTextures[i] = m_r->createAndClearRenderTarget(m_r->create2DRenderTargetInitInfo(m_r->getWidth(),
			m_r->getHeight(),
			LIGHT_SHADING_COLOR_ATTACHMENT_PIXEL_FORMAT,
			TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE
				| TextureUsageBit::SAMPLED_COMPUTE,
			"TemporalAA"));
	}

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	return Error::NONE;
}

void TemporalAA::run(const RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;

	cmdb->setViewport(0, 0, m_r->getWidth(), m_r->getHeight());

	cmdb->bindShaderProgram(m_grProgs[m_r->getFrameCount() & 1]);
	rgraphCtx.bindTextureAndSampler(0,
		0,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getLinearSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getLightShading().getRt(), m_r->getLinearSampler());
	rgraphCtx.bindColorTextureAndSampler(0, 2, m_runCtx.m_historyRt, m_r->getLinearSampler());

	Mat4* unis = allocateAndBindUniforms<Mat4*>(sizeof(Mat4), cmdb, 0, 0);
	*unis = ctx.m_jitterMat * ctx.m_prevViewProjMat * ctx.m_viewProjMatJitter.getInverse();

	rgraphCtx.bindUniformBuffer(0, 1, m_r->getTonemapping().getAverageLuminanceBuffer());

	drawQuad(cmdb);
}

void TemporalAA::populateRenderGraph(RenderingContext& ctx)
{
	m_runCtx.m_ctx = &ctx;
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Import RTs
	m_runCtx.m_historyRt =
		rgraph.importRenderTarget(m_rtTextures[(m_r->getFrameCount() + 1) & 1], TextureUsageBit::SAMPLED_FRAGMENT);
	m_runCtx.m_renderRt = rgraph.importRenderTarget(m_rtTextures[m_r->getFrameCount() & 1], TextureUsageBit::NONE);

	// Create pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("TemporalAA");

	pass.setWork(runCallback, this, 0);
	pass.setFramebufferInfo(m_fbDescr, {{m_runCtx.m_renderRt}}, {});

	pass.newConsumer({m_runCtx.m_renderRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
	pass.newConsumer({m_r->getGBuffer().getDepthRt(),
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH)});
	pass.newConsumer({m_r->getLightShading().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newConsumer({m_runCtx.m_historyRt, TextureUsageBit::SAMPLED_FRAGMENT});

	pass.newProducer({m_runCtx.m_renderRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
}

} // end namespace anki

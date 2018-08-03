// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/Renderer.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/TemporalAA.h>
#include <anki/renderer/Tonemapping.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Ssr.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/UiStage.h>
#include <anki/util/Logger.h>
#include <anki/misc/ConfigSet.h>

namespace anki
{

FinalComposite::FinalComposite(Renderer* r)
	: RendererObject(r)
{
}

FinalComposite::~FinalComposite()
{
}

Error FinalComposite::initInternal(const ConfigSet& config)
{
	ANKI_ASSERT("Initializing PPS");

	ANKI_CHECK(loadColorGradingTexture("engine_data/DefaultLut.ankitex"));

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
	m_fbDescr.bake();

	ANKI_CHECK(getResourceManager().loadResource("engine_data/BlueNoiseLdrRgb64x64.ankitex", m_blueNoise));

	// Progs
	ANKI_CHECK(getResourceManager().loadResource("shaders/FinalComposite.glslp", m_prog));

	ShaderProgramResourceMutationInitList<3> mutations(m_prog);
	mutations.add("BLUE_NOISE", 1).add("BLOOM_ENABLED", 1).add("DBG_ENABLED", 0);

	ShaderProgramResourceConstantValueInitList<3> consts(m_prog);
	consts.add("LUT_SIZE", U32(LUT_SIZE))
		.add("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()))
		.add("MOTION_BLUR_SAMPLES", U32(config.getNumber("r.final.motionBlurSamples")));

	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
	m_grProgs[0] = variant->getProgram();

	mutations[2].m_value = 1;
	m_prog->getOrCreateVariant(mutations.get(), consts.get(), variant);
	m_grProgs[1] = variant->getProgram();

	return Error::NONE;
}

Error FinalComposite::init(const ConfigSet& config)
{
	Error err = initInternal(config);
	if(err)
	{
		ANKI_R_LOGE("Failed to init PPS");
	}

	return err;
}

Error FinalComposite::loadColorGradingTexture(CString filename)
{
	m_lut.reset(nullptr);
	ANKI_CHECK(getResourceManager().loadResource(filename, m_lut));
	ANKI_ASSERT(m_lut->getWidth() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getHeight() == LUT_SIZE);
	ANKI_ASSERT(m_lut->getDepth() == LUT_SIZE);

	return Error::NONE;
}

void FinalComposite::run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const Bool dbgEnabled = m_r->getDbg().getEnabled();

	cmdb->bindShaderProgram(m_grProgs[dbgEnabled]);

	// Bind stuff
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_r->getTemporalAA().getRt(), m_r->getLinearSampler());

	rgraphCtx.bindColorTextureAndSampler(0, 1, m_r->getBloom().getRt(), m_r->getLinearSampler());
	cmdb->bindTextureAndSampler(
		0, 2, m_lut->getGrTextureView(), m_r->getLinearSampler(), TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTextureAndSampler(
		0, 3, m_blueNoise->getGrTextureView(), m_blueNoise->getSampler(), TextureUsageBit::SAMPLED_FRAGMENT);
	rgraphCtx.bindColorTextureAndSampler(0, 4, m_r->getGBuffer().getColorRt(3), m_r->getNearestSampler());
	rgraphCtx.bindTextureAndSampler(0,
		5,
		m_r->getGBuffer().getDepthRt(),
		TextureSubresourceInfo(DepthStencilAspectBit::DEPTH),
		m_r->getNearestSampler());

	if(dbgEnabled)
	{
		rgraphCtx.bindColorTextureAndSampler(0, 6, m_r->getDbg().getRt(), m_r->getLinearSampler());
	}

	rgraphCtx.bindUniformBuffer(0, 1, m_r->getTonemapping().getAverageLuminanceBuffer());

	struct PushConsts
	{
		Vec4 m_blueNoiseLayerPad3;
		Mat4 m_prevViewProjMatMulInvViewProjMat;
	} pconsts;
	pconsts.m_blueNoiseLayerPad3.x() = F32(m_r->getFrameCount() % m_blueNoise->getLayerCount());
	pconsts.m_prevViewProjMatMulInvViewProjMat = ctx.m_matrices.m_jitter * ctx.m_prevMatrices.m_viewProjection
												 * ctx.m_matrices.m_viewProjectionJitter.getInverse();
	cmdb->setPushConstants(&pconsts, sizeof(pconsts));

	cmdb->setViewport(0, 0, ctx.m_outRenderTargetWidth, ctx.m_outRenderTargetHeight);

	drawQuad(cmdb);

	// Draw UI
	m_r->getUiStage().draw(ctx, cmdb);
}

void FinalComposite::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create the pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Final Composite");

	pass.setWork(runCallback, this, 0);
	pass.setFramebufferInfo(m_fbDescr, {{ctx.m_outRenderTarget}}, {});

	pass.newDependency({ctx.m_outRenderTarget, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

	if(m_r->getDbg().getEnabled())
	{
		pass.newDependency({m_r->getDbg().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	}

	pass.newDependency({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getBloom().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	pass.newDependency({m_r->getGBuffer().getColorRt(3), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT});
}

} // end namespace anki

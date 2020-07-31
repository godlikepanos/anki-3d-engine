// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
#include <anki/core/ConfigSet.h>

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
	ANKI_CHECK(getResourceManager().loadResource("shaders/FinalComposite.ankiprog", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("BLUE_NOISE", 1);
	variantInitInfo.addMutation("BLOOM_ENABLED", 1);
	variantInitInfo.addConstant("LUT_SIZE", U32(LUT_SIZE));
	variantInitInfo.addConstant("LUT_SIZE", U32(LUT_SIZE));
	variantInitInfo.addConstant("FB_SIZE", UVec2(m_r->getWidth(), m_r->getHeight()));
	variantInitInfo.addConstant("MOTION_BLUR_SAMPLES", config.getNumberU32("r_motionBlurSamples"));

	for(U32 dbg = 0; dbg < 2; ++dbg)
	{
		for(U32 dbgRt = 0; dbgRt < 2; ++dbgRt)
		{
			const ShaderProgramResourceVariant* variant;
			variantInitInfo.addMutation("DBG_ENABLED", dbg);
			variantInitInfo.addMutation("DBG_RENDER_TARGET_ENABLED", dbgRt);
			m_prog->getOrCreateVariant(variantInitInfo, variant);
			m_grProgs[dbg][dbgRt] = variant->getProgram();
		}
	}

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
	RenderTargetHandle dbgRt;
	Bool dbgRtValid;
	m_r->getCurrentDebugRenderTarget(dbgRt, dbgRtValid);

	cmdb->bindShaderProgram(m_grProgs[dbgEnabled][dbgRtValid]);

	// Bind stuff
	rgraphCtx.bindUniformBuffer(0, 0, m_r->getTonemapping().getAverageLuminanceBuffer());

	cmdb->bindSampler(0, 1, m_r->getSamplers().m_nearestNearestClamp);
	cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearClamp);
	cmdb->bindSampler(0, 3, m_r->getSamplers().m_trilinearRepeat);

	rgraphCtx.bindColorTexture(0, 4, m_r->getTemporalAA().getRt());

	rgraphCtx.bindColorTexture(0, 5, m_r->getBloom().getRt());
	cmdb->bindTexture(0, 6, m_lut->getGrTextureView(), TextureUsageBit::SAMPLED_FRAGMENT);
	cmdb->bindTexture(0, 7, m_blueNoise->getGrTextureView(), TextureUsageBit::SAMPLED_FRAGMENT);
	rgraphCtx.bindColorTexture(0, 8, m_r->getGBuffer().getColorRt(3));
	rgraphCtx.bindTexture(0, 9, m_r->getGBuffer().getDepthRt(), TextureSubresourceInfo(DepthStencilAspectBit::DEPTH));

	if(dbgEnabled)
	{
		rgraphCtx.bindColorTexture(0, 10, m_r->getDbg().getRt());
	}

	if(dbgRtValid)
	{
		rgraphCtx.bindColorTexture(0, 11, dbgRt);
	}

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
	m_r->getUiStage().draw(ctx.m_outRenderTargetWidth, ctx.m_outRenderTargetHeight, ctx, cmdb);
}

void FinalComposite::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;
	m_runCtx.m_ctx = &ctx;

	// Create the pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Final Composite");

	pass.setWork(
		[](RenderPassWorkContext& rgraphCtx) {
			FinalComposite* self = static_cast<FinalComposite*>(rgraphCtx.m_userData);
			self->run(*self->m_runCtx.m_ctx, rgraphCtx);
		},
		this, 0);
	pass.setFramebufferInfo(m_fbDescr, {ctx.m_outRenderTarget}, {});

	pass.newDependency({ctx.m_outRenderTarget, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});

	if(m_r->getDbg().getEnabled())
	{
		pass.newDependency({m_r->getDbg().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	}

	pass.newDependency({m_r->getTemporalAA().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getBloom().getRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	pass.newDependency({m_r->getGBuffer().getColorRt(3), TextureUsageBit::SAMPLED_FRAGMENT});
	pass.newDependency({m_r->getGBuffer().getDepthRt(), TextureUsageBit::SAMPLED_FRAGMENT});

	RenderTargetHandle dbgRt;
	Bool dbgRtValid;
	m_r->getCurrentDebugRenderTarget(dbgRt, dbgRtValid);
	if(dbgRtValid)
	{
		pass.newDependency({dbgRt, TextureUsageBit::SAMPLED_FRAGMENT});
	}
}

} // end namespace anki

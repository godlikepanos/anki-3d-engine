// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/Scale.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/DownscaleBlur.h>
#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/MotionVectors.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

FinalComposite::FinalComposite(Renderer* r)
	: RendererObject(r)
{
}

FinalComposite::~FinalComposite()
{
}

Error FinalComposite::initInternal()
{
	ANKI_R_LOGV("Initializing final composite");

	ANKI_CHECK(loadColorGradingTextureImage("EngineAssets/DefaultLut.ankitex"));

	m_fbDescr.m_colorAttachmentCount = 1;
	m_fbDescr.bake();

	// Progs
	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/FinalComposite.ankiprogbin", m_prog));

	ShaderProgramResourceVariantInitInfo variantInitInfo(m_prog);
	variantInitInfo.addMutation("FILM_GRAIN", (getConfig().getRFilmGrainStrength() > 0.0) ? 1 : 0);
	variantInitInfo.addMutation("BLOOM_ENABLED", 1);
	variantInitInfo.addConstant("kLutSize", U32(kLutSize));
	variantInitInfo.addConstant("kFramebufferSize", m_r->getPostProcessResolution());
	variantInitInfo.addConstant("kMotionBlurSamples", getConfig().getRMotionBlurSamples());

	for(U32 dbg = 0; dbg < 2; ++dbg)
	{
		const ShaderProgramResourceVariant* variant;
		variantInitInfo.addMutation("DBG_ENABLED", dbg);
		m_prog->getOrCreateVariant(variantInitInfo, variant);
		m_grProgs[dbg] = variant->getProgram();
	}

	ANKI_CHECK(getResourceManager().loadResource("ShaderBinaries/VisualizeRenderTarget.ankiprogbin",
												 m_defaultVisualizeRenderTargetProg));
	const ShaderProgramResourceVariant* variant;
	m_defaultVisualizeRenderTargetProg->getOrCreateVariant(variant);
	m_defaultVisualizeRenderTargetGrProg = variant->getProgram();

	return Error::kNone;
}

Error FinalComposite::init()
{
	const Error err = initInternal();
	if(err)
	{
		ANKI_R_LOGE("Failed to init final composite");
	}

	return err;
}

Error FinalComposite::loadColorGradingTextureImage(CString filename)
{
	m_lut.reset(nullptr);
	ANKI_CHECK(getResourceManager().loadResource(filename, m_lut));
	ANKI_ASSERT(m_lut->getWidth() == kLutSize);
	ANKI_ASSERT(m_lut->getHeight() == kLutSize);
	ANKI_ASSERT(m_lut->getDepth() == kLutSize);

	return Error::kNone;
}

void FinalComposite::populateRenderGraph(RenderingContext& ctx)
{
	RenderGraphDescription& rgraph = ctx.m_renderGraphDescr;

	// Create the pass
	GraphicsRenderPassDescription& pass = rgraph.newGraphicsRenderPass("Final Composite");

	pass.setWork([this, &ctx](RenderPassWorkContext& rgraphCtx) {
		run(ctx, rgraphCtx);
	});
	pass.setFramebufferInfo(m_fbDescr, {ctx.m_outRenderTarget});

	pass.newTextureDependency(ctx.m_outRenderTarget, TextureUsageBit::kFramebufferWrite);

	if(getConfig().getRDbgEnabled())
	{
		pass.newTextureDependency(m_r->getDbg().getRt(), TextureUsageBit::kSampledFragment);
	}

	pass.newTextureDependency(m_r->getScale().getTonemappedRt(), TextureUsageBit::kSampledFragment);
	pass.newTextureDependency(m_r->getBloom().getRt(), TextureUsageBit::kSampledFragment);
	pass.newTextureDependency(m_r->getMotionVectors().getMotionVectorsRt(), TextureUsageBit::kSampledFragment);
	pass.newTextureDependency(m_r->getGBuffer().getDepthRt(), TextureUsageBit::kSampledFragment);

	Array<RenderTargetHandle, kMaxDebugRenderTargets> dbgRts;
	ShaderProgramPtr debugProgram;
	const Bool hasDebugRt = m_r->getCurrentDebugRenderTarget(dbgRts, debugProgram);
	if(hasDebugRt)
	{
		for(const RenderTargetHandle& handle : dbgRts)
		{
			if(handle.isValid())
			{
				pass.newTextureDependency(handle, TextureUsageBit::kSampledFragment);
			}
		}
	}
}

void FinalComposite::run(RenderingContext& ctx, RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	const Bool dbgEnabled = getConfig().getRDbgEnabled();

	Array<RenderTargetHandle, kMaxDebugRenderTargets> dbgRts;
	ShaderProgramPtr optionalDebugProgram;
	const Bool hasDebugRt = m_r->getCurrentDebugRenderTarget(dbgRts, optionalDebugProgram);

	// Bind program
	if(hasDebugRt && optionalDebugProgram.isCreated())
	{
		cmdb->bindShaderProgram(optionalDebugProgram);
	}
	else if(hasDebugRt)
	{
		cmdb->bindShaderProgram(m_defaultVisualizeRenderTargetGrProg);
	}
	else
	{
		cmdb->bindShaderProgram(m_grProgs[dbgEnabled]);
	}

	// Bind stuff
	if(!hasDebugRt)
	{
		cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);
		cmdb->bindSampler(0, 1, m_r->getSamplers().m_trilinearClamp);
		cmdb->bindSampler(0, 2, m_r->getSamplers().m_trilinearRepeat);

		rgraphCtx.bindColorTexture(0, 3, m_r->getScale().getTonemappedRt());

		rgraphCtx.bindColorTexture(0, 4, m_r->getBloom().getRt());
		cmdb->bindTexture(0, 5, m_lut->getTextureView());
		rgraphCtx.bindColorTexture(0, 6, m_r->getMotionVectors().getMotionVectorsRt());
		rgraphCtx.bindTexture(0, 7, m_r->getGBuffer().getDepthRt(),
							  TextureSubresourceInfo(DepthStencilAspectBit::kDepth));

		if(dbgEnabled)
		{
			rgraphCtx.bindColorTexture(0, 8, m_r->getDbg().getRt());
		}

		const UVec4 pc(0, 0, floatBitsToUint(getConfig().getRFilmGrainStrength()), m_r->getFrameCount() & kMaxU32);
		cmdb->setPushConstants(&pc, sizeof(pc));
	}
	else
	{
		cmdb->bindSampler(0, 0, m_r->getSamplers().m_nearestNearestClamp);

		U32 count = 1;
		for(const RenderTargetHandle& handle : dbgRts)
		{
			if(handle.isValid())
			{
				rgraphCtx.bindColorTexture(0, count++, handle);
			}
		}
	}

	cmdb->setViewport(0, 0, m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y());
	drawQuad(cmdb);

	// Draw UI
	m_r->getUiStage().draw(m_r->getPostProcessResolution().x(), m_r->getPostProcessResolution().y(), ctx, cmdb);
}

} // end namespace anki

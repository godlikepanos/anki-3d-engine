// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/MotionBlur.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/CVarSet.h>

namespace anki {

Error FinalComposite::initInternal()
{
	ANKI_R_LOGV("Initializing final composite");

	ANKI_CHECK(loadColorGradingTextureImage("EngineAssets/DefaultLut.ankitex"));

	// Progs
	for(MutatorValue dbg = 0; dbg < 2; ++dbg)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/FinalComposite.ankiprogbin",
									 {{"FILM_GRAIN", (g_filmGrainStrengthCVar > 0.0) ? 1 : 0}, {"BLOOM_ENABLED", 1}, {"DBG_ENABLED", dbg}}, m_prog,
									 m_grProgs[dbg]));
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VisualizeRenderTarget.ankiprogbin", m_defaultVisualizeRenderTargetProg,
								 m_defaultVisualizeRenderTargetGrProg));

	if(getRenderer().getSwapchainResolution() != getRenderer().getPostProcessResolution())
	{
		m_rtDesc = getRenderer().create2DRenderTargetDescription(
			getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y(),
			(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm : Format::kR8G8B8A8_Unorm,
			"Final Composite");
		m_rtDesc.bake();
	}

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
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, m_lut));
	ANKI_ASSERT(m_lut->getWidth() == m_lut->getHeight());
	ANKI_ASSERT(m_lut->getWidth() == m_lut->getDepth());

	return Error::kNone;
}

void FinalComposite::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RFinalComposite);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create the pass
	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Final Composite");

	const Bool bRendersToSwapchain = getRenderer().getSwapchainResolution() == getRenderer().getPostProcessResolution();
	const RenderTargetHandle outRt = (!bRendersToSwapchain) ? rgraph.newRenderTarget(m_rtDesc) : ctx.m_swapchainRenderTarget;

	if(!bRendersToSwapchain)
	{
		m_runCtx.m_rt = outRt;
	}

	pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(outRt)});

	pass.newTextureDependency(outRt, TextureUsageBit::kRtvDsvWrite);

	if(g_dbgCVar)
	{
		pass.newTextureDependency(getRenderer().getDbg().getRt(), TextureUsageBit::kSrvPixel);
	}

	pass.newTextureDependency((g_motionBlurSampleCountCVar != 0) ? getRenderer().getMotionBlur().getRt() : getRenderer().getScale().getTonemappedRt(),
							  TextureUsageBit::kSrvPixel);
	pass.newTextureDependency(getRenderer().getBloom().getBloomRt(), TextureUsageBit::kSrvPixel);

	Array<RenderTargetHandle, kMaxDebugRenderTargets> dbgRts;
	ShaderProgramPtr debugProgram;
	const Bool hasDebugRt = getRenderer().getCurrentDebugRenderTarget(dbgRts, debugProgram);
	if(hasDebugRt)
	{
		for(const RenderTargetHandle& handle : dbgRts)
		{
			if(handle.isValid())
			{
				pass.newTextureDependency(handle, TextureUsageBit::kSrvPixel);
			}
		}
	}

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(FinalComposite);

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
		const Bool dbgEnabled = g_dbgCVar;

		Array<RenderTargetHandle, kMaxDebugRenderTargets> dbgRts;
		ShaderProgramPtr optionalDebugProgram;
		const Bool hasDebugRt = getRenderer().getCurrentDebugRenderTarget(dbgRts, optionalDebugProgram);

		// Bind program
		if(hasDebugRt && optionalDebugProgram.isCreated())
		{
			cmdb.bindShaderProgram(optionalDebugProgram.get());
		}
		else if(hasDebugRt)
		{
			cmdb.bindShaderProgram(m_defaultVisualizeRenderTargetGrProg.get());
		}
		else
		{
			cmdb.bindShaderProgram(m_grProgs[dbgEnabled].get());
		}

		// Bind stuff
		if(!hasDebugRt)
		{
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());
			cmdb.bindSampler(1, 0, getRenderer().getSamplers().m_trilinearRepeat.get());

			rgraphCtx.bindSrv(
				0, 0, (g_motionBlurSampleCountCVar != 0) ? getRenderer().getMotionBlur().getRt() : getRenderer().getScale().getTonemappedRt());

			rgraphCtx.bindSrv(1, 0, getRenderer().getBloom().getBloomRt());
			cmdb.bindSrv(2, 0, TextureView(&m_lut->getTexture(), TextureSubresourceDesc::all()));

			if(dbgEnabled)
			{
				rgraphCtx.bindSrv(3, 0, getRenderer().getDbg().getRt());
			}

			const UVec4 pc(floatBitsToUint(g_filmGrainStrengthCVar), getRenderer().getFrameCount() & kMaxU32, 0, 0);
			cmdb.setFastConstants(&pc, sizeof(pc));
		}
		else
		{
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

			U32 count = 0;
			for(const RenderTargetHandle& handle : dbgRts)
			{
				if(handle.isValid())
				{
					rgraphCtx.bindSrv(count++, 0, handle);
				}
			}
		}

		cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
		drawQuad(cmdb);

		// Draw UI
		const Bool bRendersToSwapchain = getRenderer().getSwapchainResolution() == getRenderer().getPostProcessResolution();
		if(bRendersToSwapchain)
		{
			getRenderer().getUiStage().draw(getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y(), cmdb);
		}
	});
}

} // end namespace anki

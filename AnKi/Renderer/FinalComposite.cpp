// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Renderer.h>
#include <AnKi/Renderer/Bloom.h>
#include <AnKi/Renderer/Tonemapping.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/UiStage.h>
#include <AnKi/Renderer/MotionBlur.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/CVarSet.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-function"
#	pragma GCC diagnostic ignored "-Wignored-qualifiers"
#elif ANKI_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable : 4505)
#endif
#define A_CPU
#include <ThirdParty/FidelityFX/ffx_a.h>
#include <ThirdParty/FidelityFX/ffx_fsr1.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#elif ANKI_COMPILER_MSVC
#	pragma warning(pop)
#endif

namespace anki {

Error FinalComposite::initInternal()
{
	// Progs
	for(MutatorValue dbg = 0; dbg < 2; ++dbg)
	{
		ANKI_CHECK(loadShaderProgram("ShaderBinaries/FinalComposite.ankiprogbin",
									 {{"FILM_GRAIN", (g_cvarRenderFilmGrain > 0.0) ? 1 : 0},
									  {"BLOOM", 1},
									  {"DBG", dbg},
									  {"SHARPEN", MutatorValue(g_cvarRenderSharpness > 0.0)}},
									 m_prog, m_grProgs[dbg]));
	}

	ANKI_CHECK(loadShaderProgram("ShaderBinaries/VisualizeRenderTarget.ankiprogbin", m_visualizeRenderTargetProg, m_visualizeRenderTargetGrProg));

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

void FinalComposite::populateRenderGraph(RenderingContext& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(RFinalComposite);

	RenderGraphBuilder& rgraph = ctx.m_renderGraphDescr;

	// Create the pass
	GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("Final Composite");

	const Bool bRendersToSwapchain = getRenderer().getSwapchainResolution() == getRenderer().getPostProcessResolution();
	RenderTargetHandle outRt;
	if(bRendersToSwapchain)
	{
		TexturePtr presentableTex = GrManager::getSingleton().acquireNextPresentableTexture();
		outRt = ctx.m_renderGraphDescr.importRenderTarget(presentableTex.get(), TextureUsageBit::kNone);
		ANKI_ASSERT(!ctx.m_swapchainRenderTarget.isValid());
		ctx.m_swapchainRenderTarget = outRt;

		pass.setWritesToSwapchain();
	}
	else
	{
		outRt = rgraph.newRenderTarget(m_rtDesc);
	}

	if(!bRendersToSwapchain)
	{
		m_runCtx.m_rt = outRt;
	}

	pass.setRenderpassInfo({GraphicsRenderPassTargetDesc(outRt)});

	pass.newTextureDependency(outRt, TextureUsageBit::kRtvDsvWrite);

	if(g_cvarRenderDbgScene || g_cvarRenderDbgPhysics)
	{
		pass.newTextureDependency(getRenderer().getDbg().getRt(), TextureUsageBit::kSrvPixel);
	}

	pass.newTextureDependency((g_cvarRenderMotionBlurSampleCount != 0) ? getRenderer().getMotionBlur().getRt()
																	   : getRenderer().getTonemapping().getRt(),
							  TextureUsageBit::kSrvPixel);
	pass.newTextureDependency(getRenderer().getBloom().getBloomRt(), TextureUsageBit::kSrvPixel);

	Array<RenderTargetHandle, kMaxDebugRenderTargets> dbgRts;
	Array<DebugRenderTargetDrawStyle, kMaxDebugRenderTargets> drawStyles;
	const Bool hasDebugRt = getRenderer().getCurrentDebugRenderTarget(dbgRts, drawStyles);
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

	getUiStage().setDependencies(pass);

	pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
		ANKI_TRACE_SCOPED_EVENT(FinalComposite);

		CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
		const Bool dbgEnabled = g_cvarRenderDbgScene || g_cvarRenderDbgPhysics;

		Array<RenderTargetHandle, kMaxDebugRenderTargets> dbgRts;
		Array<DebugRenderTargetDrawStyle, kMaxDebugRenderTargets> drawStyles;
		const Bool hasDebugRt = getRenderer().getCurrentDebugRenderTarget(dbgRts, drawStyles);

		// Bind program
		if(hasDebugRt)
		{
			cmdb.bindShaderProgram(m_visualizeRenderTargetGrProg.get());
		}
		else
		{
			cmdb.bindShaderProgram(m_grProgs[dbgEnabled].get());
		}

		// Bind stuff
		if(!hasDebugRt)
		{
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_trilinearClamp.get());

			rgraphCtx.bindSrv(
				0, 0, (g_cvarRenderMotionBlurSampleCount != 0) ? getRenderer().getMotionBlur().getRt() : getRenderer().getTonemapping().getRt());

			rgraphCtx.bindSrv(1, 0, getRenderer().getBloom().getBloomRt());

			if(dbgEnabled)
			{
				rgraphCtx.bindSrv(2, 0, getRenderer().getDbg().getRt());
			}

			struct Constants
			{
				UVec4 m_fsrConsts0;

				F32 m_filmGrainStrength;
				U32 m_frameCount;
				U32 m_padding1;
				U32 m_padding2;
			} consts;

			F32 sharpness = g_cvarRenderSharpness; // [0, 1]
			sharpness *= 3.0f; // [0, 3]
			sharpness = 3.0f - sharpness; // [3, 0], RCAS translates 0 to max sharpness
			FsrRcasCon(&consts.m_fsrConsts0[0], sharpness);

			consts.m_filmGrainStrength = g_cvarRenderFilmGrain;
			consts.m_frameCount = getRenderer().getFrameCount() & kMaxU32;

			cmdb.setFastConstants(&consts, sizeof(consts));
		}
		else
		{
			cmdb.bindSampler(0, 0, getRenderer().getSamplers().m_nearestNearestClamp.get());

			U32 count = 0;
			UVec4 consts;
			for(const RenderTargetHandle& handle : dbgRts)
			{
				if(handle.isValid())
				{
					consts[count] = U32(drawStyles[count]);
					rgraphCtx.bindSrv(count, 0, handle);
				}
				else
				{
					cmdb.bindSrv(count, 0, TextureView(getDummyGpuResources().m_texture2DSrv.get(), TextureSubresourceDesc::all()));
				}

				++count;
			}

			cmdb.setFastConstants(&consts, sizeof(consts));
		}

		cmdb.setViewport(0, 0, getRenderer().getPostProcessResolution().x(), getRenderer().getPostProcessResolution().y());
		drawQuad(cmdb);

		// Draw UI
		const Bool bRendersToSwapchain = getRenderer().getSwapchainResolution() == getRenderer().getPostProcessResolution();
		if(bRendersToSwapchain)
		{
			getRenderer().getUiStage().drawUi(cmdb);
		}
	});
}

} // end namespace anki

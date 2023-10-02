// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

static StatCounter g_rendererCpuTimeStatVar(StatCategory::kTime, "Renderer",
											StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates);
StatCounter g_rendererGpuTimeStatVar(StatCategory::kTime, "GPU frame", StatFlag::kMilisecond | StatFlag::kShowAverage | StatFlag::kMainThreadUpdates);

MainRenderer::MainRenderer()
{
}

MainRenderer::~MainRenderer()
{
	ANKI_R_LOGI("Destroying main renderer");

	deleteInstance(RendererMemoryPool::getSingleton(), m_r);

	RendererMemoryPool::freeSingleton();
}

Error MainRenderer::init(const MainRendererInitInfo& inf)
{
	RendererMemoryPool::allocateSingleton(inf.m_allocCallback, inf.m_allocCallbackUserData);

	m_framePool.init(inf.m_allocCallback, inf.m_allocCallbackUserData, 10_MB, 1.0f);

	// Init renderer and manipulate the width/height
	m_swapchainResolution = inf.m_swapchainSize;
	m_rDrawToDefaultFb = g_renderScalingCVar.get() == 1.0f;

	ANKI_R_LOGI("Initializing main renderer. Swapchain resolution %ux%u", m_swapchainResolution.x(), m_swapchainResolution.y());

	m_r = newInstance<Renderer>(RendererMemoryPool::getSingleton());
	ANKI_CHECK(m_r->init(m_swapchainResolution, &m_framePool));

	// Init other
	if(!m_rDrawToDefaultFb)
	{
		ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/BlitRaster.ankiprogbin", m_blitProg));
		const ShaderProgramResourceVariant* variant;
		m_blitProg->getOrCreateVariant(variant);
		m_blitGrProg.reset(&variant->getProgram());

		// The RT desc
		UVec2 resolution = UVec2(Vec2(m_swapchainResolution) * g_renderScalingCVar.get());
		alignRoundDown(2, resolution.x());
		alignRoundDown(2, resolution.y());
		m_tmpRtDesc = m_r->create2DRenderTargetDescription(
			resolution.x(), resolution.y(),
			(GrManager::getSingleton().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm : Format::kR8G8B8A8_Unorm,
			"Final Composite");
		m_tmpRtDesc.bake();

		// FB descr
		m_fbDescr.m_colorAttachmentCount = 1;
		m_fbDescr.bake();

		ANKI_R_LOGI("There will be a blit pass to the swapchain because render scaling is not 1.0");
	}

	m_rgraph = GrManager::getSingleton().newRenderGraph();

	return Error::kNone;
}

Error MainRenderer::render(Texture* presentTex)
{
	ANKI_TRACE_SCOPED_EVENT(Render);

	const Second startTime = HighRezTimer::getCurrentTime();

	// First thing, reset the temp mem pool
	m_framePool.reset();

	// Run renderer
	RenderingContext ctx(&m_framePool);
	m_runCtx.m_ctx = &ctx;
	m_runCtx.m_secondaryTaskId.setNonAtomically(0);
	ctx.m_renderGraphDescr.setStatisticsEnabled(ANKI_STATS_ENABLED);

	RenderTargetHandle presentRt = ctx.m_renderGraphDescr.importRenderTarget(presentTex, TextureUsageBit::kNone);

	if(m_rDrawToDefaultFb)
	{
		// m_r will draw to a presentable texture
		ctx.m_outRenderTarget = presentRt;
	}
	else
	{
		// m_r will draw to a temp tex
		ctx.m_outRenderTarget = ctx.m_renderGraphDescr.newRenderTarget(m_tmpRtDesc);
	}

	ANKI_CHECK(m_r->populateRenderGraph(ctx));

	// Blit renderer's result to default FB if needed
	if(!m_rDrawToDefaultFb)
	{
		GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Final Blit");

		pass.setFramebufferInfo(m_fbDescr, {presentRt});
		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;
			cmdb.setViewport(0, 0, m_swapchainResolution.x(), m_swapchainResolution.y());

			cmdb.bindShaderProgram(m_blitGrProg.get());
			cmdb.bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp.get());
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_ctx->m_outRenderTarget);

			cmdb.draw(PrimitiveTopology::kTriangles, 3);
		});

		pass.newTextureDependency(presentRt, TextureUsageBit::kFramebufferWrite);
		pass.newTextureDependency(ctx.m_outRenderTarget, TextureUsageBit::kSampledFragment);
	}

	// Create a dummy pass to transition the presentable image to present
	{
		ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Present");

		pass.setWork([]([[maybe_unused]] RenderPassWorkContext& rgraphCtx) {
			// Do nothing. This pass is dummy
		});
		pass.newTextureDependency(presentRt, TextureUsageBit::kPresent);
	}

	// Bake the render graph
	m_rgraph->compileNewGraph(ctx.m_renderGraphDescr, m_framePool);

	// Populate the 2nd level command buffers
	m_rgraph->runSecondLevel();

	// Populate 1st level command buffers
	m_rgraph->run();

	// Flush
	FencePtr fence;
	m_rgraph->flush(&fence);

	// Reset for the next frame
	m_rgraph->reset();
	m_r->finalize(ctx, fence.get());

	// Stats
	if(ANKI_STATS_ENABLED || ANKI_TRACING_ENABLED)
	{
		g_rendererCpuTimeStatVar.set((HighRezTimer::getCurrentTime() - startTime) * 1000.0);

		RenderGraphStatistics rgraphStats;
		m_rgraph->getStatistics(rgraphStats);
		g_rendererGpuTimeStatVar.set(rgraphStats.m_gpuTime * 1000.0);

		if(rgraphStats.m_gpuTime > 0.0)
		{
			// WARNING: The name of the event is somewhat special. Search it to see why
			ANKI_TRACE_CUSTOM_EVENT(GpuFrameTime, rgraphStats.m_cpuStartTime, rgraphStats.m_gpuTime);
		}
	}

	return Error::kNone;
}

Dbg& MainRenderer::getDbg()
{
	return m_r->getDbg();
}

F32 MainRenderer::getAspectRatio() const
{
	return m_r->getAspectRatio();
}

} // end namespace anki

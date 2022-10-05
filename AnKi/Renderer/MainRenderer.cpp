// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/MainRenderer.h>
#include <AnKi/Renderer/LightShading.h>
#include <AnKi/Renderer/FinalComposite.h>
#include <AnKi/Renderer/Dbg.h>
#include <AnKi/Renderer/GBuffer.h>
#include <AnKi/Renderer/RenderQueue.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/ThreadHive.h>

namespace anki {

MainRenderer::MainRenderer()
{
}

MainRenderer::~MainRenderer()
{
	ANKI_R_LOGI("Destroying main renderer");
}

Error MainRenderer::init(const MainRendererInitInfo& inf)
{
	m_pool.init(inf.m_allocCallback, inf.m_allocCallbackUserData, "MainRenderer");
	m_framePool.init(inf.m_allocCallback, inf.m_allocCallbackUserData, 10_MB, 1.0f);

	// Init renderer and manipulate the width/height
	m_swapchainResolution = inf.m_swapchainSize;
	m_rDrawToDefaultFb = inf.m_config->getRRenderScaling() == 1.0f;

	ANKI_R_LOGI("Initializing main renderer. Swapchain resolution %ux%u", m_swapchainResolution.x(),
				m_swapchainResolution.y());

	m_r.reset(newInstance<Renderer>(m_pool));
	ANKI_CHECK(m_r->init(inf.m_threadHive, inf.m_resourceManager, inf.m_gr, inf.m_stagingMemory, inf.m_ui, &m_pool,
						 inf.m_config, inf.m_globTimestamp, m_swapchainResolution));

	// Init other
	if(!m_rDrawToDefaultFb)
	{
		ANKI_CHECK(inf.m_resourceManager->loadResource("ShaderBinaries/BlitRaster.ankiprogbin", m_blitProg));
		const ShaderProgramResourceVariant* variant;
		m_blitProg->getOrCreateVariant(variant);
		m_blitGrProg = variant->getProgram();

		// The RT desc
		UVec2 resolution = UVec2(Vec2(m_swapchainResolution) * inf.m_config->getRRenderScaling());
		alignRoundDown(2, resolution.x());
		alignRoundDown(2, resolution.y());
		m_tmpRtDesc = m_r->create2DRenderTargetDescription(
			resolution.x(), resolution.y(),
			(m_r->getGrManager().getDeviceCapabilities().m_unalignedBbpTextureFormats) ? Format::kR8G8B8_Unorm
																					   : Format::kR8G8B8A8_Unorm,
			"Final Composite");
		m_tmpRtDesc.bake();

		// FB descr
		m_fbDescr.m_colorAttachmentCount = 1;
		m_fbDescr.bake();

		ANKI_R_LOGI("There will be a blit pass to the swapchain because render scaling is not 1.0");
	}

	m_rgraph = inf.m_gr->newRenderGraph();

	return Error::kNone;
}

Error MainRenderer::render(RenderQueue& rqueue, TexturePtr presentTex)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER);

	m_stats.m_renderingCpuTime = (m_statsEnabled) ? HighRezTimer::getCurrentTime() : -1.0;

	// First thing, reset the temp mem pool
	m_framePool.reset();

	// Run renderer
	RenderingContext ctx(&m_framePool);
	m_runCtx.m_ctx = &ctx;
	m_runCtx.m_secondaryTaskId.setNonAtomically(0);
	ctx.m_renderGraphDescr.setStatisticsEnabled(m_statsEnabled);

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

	ctx.m_renderQueue = &rqueue;
	ANKI_CHECK(m_r->populateRenderGraph(ctx));

	// Blit renderer's result to default FB if needed
	if(!m_rDrawToDefaultFb)
	{
		GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Final Blit");

		pass.setFramebufferInfo(m_fbDescr, {presentRt});
		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->setViewport(0, 0, m_swapchainResolution.x(), m_swapchainResolution.y());

			cmdb->bindShaderProgram(m_blitGrProg);
			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_ctx->m_outRenderTarget);

			cmdb->drawArrays(PrimitiveTopology::kTriangles, 3);
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
	Array<ThreadHiveTask, ThreadHive::kMaxThreads> tasks;
	for(U i = 0; i < m_r->getThreadHive().getThreadCount(); ++i)
	{
		tasks[i].m_argument = this;
		tasks[i].m_callback = [](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
								 [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
			MainRenderer& self = *static_cast<MainRenderer*>(userData);

			const U32 taskId = self.m_runCtx.m_secondaryTaskId.fetchAdd(1);
			self.m_rgraph->runSecondLevel(taskId);
		};
	}
	m_r->getThreadHive().submitTasks(&tasks[0], m_r->getThreadHive().getThreadCount());
	m_r->getThreadHive().waitAllTasks();

	// Populate 1st level command buffers
	m_rgraph->run();

	// Flush
	m_rgraph->flush();

	// Reset for the next frame
	m_rgraph->reset();
	m_r->finalize(ctx);

	// Stats
	if(m_statsEnabled)
	{
		m_stats.m_renderingCpuTime = HighRezTimer::getCurrentTime() - m_stats.m_renderingCpuTime;

		RenderGraphStatistics rgraphStats;
		m_rgraph->getStatistics(rgraphStats);
		m_stats.m_renderingGpuTime = rgraphStats.m_gpuTime;
		m_stats.m_renderingGpuSubmitTimestamp = rgraphStats.m_cpuStartTime;
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

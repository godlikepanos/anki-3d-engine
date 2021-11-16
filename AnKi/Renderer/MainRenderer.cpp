// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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

Error MainRenderer::init(ThreadHive* hive, ResourceManager* resources, GrManager* gr, StagingGpuMemoryPool* stagingMem,
						 UiManager* ui, AllocAlignedCallback allocCb, void* allocCbUserData, const ConfigSet& config,
						 Timestamp* globTimestamp)
{
	ANKI_R_LOGI("Initializing main renderer");

	m_alloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	m_frameAlloc = StackAllocator<U8>(allocCb, allocCbUserData, 1024 * 1024 * 10, 1.0f);

	// Init renderer and manipulate the width/height
	m_swapchainResolution.x() = config.getNumberU32("width");
	m_swapchainResolution.y() = config.getNumberU32("height");
	m_renderScaling = config.getNumberF32("r_renderScaling");

	m_rDrawToDefaultFb = m_renderScaling == 1.0f;

	m_r.reset(m_alloc.newInstance<Renderer>());
	ANKI_CHECK(m_r->init(hive, resources, gr, stagingMem, ui, m_alloc, config, globTimestamp));

	// Init other
	if(!m_rDrawToDefaultFb)
	{
		ANKI_CHECK(resources->loadResource("Shaders/BlitGraphics.ankiprog", m_blitProg));
		const ShaderProgramResourceVariant* variant;
		m_blitProg->getOrCreateVariant(variant);
		m_blitGrProg = variant->getProgram();

		// The RT desc
		const Vec2 fresolution = Vec2(F32(config.getNumberU32("width")), F32(config.getNumberU32("height")));
		UVec2 resolution = UVec2(fresolution * m_renderScaling);
		alignRoundDown(2, resolution.x());
		alignRoundDown(2, resolution.y());
		m_tmpRtDesc = m_r->create2DRenderTargetDescription(resolution.x(), resolution.y(), Format::R8G8B8_UNORM,
														   "Final Composite");
		m_tmpRtDesc.bake();

		// FB descr
		m_fbDescr.m_colorAttachmentCount = 1;
		m_fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		m_fbDescr.bake();

		ANKI_R_LOGI("The main renderer will have to blit the offscreen renderer's result");
	}

	m_rgraph = gr->newRenderGraph();

	ANKI_R_LOGI("Main renderer initialized. Rendering size %ux%u", m_swapchainResolution.x(),
				m_swapchainResolution.x());

	return Error::NONE;
}

Error MainRenderer::render(RenderQueue& rqueue, TexturePtr presentTex)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER);

	m_stats.m_renderingCpuTime = (m_statsEnabled) ? HighRezTimer::getCurrentTime() : -1.0;

	// First thing, reset the temp mem pool
	m_frameAlloc.getMemoryPool().reset();

	// Run renderer
	RenderingContext ctx(m_frameAlloc);
	m_runCtx.m_ctx = &ctx;
	m_runCtx.m_secondaryTaskId.setNonAtomically(0);
	ctx.m_renderGraphDescr.setStatisticsEnabled(m_statsEnabled);

	RenderTargetHandle presentRt = ctx.m_renderGraphDescr.importRenderTarget(presentTex, TextureUsageBit::NONE);

	if(m_rDrawToDefaultFb)
	{
		// m_r will draw to a presentable texture

		ctx.m_outRenderTarget = presentRt;
		ctx.m_outRenderTargetWidth = presentTex->getWidth();
		ctx.m_outRenderTargetHeight = presentTex->getHeight();
	}
	else
	{
		// m_r will draw to a temp tex

		ctx.m_outRenderTarget = ctx.m_renderGraphDescr.newRenderTarget(m_tmpRtDesc);
		ctx.m_outRenderTargetWidth = U32(F32(m_swapchainResolution.x()) * m_renderScaling);
		ctx.m_outRenderTargetHeight = U32(F32(m_swapchainResolution.y()) * m_renderScaling);
	}

	ctx.m_renderQueue = &rqueue;
	ANKI_CHECK(m_r->populateRenderGraph(ctx));

	// Blit renderer's result to default FB if needed
	if(!m_rDrawToDefaultFb)
	{
		GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Final Blit");

		pass.setFramebufferInfo(m_fbDescr, {{presentRt}}, {});
		pass.setWork([this](RenderPassWorkContext& rgraphCtx) {
			CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
			cmdb->setViewport(0, 0, m_swapchainResolution.x(), m_swapchainResolution.y());

			cmdb->bindShaderProgram(m_blitGrProg);
			cmdb->bindSampler(0, 0, m_r->getSamplers().m_trilinearClamp);
			rgraphCtx.bindColorTexture(0, 1, m_runCtx.m_ctx->m_outRenderTarget);

			cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3, 1);
		});

		pass.newDependency({presentRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newDependency({ctx.m_outRenderTarget, TextureUsageBit::SAMPLED_FRAGMENT});
	}

	// Create a dummy pass to transition the presentable image to present
	{
		ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Present");

		pass.setWork([](RenderPassWorkContext& rgraphCtx) {
			// Do nothing. This pass is dummy
		});
		pass.newDependency({presentRt, TextureUsageBit::PRESENT});
	}

	// Bake the render graph
	m_rgraph->compileNewGraph(ctx.m_renderGraphDescr, m_frameAlloc);

	// Populate the 2nd level command buffers
	Array<ThreadHiveTask, ThreadHive::MAX_THREADS> tasks;
	for(U i = 0; i < m_r->getThreadHive().getThreadCount(); ++i)
	{
		tasks[i].m_argument = this;
		tasks[i].m_callback = [](void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore) {
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

	return Error::NONE;
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

// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/MainRenderer.h>
#include <anki/renderer/LightShading.h>
#include <anki/renderer/FinalComposite.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/GBuffer.h>
#include <anki/renderer/Indirect.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/util/Logger.h>
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/core/Trace.h>
#include <anki/core/App.h>
#include <anki/misc/ConfigSet.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/ThreadHive.h>

namespace anki
{

MainRenderer::MainRenderer()
{
}

MainRenderer::~MainRenderer()
{
	ANKI_R_LOGI("Destroying main renderer");
}

Error MainRenderer::init(ThreadHive* hive,
	ResourceManager* resources,
	GrManager* gr,
	StagingGpuMemoryManager* stagingMem,
	UiManager* ui,
	AllocAlignedCallback allocCb,
	void* allocCbUserData,
	const ConfigSet& config,
	Timestamp* globTimestamp)
{
	ANKI_R_LOGI("Initializing main renderer");

	m_alloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	m_frameAlloc = StackAllocator<U8>(allocCb, allocCbUserData, 1024 * 1024 * 10, 1.0);

	// Init renderer and manipulate the width/height
	m_width = config.getNumber("width");
	m_height = config.getNumber("height");
	ConfigSet config2 = config;
	m_renderingQuality = config.getNumber("r.renderingQuality");
	UVec2 size(m_renderingQuality * F32(m_width), m_renderingQuality * F32(m_height));

	config2.set("width", size.x());
	config2.set("height", size.y());

	m_rDrawToDefaultFb = m_renderingQuality == 1.0;

	m_r.reset(m_alloc.newInstance<Renderer>());
	ANKI_CHECK(m_r->init(hive, resources, gr, stagingMem, ui, m_alloc, config2, globTimestamp));

	// Init other
	if(!m_rDrawToDefaultFb)
	{
		ANKI_CHECK(resources->loadResource("shaders/Blit.glslp", m_blitProg));
		const ShaderProgramResourceVariant* variant;
		m_blitProg->getOrCreateVariant(variant);
		m_blitGrProg = variant->getProgram();

		// The RT desc
		m_tmpRtDesc = m_r->create2DRenderTargetDescription(m_width, m_height, Format::R8G8B8_UNORM, "Final Composite");
		m_tmpRtDesc.bake();

		ANKI_R_LOGI("The main renderer will have to blit the offscreen renderer's result");
	}

	m_rgraph = gr->newRenderGraph();

	ANKI_R_LOGI("Main renderer initialized. Rendering size %ux%u", m_width, m_height);

	return Error::NONE;
}

Error MainRenderer::render(RenderQueue& rqueue, TexturePtr presentTex)
{
	ANKI_TRACE_SCOPED_EVENT(RENDER);

	m_stats.m_renderingTime = HighRezTimer::getCurrentTime();

	// First thing, reset the temp mem pool
	m_frameAlloc.getMemoryPool().reset();

	// Run renderer
	RenderingContext ctx(m_frameAlloc);
	m_runCtx.m_ctx = &ctx;
	m_runCtx.m_secondaryTaskId.set(0);

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
		ctx.m_outRenderTargetWidth = m_width;
		ctx.m_outRenderTargetHeight = m_height;
	}

	ctx.m_renderQueue = &rqueue;
	ANKI_CHECK(m_r->populateRenderGraph(ctx));

	// Blit renderer's result to default FB if needed
	if(!m_rDrawToDefaultFb)
	{
		GraphicsRenderPassDescription& pass = ctx.m_renderGraphDescr.newGraphicsRenderPass("Final Blit");

		FramebufferDescription fbDescr;
		fbDescr.m_colorAttachmentCount = 1;
		fbDescr.m_colorAttachments[0].m_loadOperation = AttachmentLoadOperation::DONT_CARE;
		fbDescr.bake();

		pass.setFramebufferInfo(fbDescr, {{presentRt}}, {});
		pass.setWork(runCallback, this, 0);

		pass.newDependency({presentRt, TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE});
		pass.newDependency({ctx.m_outRenderTarget, TextureUsageBit::SAMPLED_FRAGMENT});
	}

	// Create a dummy pass to transition the presentable image to present
	{
		ComputeRenderPassDescription& pass = ctx.m_renderGraphDescr.newComputeRenderPass("Present");

		pass.setWork(presentCallback, nullptr, 0);
		pass.newDependency({presentRt, TextureUsageBit::PRESENT});
	}

	// Bake the render graph
	m_rgraph->compileNewGraph(ctx.m_renderGraphDescr, m_frameAlloc);

	// Populate the 2nd level command buffers
	Array<ThreadHiveTask, ThreadHive::MAX_THREADS> tasks;
	for(U i = 0; i < m_r->getThreadHive().getThreadCount(); ++i)
	{
		tasks[i].m_argument = this;
		tasks[i].m_callback = executeSecondaryCallback;
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
	static_cast<RendererStats&>(m_stats) = m_r->getStats();
	m_stats.m_renderingTime = HighRezTimer::getCurrentTime() - m_stats.m_renderingTime;

	return Error::NONE;
}

void MainRenderer::executeSecondaryCallback(
	void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore)
{
	MainRenderer& self = *static_cast<MainRenderer*>(userData);

	const U taskId = self.m_runCtx.m_secondaryTaskId.fetchAdd(1);
	self.m_rgraph->runSecondLevel(taskId);
}

void MainRenderer::runBlit(RenderPassWorkContext& rgraphCtx)
{
	CommandBufferPtr& cmdb = rgraphCtx.m_commandBuffer;
	cmdb->setViewport(0, 0, m_width, m_height);

	cmdb->bindShaderProgram(m_blitGrProg);
	rgraphCtx.bindColorTextureAndSampler(0, 0, m_runCtx.m_ctx->m_outRenderTarget, m_r->getLinearSampler());

	cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3, 1);
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

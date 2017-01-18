// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/renderer/Renderer.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/core/Trace.h>
#include <anki/misc/ConfigSet.h>

#include <anki/renderer/Ir.h>
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Sm.h>
#include <anki/renderer/Pps.h>
#include <anki/renderer/Ssao.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Tm.h>
#include <anki/renderer/Fs.h>
#include <anki/renderer/Lf.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/FsUpscale.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Volumetric.h>
#include <anki/renderer/DepthDownscale.h>
#include <anki/renderer/Smaa.h>

#include <cstdarg> // For var args

namespace anki
{

static Bool threadWillDoWork(
	const RenderingContext& ctx, VisibilityGroupType typeOfWork, U32 threadId, PtrSize threadCount)
{
	const VisibilityTestResults& vis = ctx.m_frustumComponent->getVisibilityTestResults();

	U problemSize = vis.getCount(typeOfWork);
	PtrSize start, end;
	ThreadPoolTask::choseStartEnd(threadId, threadCount, problemSize, start, end);

	return start != end;
}

Renderer::Renderer()
	: m_sceneDrawer(this)
{
}

Renderer::~Renderer()
{
}

Error Renderer::init(ThreadPool* threadpool,
	ResourceManager* resources,
	GrManager* gl,
	StagingGpuMemoryManager* stagingMem,
	HeapAllocator<U8> alloc,
	StackAllocator<U8> frameAlloc,
	const ConfigSet& config,
	Timestamp* globTimestamp,
	Bool willDrawToDefaultFbo)
{
	m_globTimestamp = globTimestamp;
	m_threadpool = threadpool;
	m_resources = resources;
	m_gr = gl;
	m_stagingMem = stagingMem;
	m_alloc = alloc;
	m_frameAlloc = frameAlloc;
	m_willDrawToDefaultFbo = willDrawToDefaultFbo;

	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to initialize the renderer");
	}

	return err;
}

Error Renderer::initInternal(const ConfigSet& config)
{
	// Set from the config
	m_width = config.getNumber("width");
	m_height = config.getNumber("height");
	ANKI_LOGI("Initializing offscreen renderer. Size %ux%u", m_width, m_height);

	m_lodDistance = config.getNumber("lodDistance");
	m_frameCount = 0;

	m_tessellation = config.getNumber("tessellation");

	// A few sanity checks
	if(m_width < 10 || m_height < 10)
	{
		ANKI_LOGE("Incorrect sizes");
		return ErrorCode::USER_DATA;
	}

	// quad setup
	ANKI_CHECK(m_resources->loadResource("shaders/Quad.vert.glsl", m_drawQuadVert));

	// Init the stages. Careful with the order!!!!!!!!!!
	m_ir.reset(m_alloc.newInstance<Ir>(this));
	ANKI_CHECK(m_ir->init(config));

	m_ms.reset(m_alloc.newInstance<Ms>(this));
	ANKI_CHECK(m_ms->init(config));

	m_sm.reset(m_alloc.newInstance<Sm>(this));
	ANKI_CHECK(m_sm->init(config));

	m_is.reset(m_alloc.newInstance<Is>(this));
	ANKI_CHECK(m_is->init(config));

	m_depth.reset(m_alloc.newInstance<DepthDownscale>(this));
	ANKI_CHECK(m_depth->init(config));

	m_vol.reset(m_alloc.newInstance<Volumetric>(this));
	ANKI_CHECK(m_vol->init(config));

	m_fs.reset(m_alloc.newInstance<Fs>(this));
	ANKI_CHECK(m_fs->init(config));

	m_lf.reset(m_alloc.newInstance<Lf>(this));
	ANKI_CHECK(m_lf->init(config));

	m_ssao.reset(m_alloc.newInstance<Ssao>(this));
	ANKI_CHECK(m_ssao->init(config));

	m_fsUpscale.reset(m_alloc.newInstance<FsUpscale>(this));
	ANKI_CHECK(m_fsUpscale->init(config));

	m_tm.reset(getAllocator().newInstance<Tm>(this));
	ANKI_CHECK(m_tm->init(config));

	m_downscale.reset(getAllocator().newInstance<DownscaleBlur>(this));
	ANKI_CHECK(m_downscale->init(config));

	m_smaa.reset(getAllocator().newInstance<Smaa>(this));
	ANKI_CHECK(m_smaa->init(config));

	m_bloom.reset(m_alloc.newInstance<Bloom>(this));
	ANKI_CHECK(m_bloom->init(config));

	m_pps.reset(m_alloc.newInstance<Pps>(this));
	ANKI_CHECK(m_pps->init(config));

	m_dbg.reset(m_alloc.newInstance<Dbg>(this));
	ANKI_CHECK(m_dbg->init(config));

	return ErrorCode::NONE;
}

Error Renderer::render(RenderingContext& ctx)
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	ANKI_ASSERT(ctx.m_frustumComponent->getFrustum().getType() == FrustumType::PERSPECTIVE);

	ctx.m_prevViewProjMat = m_prevViewProjMat;

	// Check if resources got loaded
	if(m_prevLoadRequestCount != m_resources->getLoadingRequestCount()
		|| m_prevAsyncTasksCompleted != m_resources->getAsyncTaskCompletedCount())
	{
		m_prevLoadRequestCount = m_resources->getLoadingRequestCount();
		m_prevAsyncTasksCompleted = m_resources->getAsyncTaskCompletedCount();
		m_resourcesDirty = true;
	}
	else
	{
		m_resourcesDirty = false;
	}

	// Run stages
	ANKI_CHECK(m_ir->run(ctx));

	ANKI_CHECK(m_is->binLights(ctx));

	m_lf->resetOcclusionQueries(ctx, cmdb);
	ANKI_CHECK(buildCommandBuffers(ctx));

	// Perform image transitions
	m_sm->setPreRunBarriers(ctx);
	m_ms->setPreRunBarriers(ctx);
	m_is->setPreRunBarriers(ctx);
	m_fs->setPreRunBarriers(ctx);
	m_ssao->setPreRunBarriers(ctx);
	m_bloom->m_extractExposure.setPreRunBarriers(ctx);
	m_bloom->m_upscale.setPreRunBarriers(ctx);
	m_depth->m_hd.setPreRunBarriers(ctx);
	m_depth->m_qd.setPreRunBarriers(ctx);
	m_smaa->m_edge.setPreRunBarriers(ctx);
	m_smaa->m_weights.setPreRunBarriers(ctx);
	m_vol->setPreRunBarriers(ctx);

	// SM
	m_sm->run(ctx);

	m_ms->run(ctx);

	m_ms->setPostRunBarriers(ctx);
	m_sm->setPostRunBarriers(ctx);

	// Batch IS + HD
	m_is->run(ctx);
	m_depth->m_hd.run(ctx);

	m_depth->m_hd.setPostRunBarriers(ctx);

	m_depth->m_qd.run(ctx);

	m_depth->m_qd.setPostRunBarriers(ctx);

	m_vol->run(ctx);
	m_vol->setPostRunBarriers(ctx);

	m_lf->updateIndirectInfo(ctx, cmdb);

	// Batch FS & SSAO & VOL
	m_fs->run(ctx);
	m_ssao->run(ctx);

	m_ssao->setPostRunBarriers(ctx);
	m_fs->setPostRunBarriers(ctx);

	m_fsUpscale->run(ctx);

	cmdb->setTextureSurfaceBarrier(m_is->getRt(),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(0, 0, 0, 0));

	m_downscale->run(ctx);

	cmdb->setTextureSurfaceBarrier(m_is->getRt(),
		TextureUsageBit::FRAMEBUFFER_ATTACHMENT_WRITE,
		TextureUsageBit::SAMPLED_COMPUTE,
		TextureSurfaceInfo(m_is->getRtMipmapCount() - 1, 0, 0, 0));

	// Batch TM + SMAA_pass0
	m_tm->run(ctx);
	m_smaa->m_edge.run(ctx);

	// Barriers
	cmdb->setTextureSurfaceBarrier(m_is->getRt(),
		TextureUsageBit::SAMPLED_COMPUTE,
		TextureUsageBit::SAMPLED_FRAGMENT,
		TextureSurfaceInfo(m_is->getRtMipmapCount() - 1, 0, 0, 0));
	m_smaa->m_edge.setPostRunBarriers(ctx);

	// Batch bloom + SSLF + SMAA_pass1
	m_bloom->m_extractExposure.run(ctx);
	m_smaa->m_weights.run(ctx);

	// Barriers
	m_bloom->m_extractExposure.setPostRunBarriers(ctx);
	m_smaa->m_weights.setPostRunBarriers(ctx);

	// Bloom last pass
	m_bloom->m_upscale.run(ctx);
	m_bloom->m_upscale.setPostRunBarriers(ctx);

	if(m_dbg->getEnabled())
	{
		ANKI_CHECK(m_dbg->run(ctx));
	}

	ANKI_CHECK(m_pps->run(ctx));

	++m_frameCount;
	m_prevViewProjMat = ctx.m_frustumComponent->getViewProjectionMatrix();

	return ErrorCode::NONE;
}

Vec3 Renderer::unproject(
	const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat, const int view[4])
{
	Mat4 invPm = projectionMat * modelViewMat;
	invPm.invert();

	// the vec is in NDC space meaning: -1<=vec.x<=1 -1<=vec.y<=1 -1<=vec.z<=1
	Vec4 vec;
	vec.x() = (2.0 * (windowCoords.x() - view[0])) / view[2] - 1.0;
	vec.y() = (2.0 * (windowCoords.y() - view[1])) / view[3] - 1.0;
	vec.z() = 2.0 * windowCoords.z() - 1.0;
	vec.w() = 1.0;

	Vec4 out = invPm * vec;
	out /= out.w();
	return out.xyz();
}

void Renderer::createRenderTarget(
	U32 w, U32 h, const PixelFormat& format, TextureUsageBit usage, SamplingFilter filter, U mipsCount, TexturePtr& rt)
{
	// Not very important but keep the resolution of render targets aligned to 16
	if(0)
	{
		ANKI_ASSERT(isAligned(16, w));
		ANKI_ASSERT(isAligned(16, h));
	}

	TextureInitInfo init;

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 1;
	init.m_layerCount = 1;
	init.m_type = TextureType::_2D;
	init.m_format = format;
	init.m_mipmapsCount = mipsCount;
	init.m_samples = 1;
	init.m_usage = usage;
	init.m_sampling.m_minMagFilter = filter;
	if(mipsCount > 1)
	{
		init.m_sampling.m_mipmapFilter = filter;
	}
	else
	{
		init.m_sampling.m_mipmapFilter = SamplingFilter::BASE;
	}
	init.m_sampling.m_repeat = false;
	init.m_sampling.m_anisotropyLevel = 0;

	rt = m_gr->newInstance<Texture>(init);
}

void Renderer::clearRenderTarget(TexturePtr rt, const ClearValue& clear, TextureUsageBit transferTo)
{
	TextureSurfaceInfo surf = {0, 0, 0, 0};
	CommandBufferInitInfo cinit;
	cinit.m_flags = CommandBufferFlag::SMALL_BATCH | CommandBufferFlag::TRANSFER_WORK;
	CommandBufferPtr cmdb = m_gr->newInstance<CommandBuffer>(cinit);
	cmdb->setTextureSurfaceBarrier(rt, TextureUsageBit::NONE, TextureUsageBit::CLEAR, surf);
	cmdb->clearTextureSurface(rt, surf, clear);
	cmdb->setTextureSurfaceBarrier(rt, TextureUsageBit::CLEAR, transferTo, surf);
	cmdb->flush();
}

Error Renderer::buildCommandBuffersInternal(RenderingContext& ctx, U32 threadId, PtrSize threadCount)
{
	// MS
	//
	ANKI_CHECK(m_ms->buildCommandBuffers(ctx, threadId, threadCount));

	// Append to the last MS's cmdb the occlusion tests
	if(ctx.m_ms.m_lastThreadWithWork == threadId)
	{
		m_lf->runOcclusionTests(ctx, ctx.m_ms.m_commandBuffers[threadId]);
	}

	if(ctx.m_ms.m_commandBuffers[threadId])
	{
		ctx.m_ms.m_commandBuffers[threadId]->flush();
	}

	// SM
	//
	ANKI_CHECK(m_sm->buildCommandBuffers(ctx, threadId, threadCount));

	// FS
	//
	ANKI_CHECK(m_fs->buildCommandBuffers(ctx, threadId, threadCount));

	// Append to the last FB's cmdb the other passes
	if(ctx.m_fs.m_lastThreadWithWork == threadId)
	{
		m_lf->run(ctx, ctx.m_fs.m_commandBuffers[threadId]);
		m_fs->drawVolumetric(ctx, ctx.m_fs.m_commandBuffers[threadId]);
	}
	else if(threadId == threadCount - 1 && ctx.m_fs.m_lastThreadWithWork == MAX_U32)
	{
		// There is no FS work. Create a cmdb just for LF & VOL

		CommandBufferInitInfo init;
		init.m_flags =
			CommandBufferFlag::GRAPHICS_WORK | CommandBufferFlag::SECOND_LEVEL | CommandBufferFlag::SMALL_BATCH;
		init.m_framebuffer = m_fs->getFramebuffer();
		CommandBufferPtr cmdb = getGrManager().newInstance<CommandBuffer>(init);
		cmdb->setViewport(0, 0, m_fs->getWidth(), m_fs->getHeight());

		m_lf->run(ctx, cmdb);
		m_fs->drawVolumetric(ctx, cmdb);

		ctx.m_fs.m_commandBuffers[threadId] = cmdb;
	}

	if(ctx.m_fs.m_commandBuffers[threadId])
	{
		ctx.m_fs.m_commandBuffers[threadId]->flush();
	}

	return ErrorCode::NONE;
}

Error Renderer::buildCommandBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDERER_COMMAND_BUFFER_BUILDING);
	ThreadPool& threadPool = getThreadPool();

	// Prepare
	if(m_sm)
	{
		m_sm->prepareBuildCommandBuffers(ctx);
	}

	// Find the last jobs for MS and FS
	U32 lastMsJob = MAX_U32;
	U32 lastFsJob = MAX_U32;
	U threadCount = threadPool.getThreadsCount();
	for(U i = threadCount - 1; i != 0; --i)
	{
		if(threadWillDoWork(ctx, VisibilityGroupType::RENDERABLES_MS, i, threadCount) && lastMsJob == MAX_U32)
		{
			lastMsJob = i;
		}

		if(threadWillDoWork(ctx, VisibilityGroupType::RENDERABLES_FS, i, threadCount) && lastFsJob == MAX_U32)
		{
			lastFsJob = i;
		}
	}

	ctx.m_ms.m_lastThreadWithWork = lastMsJob;
	ctx.m_fs.m_lastThreadWithWork = lastFsJob;

	// Build
	class Task : public ThreadPoolTask
	{
	public:
		Renderer* m_r ANKI_DBG_NULLIFY_PTR;
		RenderingContext* m_ctx ANKI_DBG_NULLIFY_PTR;

		Error operator()(U32 threadId, PtrSize threadCount)
		{
			return m_r->buildCommandBuffersInternal(*m_ctx, threadId, threadCount);
		}
	};

	Task task;
	task.m_r = this;
	task.m_ctx = &ctx;
	for(U i = 0; i < threadPool.getThreadsCount(); i++)
	{
		threadPool.assignNewTask(i, &task);
	}

	Error err = threadPool.waitForAllThreadsToFinish();
	ANKI_TRACE_STOP_EVENT(RENDERER_COMMAND_BUFFER_BUILDING);

	return err;
}

Error Renderer::createShader(CString fname, ShaderResourcePtr& shader, CString extra)
{
	return m_resources->loadResourceToCache(shader, fname, &extra[0], "r_");
}

Error Renderer::createShaderf(CString fname, ShaderResourcePtr& shader, CString fmt, ...)
{
	Array<char, 512> buffer;
	va_list args;

	va_start(args, fmt);
	I len = std::vsnprintf(&buffer[0], sizeof(buffer), &fmt[0], args);
	va_end(args);
	ANKI_ASSERT(len > 0 && len < I(sizeof(buffer) - 1));
	(void)len;

	return m_resources->loadResourceToCache(shader, fname, &buffer[0], "r_");
}

void Renderer::createDrawQuadShaderProgram(ShaderPtr frag, ShaderProgramPtr& prog)
{
	prog = m_gr->newInstance<ShaderProgram>(m_drawQuadVert->getGrShader(), frag);
}

} // end namespace anki

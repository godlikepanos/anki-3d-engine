// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
#include <anki/renderer/Sslf.h>
#include <anki/renderer/Bloom.h>
#include <anki/renderer/Tm.h>
#include <anki/renderer/Fs.h>
#include <anki/renderer/Lf.h>
#include <anki/renderer/Dbg.h>
#include <anki/renderer/Tiler.h>
#include <anki/renderer/Upsample.h>
#include <anki/renderer/DownscaleBlur.h>
#include <anki/renderer/Volumetric.h>

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

/// See shader for documentation
class RendererCommonUniforms
{
public:
	Vec4 m_projectionParams;
	Vec4 m_nearFarLinearizeDepth;
	Mat4 m_projectionMatrix;
};

//==============================================================================
// Renderer                                                                    =
//==============================================================================

//==============================================================================
Renderer::Renderer()
	: m_sceneDrawer(this)
{
}

//==============================================================================
Renderer::~Renderer()
{
}

//==============================================================================
Error Renderer::init(ThreadPool* threadpool,
	ResourceManager* resources,
	GrManager* gl,
	HeapAllocator<U8> alloc,
	StackAllocator<U8> frameAlloc,
	const ConfigSet& config,
	Timestamp* globTimestamp)
{
	m_globTimestamp = globTimestamp;
	m_threadpool = threadpool;
	m_resources = resources;
	m_gr = gl;
	m_alloc = alloc;
	m_frameAlloc = frameAlloc;

	Error err = initInternal(config);
	if(err)
	{
		ANKI_LOGE("Failed to initialize the renderer");
	}

	return err;
}

//==============================================================================
Error Renderer::initInternal(const ConfigSet& config)
{
	// Set from the config
	m_width = config.getNumber("width");
	m_height = config.getNumber("height");
	ANKI_ASSERT(
		isAligned(TILE_SIZE, m_width) && isAligned(TILE_SIZE, m_height));
	ANKI_LOGI("Initializing offscreen renderer. Size %ux%u", m_width, m_height);

	m_lodDistance = config.getNumber("lodDistance");
	m_frameCount = 0;
	m_samples = config.getNumber("samples");
	m_tileCountXY.x() = m_width / TILE_SIZE;
	m_tileCountXY.y() = m_height / TILE_SIZE;
	m_tileCount = m_tileCountXY.x() * m_tileCountXY.y();

	m_clusterer.init(getAllocator(),
		m_tileCountXY.x(),
		m_tileCountXY.y(),
		config.getNumber("clusterSizeZ"));

	m_tessellation = config.getNumber("tessellation");

	// A few sanity checks
	if(m_samples != 1 && m_samples != 4 && m_samples != 8 && m_samples != 16
		&& m_samples != 32)
	{
		ANKI_LOGE("Incorrect samples");
		return ErrorCode::USER_DATA;
	}

	if(m_width < 10 || m_height < 10)
	{
		ANKI_LOGE("Incorrect sizes");
		return ErrorCode::USER_DATA;
	}

	if(m_width % m_tileCountXY.x() != 0)
	{
		ANKI_LOGE("Width is not multiple of tile width");
		return ErrorCode::USER_DATA;
	}

	if(m_height % m_tileCountXY.y() != 0)
	{
		ANKI_LOGE("Height is not multiple of tile height");
		return ErrorCode::USER_DATA;
	}

	// quad setup
	ANKI_CHECK(
		m_resources->loadResource("shaders/Quad.vert.glsl", m_drawQuadVert));

	// Init the stages. Careful with the order!!!!!!!!!!
	if(config.getNumber("ir.enabled"))
	{
		m_ir.reset(m_alloc.newInstance<Ir>(this));
		ANKI_CHECK(m_ir->init(config));
	}

	m_ms.reset(m_alloc.newInstance<Ms>(this));
	ANKI_CHECK(m_ms->init(config));

	if(config.getNumber("sm.enabled"))
	{
		m_sm.reset(m_alloc.newInstance<Sm>(this));
		ANKI_CHECK(m_sm->init(config));
	}

	m_tiler.reset(m_alloc.newInstance<Tiler>(this));
	ANKI_CHECK(m_tiler->init());

	m_is.reset(m_alloc.newInstance<Is>(this));
	ANKI_CHECK(m_is->init(config));

	m_fs.reset(m_alloc.newInstance<Fs>(this));
	ANKI_CHECK(m_fs->init(config));

	m_vol.reset(m_alloc.newInstance<Volumetric>(this));
	ANKI_CHECK(m_vol->init(config));

	m_lf.reset(m_alloc.newInstance<Lf>(this));
	ANKI_CHECK(m_lf->init(config));

	if(config.getNumber("ssao.enabled") && config.getNumber("pps.enabled"))
	{
		m_ssao.reset(m_alloc.newInstance<Ssao>(this));
		ANKI_CHECK(m_ssao->init(config));
	}

	m_upsample.reset(m_alloc.newInstance<Upsample>(this));
	ANKI_CHECK(m_upsample->init(config));

	if(config.getNumber("tm.enabled") && config.getNumber("pps.enabled"))
	{
		m_tm.reset(getAllocator().newInstance<Tm>(this));
		ANKI_CHECK(m_tm->create(config));
	}

	if(config.getNumber("tm.enabled") && config.getNumber("pps.enabled"))
	{
		m_downscale.reset(getAllocator().newInstance<DownscaleBlur>(this));
		ANKI_CHECK(m_downscale->init(config));
	}

	if(config.getNumber("bloom.enabled") && config.getNumber("pps.enabled"))
	{
		m_bloom.reset(m_alloc.newInstance<Bloom>(this));
		ANKI_CHECK(m_bloom->init(config));
	}

	if(config.getNumber("sslf.enabled") && m_bloom
		&& config.getNumber("pps.enabled"))
	{
		m_sslf.reset(m_alloc.newInstance<Sslf>(this));
		ANKI_CHECK(m_sslf->init(config));
	}

	if(config.getNumber("pps.enabled"))
	{
		m_pps.reset(m_alloc.newInstance<Pps>(this));
		ANKI_CHECK(m_pps->init(config));
	}

	m_dbg.reset(m_alloc.newInstance<Dbg>(this));
	ANKI_CHECK(m_dbg->init(config));

	return ErrorCode::NONE;
}

//==============================================================================
Error Renderer::render(RenderingContext& ctx)
{
	FrustumComponent& frc = *ctx.m_frustumComponent;
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	// Misc
	ANKI_ASSERT(frc.getFrustum().getType() == Frustum::Type::PERSPECTIVE);
	m_clusterer.prepare(getThreadPool(), frc);

	// Write the common uniforms
	RendererCommonUniforms* commonUniforms =
		static_cast<RendererCommonUniforms*>(
			getGrManager().allocateFrameHostVisibleMemory(
				sizeof(*commonUniforms),
				BufferUsage::UNIFORM,
				m_commonUniformsToken));

	commonUniforms->m_projectionParams = frc.getProjectionParameters();
	commonUniforms->m_nearFarLinearizeDepth.x() = frc.getFrustum().getNear();
	commonUniforms->m_nearFarLinearizeDepth.y() = frc.getFrustum().getFar();
	computeLinearizeDepthOptimal(frc.getFrustum().getNear(),
		frc.getFrustum().getFar(),
		commonUniforms->m_nearFarLinearizeDepth.z(),
		commonUniforms->m_nearFarLinearizeDepth.w());

	commonUniforms->m_projectionMatrix = frc.getProjectionMatrix();

	// Run stages
	if(m_ir)
	{
		ANKI_CHECK(m_ir->run(ctx));
	}

	ANKI_CHECK(m_is->populateBuffers(ctx));

	ANKI_CHECK(buildCommandBuffers(ctx));

	if(m_sm)
	{
		m_sm->run(ctx);
	}

	m_ms->run(ctx);
	m_lf->runOcclusionTests(ctx);
	cmdb->endRenderPass();

	m_is->run(ctx);

	cmdb->generateMipmaps(m_ms->getDepthRt());
	cmdb->generateMipmaps(m_ms->getRt2());

	m_fs->run(ctx);
	m_lf->run(ctx);
	m_vol->run(ctx);
	cmdb->endRenderPass();

	if(m_ssao)
	{
		m_ssao->run(ctx);
	}

	m_upsample->run(ctx);

	if(m_downscale)
	{
		m_downscale->run(ctx);
	}

	if(m_tm)
	{
		m_tm->run(ctx);
	}

	if(m_bloom)
	{
		m_bloom->run(ctx);
	}

	if(m_sslf)
	{
		m_sslf->run(ctx);
	}

	if(m_pps)
	{
		m_pps->run(ctx);
	}

	if(m_dbg->getEnabled())
	{
		ANKI_CHECK(m_dbg->run(ctx));
	}

	++m_frameCount;

	return ErrorCode::NONE;
}

//==============================================================================
Vec3 Renderer::unproject(const Vec3& windowCoords,
	const Mat4& modelViewMat,
	const Mat4& projectionMat,
	const int view[4])
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

//==============================================================================
void Renderer::createRenderTarget(U32 w,
	U32 h,
	const PixelFormat& format,
	U32 samples,
	SamplingFilter filter,
	U mipsCount,
	TexturePtr& rt)
{
	// Not very important but keep the resolution of render targets aligned to
	// 16
	if(0)
	{
		ANKI_ASSERT(isAligned(16, w));
		ANKI_ASSERT(isAligned(16, h));
	}

	TextureInitInfo init;

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 0;
	init.m_type = TextureType::_2D;
	init.m_format = format;
	init.m_mipmapsCount = mipsCount;
	init.m_samples = samples;
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

//==============================================================================
void Renderer::createDrawQuadPipeline(
	ShaderPtr frag, const ColorStateInfo& colorState, PipelinePtr& ppline)
{
	PipelineInitInfo init;

	init.m_inputAssembler.m_topology = PrimitiveTopology::TRIANGLE_STRIP;

	init.m_depthStencil.m_depthWriteEnabled = false;
	init.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	init.m_color = colorState;

	init.m_shaders[U(ShaderType::VERTEX)] = m_drawQuadVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = frag;
	ppline = m_gr->newInstance<Pipeline>(init);
}

//==============================================================================
void Renderer::prepareForVisibilityTests(const SceneNode& cam)
{
	m_tiler->prepareForVisibilityTests(cam);
}

//==============================================================================
Bool Renderer::doGpuVisibilityTest(
	const CollisionShape& cs, const Aabb& aabb) const
{
	return m_tiler->test(cs, aabb);
}

//==============================================================================
Error Renderer::buildCommandBuffers(RenderingContext& ctx)
{
	ANKI_TRACE_START_EVENT(RENDERER_COMMAND_BUFFER_BUILDING);

	// Prepare
	m_ms->prepareBuildCommandBuffers(ctx);
	if(m_sm)
	{
		m_sm->prepareBuildCommandBuffers(ctx);
	}
	m_fs->prepareBuildCommandBuffers(ctx);

	// Build
	class Task : public ThreadPool::Task
	{
	public:
		Renderer* m_r ANKI_DBG_NULLIFY_PTR;
		RenderingContext* m_ctx ANKI_DBG_NULLIFY_PTR;

		Error operator()(U32 threadId, PtrSize threadsCount)
		{
			ANKI_CHECK(m_r->getMs().buildCommandBuffers(
				*m_ctx, threadId, threadsCount));

			if(m_r->getSmEnabled())
			{
				ANKI_CHECK(m_r->getSm().buildCommandBuffers(
					*m_ctx, threadId, threadsCount));
			}

			ANKI_CHECK(m_r->getFs().buildCommandBuffers(
				*m_ctx, threadId, threadsCount));

			return ErrorCode::NONE;
		}
	};

	ThreadPool& threadPool = getThreadPool();
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

} // end namespace anki

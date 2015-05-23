// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/renderer/Renderer.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Counters.h"
#include "anki/misc/ConfigSet.h"

#include "anki/renderer/Fs.h"
#include "anki/renderer/Lf.h"
#include "anki/renderer/Is.h"

namespace anki {

//==============================================================================
Renderer::Renderer()
:	m_ms(this),
	m_is(this),
	m_pps(this),
	m_dbg(this),
	m_tiler(this)
{}

//==============================================================================
Renderer::~Renderer()
{
	m_shadersPrependedSource.destroy(m_alloc);

	if(m_fs)
	{
		m_alloc.deleteInstance(m_fs);
	}

	if(m_lf)
	{
		m_alloc.deleteInstance(m_lf);
	}
}

//==============================================================================
Error Renderer::init(
	Threadpool* threadpool,
	ResourceManager* resources,
	GrManager* gl,
	AllocAlignedCallback allocCb,
	void* allocCbUserData,
	const ConfigSet& config,
	const Timestamp* globalTimestamp)
{
	m_grobalTimestamp = globalTimestamp;
	m_threadpool = threadpool;
	m_resources = resources;
	m_gr = gl;
	m_alloc = HeapAllocator<U8>(allocCb, allocCbUserData);
	m_frameAlloc = StackAllocator<U8>(allocCb, allocCbUserData, 1024 * 1024);

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
	m_renderingQuality = config.get("renderingQuality");
	m_defaultFbWidth = config.get("width");
	m_defaultFbHeight = config.get("height");
	m_width = getAlignedRoundDown(
		TILE_SIZE, U(m_defaultFbWidth * m_renderingQuality));
	m_height = getAlignedRoundDown(
		TILE_SIZE, U(m_defaultFbHeight * m_renderingQuality));
	m_lodDistance = config.get("lodDistance");
	m_framesNum = 0;
	m_samples = config.get("samples");
	m_isOffscreen = config.get("offscreen");
	m_tilesCount.x() = m_width / TILE_SIZE;
	m_tilesCount.y() = m_height / TILE_SIZE;
	m_tilesCountXY = m_tilesCount.x() * m_tilesCount.y();

	m_tessellation = config.get("tessellation");

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

	if(m_width % m_tilesCount.x() != 0)
	{
		ANKI_LOGE("Width is not multiple of tile width");
		return ErrorCode::USER_DATA;
	}

	if(m_height % m_tilesCount.y() != 0)
	{
		ANKI_LOGE("Height is not multiple of tile height");
		return ErrorCode::USER_DATA;
	}

	// Set the default preprocessor string
	m_shadersPrependedSource.sprintf(
		m_alloc,
		"#define ANKI_RENDERER_WIDTH %u\n"
		"#define ANKI_RENDERER_HEIGHT %u\n",
		m_width, m_height);

	// Drawer
	ANKI_CHECK(m_sceneDrawer.create(this));

	// quad setup
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {-1.0, 1.0},
		{1.0, -1.0}, {-1.0, -1.0}};

	ANKI_CHECK(m_quadPositionsBuff.create(m_gr, GL_ARRAY_BUFFER,
		&quadVertCoords[0][0], sizeof(quadVertCoords), 0));

	ANKI_CHECK(m_drawQuadVert.load("shaders/Quad.vert.glsl", m_resources));

	// Init the stages. Careful with the order!!!!!!!!!!
	ANKI_CHECK(m_tiler.init());

	ANKI_CHECK(m_ms.init(config));
	ANKI_CHECK(m_is.init(config));

	m_fs = m_alloc.newInstance<Fs>(this);
	ANKI_CHECK(m_fs->init(config));

	m_lf = m_alloc.newInstance<Lf>(this);
	ANKI_CHECK(m_lf->init(config));

	ANKI_CHECK(m_pps.init(config));
	ANKI_CHECK(m_dbg.init(config));

	// Default FB
	FramebufferPtr::Initializer fbInit;
	ANKI_CHECK(m_defaultFb.create(m_gr, fbInit));

	return ErrorCode::NONE;
}

//==============================================================================
Error Renderer::render(SceneGraph& scene,
	Array<CommandBufferPtr, JOB_CHAINS_COUNT>& cmdBuff)
{
	m_scene = &scene;
	m_frameAlloc.getMemoryPool().reset();
	Camera& cam = m_scene->getActiveCamera();

	// Calc a few vars
	//
	const FrustumComponent& fr = cam.getComponent<FrustumComponent>();
	Timestamp camUpdateTimestamp = fr.getTimestamp();
	if(m_projectionParamsUpdateTimestamp
			< m_scene->getActiveCameraChangeTimestamp()
		|| m_projectionParamsUpdateTimestamp < camUpdateTimestamp
		|| m_projectionParamsUpdateTimestamp == 0)
	{
		const FrustumComponent& frc = cam.getComponent<FrustumComponent>();

		m_projectionParams = frc.getProjectionParameters();
		m_projectionParamsUpdateTimestamp = getGlobalTimestamp();
	}

	ANKI_COUNTER_START_TIMER(RENDERER_MS_TIME);
	ANKI_CHECK(m_ms.run(cmdBuff[0]));
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_MS_TIME);

	m_lf->runOcclusionTests(cmdBuff[0]);

	m_ms.generateMipmaps(cmdBuff[0]);

	m_tiler.runMinMax(cmdBuff[0]);
	cmdBuff[0].flush();

	ANKI_COUNTER_START_TIMER(RENDERER_IS_TIME);
	ANKI_CHECK(m_is.run(cmdBuff[1]));
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_IS_TIME);

	ANKI_CHECK(m_fs->run(cmdBuff[1]));
	m_lf->run(cmdBuff[1]);

	ANKI_COUNTER_START_TIMER(RENDERER_PPS_TIME);
	if(m_pps.getEnabled())
	{
		ANKI_CHECK(m_pps.run(cmdBuff[1]));
	}
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_PPS_TIME);

	if(m_dbg.getEnabled())
	{
		ANKI_CHECK(m_dbg.run(cmdBuff[1]));
	}

	++m_framesNum;

	return ErrorCode::NONE;
}

//==============================================================================
void Renderer::drawQuad(CommandBufferPtr& cmdBuff)
{
	drawQuadInstanced(cmdBuff, 1);
}

//==============================================================================
void Renderer::drawQuadConditional(OcclusionQueryPtr& q,
	CommandBufferPtr& cmdBuff)
{
	m_quadPositionsBuff.bindVertexBuffer(cmdBuff, 2, GL_FLOAT, false, 0, 0, 0);
	cmdBuff.drawArraysConditional(q, GL_TRIANGLE_STRIP, 4, 1);
}

//==============================================================================
void Renderer::drawQuadInstanced(
	CommandBufferPtr& cmdBuff, U32 primitiveCount)
{
	m_quadPositionsBuff.bindVertexBuffer(cmdBuff, 2, GL_FLOAT, false, 0, 0, 0);

	cmdBuff.drawArrays(GL_TRIANGLE_STRIP, 4, primitiveCount);
}

//==============================================================================
Vec3 Renderer::unproject(const Vec3& windowCoords, const Mat4& modelViewMat,
	const Mat4& projectionMat, const int view[4])
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
Error Renderer::createRenderTarget(U32 w, U32 h, const PixelFormat& format,
	U32 samples, SamplingFilter filter, U mipsCount, TexturePtr& rt)
{
	// Not very important but keep the resulution of render targets aligned to
	// 16
	if(0)
	{
		ANKI_ASSERT(isAligned(16, w));
		ANKI_ASSERT(isAligned(16, h));
	}

	TexturePtr::Initializer init;

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

	CommandBufferPtr cmdBuff;
	ANKI_CHECK(cmdBuff.create(m_gr));

	rt.create(cmdBuff, init);
	cmdBuff.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Renderer::createDrawQuadPipeline(
	ShaderPtr frag, PipelinePtr& ppline)
{
	PipelinePtr::Initializer init;
	init.m_shaders[U(ShaderType::VERTEX)] = m_drawQuadVert->getGrShader();
	init.m_shaders[U(ShaderType::FRAGMENT)] = frag;

	return ppline.create(m_gr, init);
}

} // end namespace anki

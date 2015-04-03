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

namespace anki {

//==============================================================================
Renderer::Renderer()
:	m_ms(this), 
	m_dp(this), 
	m_is(this),
	m_pps(this),
	m_dbg(this), 
	m_tiler(this)
{}

//==============================================================================
Renderer::~Renderer()
{
	m_shadersPrependedSource.destroy(m_alloc);
}

//==============================================================================
Error Renderer::init(
	Threadpool* threadpool, 
	ResourceManager* resources,
	GrManager* gl,
	HeapAllocator<U8>& alloc,
	const ConfigSet& config,
	const Timestamp* globalTimestamp)
{
	m_globalTimestamp = globalTimestamp;
	m_threadpool = threadpool;
	m_resources = resources;
	m_gl = gl;
	m_alloc = alloc;

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
	m_width = m_defaultFbWidth * m_renderingQuality;
	m_height = m_defaultFbHeight * m_renderingQuality;
	m_lodDistance = config.get("lodDistance");
	m_framesNum = 0;
	m_samples = config.get("samples");
	m_isOffscreen = config.get("offscreen");
	m_tilesCount.x() = config.get("tilesXCount");
	m_tilesCount.y() = config.get("tilesYCount");
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

	// Drawer
	ANKI_CHECK(m_sceneDrawer.create(this));

	// quad setup
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {-1.0, 1.0}, 
		{1.0, -1.0}, {-1.0, -1.0}};

	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(m_gl));

	ANKI_CHECK(m_quadPositionsBuff.create(cmdBuff, GL_ARRAY_BUFFER, 
		&quadVertCoords[0][0], sizeof(quadVertCoords), 0));

	ANKI_CHECK(m_drawQuadVert.load("shaders/Quad.vert.glsl", m_resources));

	// Init the stages. Careful with the order!!!!!!!!!!
	ANKI_CHECK(m_tiler.init());

	ANKI_CHECK(m_ms.init(config));
	ANKI_CHECK(m_dp.init(config));
	ANKI_CHECK(m_is.init(config));
	
	m_fs = m_alloc.newInstance<Fs>(this);
	ANKI_CHECK(m_fs->init(config));
	ANKI_CHECK(m_pps.init(config));
	ANKI_CHECK(m_dbg.init(config));

	// Default FB
	FramebufferHandle::Initializer fbInit;
	ANKI_CHECK(m_defaultFb.create(cmdBuff, fbInit));

	cmdBuff.finish();

	// Set the default preprocessor string
	m_shadersPrependedSource.sprintf(
		m_alloc,
		"#define ANKI_RENDERER_WIDTH %u\n"
		"#define ANKI_RENDERER_HEIGHT %u\n",
		m_width, m_height);

	return ErrorCode::NONE;
}

//==============================================================================
Error Renderer::render(SceneGraph& scene, 
	Array<CommandBufferHandle, JOB_CHAINS_COUNT>& cmdBuff)
{
	m_scene = &scene;
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

	m_tiler.runMinMax(m_ms._getDepthRt(), cmdBuff[0]);
	cmdBuff[0].flush();

	if(m_pps.getEnabled() && m_pps.getLf().getEnabled())
	{
		ANKI_CHECK(m_pps.getLf().runOcclusionTests(cmdBuff[1]));
	}

	ANKI_COUNTER_START_TIMER(RENDERER_IS_TIME);
	ANKI_CHECK(m_is.run(cmdBuff[1]));
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_IS_TIME);

	ANKI_CHECK(m_fs->run(cmdBuff[1])); 

	ANKI_CHECK(m_dp.run(cmdBuff[1]));

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
void Renderer::drawQuad(CommandBufferHandle& cmdBuff)
{
	drawQuadInstanced(cmdBuff, 1);
}

//==============================================================================
void Renderer::drawQuadConditional(OcclusionQueryHandle& q,
	CommandBufferHandle& cmdBuff)
{
	m_quadPositionsBuff.bindVertexBuffer(cmdBuff, 2, GL_FLOAT, false, 0, 0, 0);
	cmdBuff.drawArraysConditional(q, GL_TRIANGLE_STRIP, 4, 1);
}

//==============================================================================
void Renderer::drawQuadInstanced(
	CommandBufferHandle& cmdBuff, U32 primitiveCount)
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
Error Renderer::createRenderTarget(U32 w, U32 h, GLenum internalFormat, 
	U32 samples, TextureHandle& rt)
{
	// Not very important but keep the resulution of render targets aligned to
	// 16
	if(0)
	{
		ANKI_ASSERT(isAligned(16, w));
		ANKI_ASSERT(isAligned(16, h));
	}

	TextureHandle::Initializer init;

	init.m_width = w;
	init.m_height = h;
	init.m_depth = 0;
#if ANKI_GL == ANKI_GL_DESKTOP
	init.m_target = (samples == 1) ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE;
#else
	ANKI_ASSERT(samples == 1);
	init.m_target = GL_TEXTURE_2D;
#endif
	init.m_internalFormat = internalFormat;
	init.m_mipmapsCount = 1;
	init.m_filterType = TextureHandle::Filter::NEAREST;
	init.m_repeat = false;
	init.m_anisotropyLevel = 0;
	init.m_samples = samples;

	CommandBufferHandle cmdBuff;
	ANKI_CHECK(cmdBuff.create(m_gl));

	rt.create(cmdBuff, init);
	cmdBuff.finish();

	return ErrorCode::NONE;
}

//==============================================================================
Error Renderer::createDrawQuadPipeline(
	ShaderHandle frag, PipelineHandle& ppline)
{
	CommandBufferHandle cmdBuff;
	Error err = cmdBuff.create(m_gl);

	if(!err)
	{
		Array<ShaderHandle, 2> progs = 
			{{m_drawQuadVert->getGrShader(), frag}};

		err = ppline.create(cmdBuff, &progs[0], &progs[0] + 2);
	}

	if(!err)
	{
		cmdBuff.finish();
	}

	return err;
}

} // end namespace anki

#include "anki/renderer/Renderer.h"
#include "anki/util/Exception.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Counters.h"
#include <sstream>

namespace anki {

//==============================================================================
// RendererInitializer                                                         =
//==============================================================================

//==============================================================================
RendererInitializer::RendererInitializer()
{
	// Ms
	newOption("ms.ez.enabled", false);
	newOption("ms.ez.maxObjectsToDraw", 10);

	// Is
	newOption("is.sm.enabled", true);
	newOption("is.sm.poissonEnabled", true);
	newOption("is.sm.bilinearEnabled", true);
	newOption("is.sm.resolution", 512);
	newOption("is.sm.maxLights", 4);

	newOption("is.groundLightEnabled", true);
	newOption("is.maxPointLights", 512 - 16);
	newOption("is.maxSpotLights", 8);
	newOption("is.maxSpotTexLights", 4);
	newOption("is.maxPointLightsPerTile", 48);
	newOption("is.maxSpotLightsPerTile", 4);
	newOption("is.maxSpotTexLightsPerTile", 4);

	// Pps
	newOption("pps.hdr.enabled", true);
	newOption("pps.hdr.renderingQuality", 0.5);
	newOption("pps.hdr.blurringDist", 1.0);
	newOption("pps.hdr.samples", 5);
	newOption("pps.hdr.blurringIterationsCount", 1);
	newOption("pps.hdr.exposure", 4.0);

	newOption("pps.ssao.enabled", true);
	newOption("pps.ssao.renderingQuality", 0.3);
	newOption("pps.ssao.blurringIterationsNum", 2);

	newOption("pps.bl.enabled", true);
	newOption("pps.bl.blurringIterationsNum", 2);
	newOption("pps.bl.sideBlurFactor", 1.0);

	newOption("pps.lf.enabled", true);
	newOption("pps.lf.maxFlaresPerLight", 8);
	newOption("pps.lf.maxLightsWithFlares", 4);

	newOption("pps.enabled", true);
	newOption("pps.sharpen", true);
	newOption("pps.gammaCorrection", true);

	// Dbg
	newOption("dbg.enabled", false);

	// Globals
	newOption("width", 0);
	newOption("height", 0);
	newOption("renderingQuality", 1.0); // Applies only to MainRenderer
	newOption("lodDistance", 10.0); // Distance that used to calculate the LOD
	newOption("samples", 1);
	newOption("tilesXCount", 16);
	newOption("tilesYCount", 16);
	newOption("tessellation", true);

	newOption("maxTextureSize", 1048576); // Cap to limit quality in resources
	newOption("offscreen", false);
}

//==============================================================================
// Renderer                                                                    =
//==============================================================================

//==============================================================================
Renderer::Renderer()
	:	m_ms(this), 
		m_is(this),
		m_pps(this),
		m_bs(this),
		m_dbg(this), 
		m_sceneDrawer(this)
{}

//==============================================================================
Renderer::~Renderer()
{}

//==============================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// Set from the initializer
	m_width = initializer.get("width");
	alignRoundUp(16, m_width);
	m_height = initializer.get("height");
	alignRoundUp(16, m_height);
	m_lodDistance = initializer.get("lodDistance");
	m_framesNum = 0;
	m_samples = initializer.get("samples");
	m_isOffscreen = initializer.get("offscreen");
	m_renderingQuality = initializer.get("renderingQuality");
	m_maxTextureSize = initializer.get("maxTextureSize");
	m_tilesCount.x() = initializer.get("tilesXCount");
	m_tilesCount.y() = initializer.get("tilesYCount");

	m_tessellation = initializer.get("tessellation");

	// A few sanity checks
	if(m_samples != 1 && m_samples != 4 && m_samples != 8 && m_samples != 16
		&& m_samples != 32)
	{
		throw ANKI_EXCEPTION("Incorrect samples");
	}

	if(m_width < 10 || m_height < 10)
	{
		throw ANKI_EXCEPTION("Incorrect sizes");
	}

	// quad setup
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {-1.0, 1.0}, 
		{1.0, -1.0}, {-1.0, -1.0}};

	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	GlClientBufferHandle tmpBuff = GlClientBufferHandle(jobs, 
		sizeof(quadVertCoords), (void*)&quadVertCoords[0][0]);

	m_quadPositionsBuff = GlBufferHandle(jobs, GL_ARRAY_BUFFER, 
		tmpBuff, 0);

	m_drawQuadVert.load("shaders/Quad.vert.glsl");

	// Init the stages. Careful with the order!!!!!!!!!!
	m_tiler.init(this);

	m_ms.init(initializer);;
	m_is.init(initializer);
	m_bs.init(initializer);
	m_pps.init(initializer);
	m_bs.init(initializer);
	m_dbg.init(initializer);

	// Init the shaderPostProcessorString
	std::stringstream ss;
	ss << "#define RENDERING_WIDTH " << m_width << "\n"
		<< "#define RENDERING_HEIGHT " << m_height << "\n";

	m_shaderPostProcessorString = ss.str();

	// Default FB
	m_defaultFb = GlFramebufferHandle(jobs, {});

	jobs.flush();
}

//==============================================================================
void Renderer::render(SceneGraph& scene, GlJobChainHandle& jobs)
{
	m_scene = &scene;
	Camera& cam = m_scene->getActiveCamera();

	// Calc a few vars
	//
	Timestamp camUpdateTimestamp = cam.FrustumComponent::getTimestamp();
	if(m_planesUpdateTimestamp < m_scene->getActiveCameraChangeTimestamp()
		|| m_planesUpdateTimestamp < camUpdateTimestamp
		|| m_planesUpdateTimestamp == 1)
	{
		calcPlanes(Vec2(cam.getNear(), cam.getFar()), m_planes);

		ANKI_ASSERT(cam.getCameraType() == Camera::CT_PERSPECTIVE);
		const PerspectiveCamera& pcam =
			static_cast<const PerspectiveCamera&>(cam);

		calcLimitsOfNearPlane(pcam, m_limitsOfNearPlane);
		m_limitsOfNearPlane2 = m_limitsOfNearPlane * 2.0;

		m_planesUpdateTimestamp = getGlobTimestamp();
	}

	ANKI_COUNTER_START_TIMER(RENDERER_MS_TIME);
	m_ms.run(jobs);
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_MS_TIME);

	m_tiler.runMinMax(m_ms.getDepthRt());

	ANKI_COUNTER_START_TIMER(RENDERER_IS_TIME);
	m_is.run(jobs);
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_IS_TIME);

	m_bs.run(jobs);

	ANKI_COUNTER_START_TIMER(RENDERER_PPS_TIME);
	if(m_pps.getEnabled())
	{
		m_pps.run(jobs);
	}
	ANKI_COUNTER_STOP_TIMER_INC(RENDERER_PPS_TIME);

	if(m_dbg.getEnabled())
	{
		m_dbg.run(jobs);
	}

	++m_framesNum;
}

//==============================================================================
void Renderer::drawQuad(GlJobChainHandle& jobs)
{
	drawQuadInstanced(jobs, 1);
}

//==============================================================================
void Renderer::drawQuadInstanced(GlJobChainHandle& jobs, U32 primitiveCount)
{
	m_quadPositionsBuff.bindVertexBuffer(jobs, 2, GL_FLOAT, false, 0, 0, 0);

	GlDrawcallArrays dc(GL_TRIANGLE_STRIP, 4, primitiveCount);
	dc.draw(jobs);
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
void Renderer::calcPlanes(const Vec2& cameraRange, Vec2& planes)
{
	F32 zNear = cameraRange.x();
	F32 zFar = cameraRange.y();

	F32 opt = zNear - zFar;

	planes.x() = zFar / opt;
	planes.y() = (zFar * zNear) / opt;
}

//==============================================================================
void Renderer::calcLimitsOfNearPlane(const PerspectiveCamera& pcam,
	Vec2& limitsOfNearPlane)
{
	limitsOfNearPlane.y() = tan(0.5 * pcam.getFovY());
	limitsOfNearPlane.x() = tan(0.5 * pcam.getFovX());
}

//==============================================================================
void Renderer::createRenderTarget(U32 w, U32 h, GLenum internalFormat, 
	GLenum format, GLenum type, U32 samples, GlTextureHandle& rt)
{
	// Not very important but keep the resulution of render targets aligned to
	// 16
	ANKI_ASSERT(isAligned(16, w));
	ANKI_ASSERT(isAligned(16, h));

	GlTextureHandle::Initializer init;

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
	init.m_format = format;
	init.m_type = type;
	init.m_mipmapsCount = 1;
	init.m_filterType = GlTextureHandle::Filter::NEAREST;
	init.m_repeat = false;
	init.m_anisotropyLevel = 0;
	init.m_genMipmaps = false;
	init.m_samples = samples;

	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);
	rt = GlTextureHandle(jobs, init);
	jobs.flush();
}

//==============================================================================
GlProgramPipelineHandle Renderer::createDrawQuadProgramPipeline(
	GlProgramHandle frag)
{
	GlManager& gl = GlManagerSingleton::get();
	GlJobChainHandle jobs(&gl);

	Array<GlProgramHandle, 2> progs = {{m_drawQuadVert->getGlProgram(), frag}};

	GlProgramPipelineHandle ppline(jobs, &progs[0], &progs[0] + 2);
	jobs.finish();

	return ppline;
}

} // end namespace anki

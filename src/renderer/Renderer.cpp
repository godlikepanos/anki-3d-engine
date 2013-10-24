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
	newOption("pps.hdr.renderingQuality", 0.25);
	newOption("pps.hdr.blurringDist", 1.0);
	newOption("pps.hdr.blurringIterationsCount", 2);
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

	if(GlStateCommonSingleton::get().getGpu() == GlStateCommon::GPU_ARM)
	{
		newOption("mrt", false); // Pack or not the GBuffer
	}
	else
	{
		newOption("mrt", true); // Pack or not the GBuffer
	}

	newOption("maxTextureSize", 1048576); // Cap to limit quality in resources
	newOption("offscreen", false);
}

//==============================================================================
// Renderer                                                                    =
//==============================================================================

//==============================================================================
Renderer::Renderer()
	: ms(this), is(this), pps(this), bs(this), dbg(this), sceneDrawer(this)
{}

//==============================================================================
Renderer::~Renderer()
{}

//==============================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// set from the initializer
	width = initializer.get("width");
	height = initializer.get("height");
	lodDistance = initializer.get("lodDistance");
	framesNum = 0;
	samples = initializer.get("samples");
	useMrt = initializer.get("mrt");
	isOffscreen = initializer.get("offscreen");
	renderingQuality = initializer.get("renderingQuality");
	maxTextureSize = initializer.get("maxTextureSize");
	tilesCount.x() = initializer.get("tilesXCount");
	tilesCount.y() = initializer.get("tilesYCount");

	tessellation = GlStateCommonSingleton::get().isTessellationSupported() 
		&& (U)initializer.get("tessellation");

	// a few sanity checks
	if(samples != 1 && samples != 4 && samples != 8 && samples != 16
		&& samples != 32)
	{
		throw ANKI_EXCEPTION("Incorrect samples");
	}

	if(width < 10 || height < 10)
	{
		throw ANKI_EXCEPTION("Incorrect sizes");
	}

	// init the stages. Careful with the order!!!!!!!!!!
	tiler.init(this);

	ms.init(initializer);;
	is.init(initializer);
	bs.init(initializer);
	pps.init(initializer);
	bs.init(initializer);
	dbg.init(initializer);

	// quad VBOs and VAO
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {-1.0, 1.0}, 
		{1.0, -1.0}, {-1.0, -1.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(
		&quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, 0);

	// Init the shaderPostProcessorString
	std::stringstream ss;
	ss << "#define USE_MRT " << (U)useMrt << "\n"
		<< "#define RENDERING_WIDTH " << width << "\n"
		<< "#define RENDERING_HEIGHT " << height << "\n";

	shaderPostProcessorString = ss.str();
}

//==============================================================================
void Renderer::render(SceneGraph& scene_)
{
	scene = &scene_;
	Camera& cam = scene->getActiveCamera();

	// Calc a few vars
	//
	U32 camUpdateTimestamp = cam.getFrustumComponent()->getTimestamp();
	if(planesUpdateTimestamp < scene->getActiveCameraChangeTimestamp()
		|| planesUpdateTimestamp < camUpdateTimestamp
		|| planesUpdateTimestamp == 1)
	{
		calcPlanes(Vec2(cam.getNear(), cam.getFar()), planes);

		ANKI_ASSERT(cam.getCameraType() == Camera::CT_PERSPECTIVE);
		const PerspectiveCamera& pcam =
			static_cast<const PerspectiveCamera&>(cam);

		calcLimitsOfNearPlane(pcam, limitsOfNearPlane);
		limitsOfNearPlane2 = limitsOfNearPlane * 2.0;

		planesUpdateTimestamp = getGlobTimestamp();
	}

	ANKI_COUNTER_START_TIMER(C_RENDERER_MS_TIME);
	ms.run();
	ANKI_COUNTER_STOP_TIMER_INC(C_RENDERER_MS_TIME);

	tiler.runMinMax(ms.getDepthFai());

	ANKI_COUNTER_START_TIMER(C_RENDERER_IS_TIME);
	is.run();
	ANKI_COUNTER_STOP_TIMER_INC(C_RENDERER_IS_TIME);

	bs.run();

	ANKI_COUNTER_START_TIMER(C_RENDERER_PPS_TIME);
	if(pps.getEnabled())
	{
		pps.run();
	}
	ANKI_COUNTER_STOP_TIMER_INC(C_RENDERER_PPS_TIME);

	if(dbg.getEnabled())
	{
		dbg.run();
	}

	ANKI_CHECK_GL_ERROR();
	++framesNum;
}

//==============================================================================
void Renderer::drawQuad()
{
	quadVao.bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

//==============================================================================
void Renderer::drawQuadInstanced(U32 primitiveCount)
{
	quadVao.bind();
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, primitiveCount);
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

	Vec4 final = invPm * vec;
	final /= final.w();
	return Vec3(final);
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
void Renderer::clearAfterBindingFbo(const GLenum cap)
{
	if(GlStateCommonSingleton::get().getGpu() == GlStateCommon::GPU_ARM)
	{
		glClear(cap);
	}
}

} // end namespace anki

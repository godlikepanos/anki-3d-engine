#include "anki/renderer/Renderer.h"
#include "anki/util/Exception.h"
#include "anki/scene/Camera.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Counters.h"

namespace anki {

//==============================================================================
Renderer::Renderer()
	: ms(this), is(this), pps(this), bs(this), sceneDrawer(this)
{}

//==============================================================================
Renderer::~Renderer()
{}

//==============================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// set from the initializer
	width = initializer.width;
	height = initializer.height;
	lodDistance = initializer.lodDistance;
	framesNum = 0;
	samples = initializer.samples;

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

	// quad VBOs and VAO
	static const F32 quadVertCoords[][2] = {{1.0, 1.0}, {-1.0, 1.0}, 
		{1.0, -1.0}, {-1.0, -1.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(
		&quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, 0);
}

//==============================================================================
void Renderer::render(SceneGraph& scene_)
{
	scene = &scene_;
	Camera& cam = scene->getActiveCamera();

	// Calc a few vars
	//
	U32 camUpdateTimestamp = cam.getFrustumable()->getFrustumableTimestamp();
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

	viewProjectionMat = cam.getViewProjectionMatrix();

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

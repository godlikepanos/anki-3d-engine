#include "anki/renderer/Renderer.h"
#include "anki/util/Exception.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Scene.h"

namespace anki {

//==============================================================================
Renderer::Renderer()
	: ms(this), is(this), pps(this), bs(this), width(640), height(480),
		sceneDrawer(this)
{
	enableStagesProfilingFlag = false;
}

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

	// a few sanity checks
	if(width < 10 || height < 10)
	{
		throw ANKI_EXCEPTION("Incorrect sizes");
	}

	// init the stages. Careful with the order!!!!!!!!!!
	ms.init(initializer);
	is.init(initializer);
	pps.init(initializer);
	bs.init(initializer);

	// quad VBOs and VAO
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0},
		{1.0, 0.0}}; /// XXX change them to NDC
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	quadVertIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces),
		quadVertIndeces, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(quadPositionsVbo, 0, 2, GL_FLOAT, false, 0,
		NULL);
	quadVao.attachElementArrayBufferVbo(quadVertIndecesVbo);

	// Other
	msTq.reset(new Query(GL_TIME_ELAPSED));
	isTq.reset(new Query(GL_TIME_ELAPSED));
	ppsTq.reset(new Query(GL_TIME_ELAPSED));
	bsTq.reset(new Query(GL_TIME_ELAPSED));
}

//==============================================================================
void Renderer::render(Scene& scene_)
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

		planesUpdateTimestamp = Timestamp::getTimestamp();
	}

	viewProjectionMat = cam.getViewProjectionMatrix();

	if(enableStagesProfilingFlag)
	{
		msTq->begin();
		ms.run();
		msTq->end();

		isTq->begin();
		is.run();
		isTq->end();

		ppsTq->begin();
		pps.runPrePass();
		ppsTq->end();

		bsTq->begin();
		bs.run();
		bsTq->end();

		pps2Tq->begin();
		pps.runPostPass();
		pps2Tq->end();

		// Now wait
		msTime = msTq->getResult();
		isTime = isTq->getResult();
		bsTime = bsTq->getResult();
		ppsTime = ppsTq->getResult() + ppsTq->getResult();
	}
	else
	{
		ms.run();
		is.run();
		/*pps.runPrePass();
		bs.run();
		pps.runPostPass();*/
	}

	++framesNum;
}

//==============================================================================
void Renderer::drawQuad()
{
	quadVao.bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
}

//==============================================================================
void Renderer::drawQuadInstanced(U32 primitiveCount)
{
	quadVao.bind();
	glDrawElementsInstanced(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0,
		primitiveCount);
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
void Renderer::createFai(uint width, uint height, int internalFormat,
	int format, int type, Texture& fai)
{
	Texture::Initializer init;
	init.width = width;
	init.height = height;
	init.internalFormat = internalFormat;
	init.format = format;
	init.type = type;
	init.data = NULL;
	init.mipmapping = false;
	init.filteringType = Texture::TFT_LINEAR;
	init.repeat = false;
	init.anisotropyLevel = 0;

	fai.create(init);
}

//==============================================================================
void Renderer::calcPlanes(const Vec2& cameraRange, Vec2& planes)
{
	float zNear = cameraRange.x();
	float zFar = cameraRange.y();

	planes.x() = zFar / (zNear - zFar);
	planes.y() = (zFar * zNear) / (zNear -zFar);
}

//==============================================================================
void Renderer::calcLimitsOfNearPlane(const PerspectiveCamera& pcam,
	Vec2& limitsOfNearPlane)
{
#if 0
	limitsOfNearPlane.y() = pcam.getNear() * tan(0.5 * pcam.getFovY());
	limitsOfNearPlane.x() = limitsOfNearPlane.y()
		* (pcam.getFovX() / pcam.getFovY());
#else
	limitsOfNearPlane.y() = pcam.getNear() * tan(0.5 * pcam.getFovY());
	limitsOfNearPlane.x() = pcam.getNear() * tan(0.5 * pcam.getFovX());
#endif
}

} // end namespace

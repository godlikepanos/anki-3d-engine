#include "anki/renderer/Renderer.h"
#include "anki/renderer/RendererInitializer.h"
#include "anki/util/Exception.h"
#include "anki/scene/PerspectiveCamera.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Renderer::Renderer():
	ms(*this),
	is(*this),
	pps(*this),
	bs(*this),
	width(640),
	height(480),
	sceneDrawer(*this)
{
	enableStagesProfilingFlag = false;
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Renderer::~Renderer()
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// set from the initializer
	width = initializer.width;
	height = initializer.height;
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
		{1.0, 0.0}};
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
	msTq.reset(new TimeQuery);
	isTq.reset(new TimeQuery);
	ppsTq.reset(new TimeQuery);
	bsTq.reset(new TimeQuery);
}


//==============================================================================
// render                                                                      =
//==============================================================================
void Renderer::render(Camera& cam_)
{
	cam = &cam_;

	//
	// Calc a few vars
	//
	calcPlanes(Vec2(cam->getZNear(), cam->getZFar()), planes);

	ASSERT(cam->getCameraType() == Camera::CT_PERSPECTIVE);
	const PerspectiveCamera& pcam = static_cast<const PerspectiveCamera&>(*cam);
	calcLimitsOfNearPlane(pcam, limitsOfNearPlane);
	limitsOfNearPlane2 = limitsOfNearPlane * 2.0;

	viewProjectionMat = cam->getProjectionMatrix() * cam->getViewMatrix();

	if(enableStagesProfilingFlag)
	{
		msTq->begin();
		ms.run();
		msTime = msTq->end();

		isTq->begin();
		is.run();
		isTime = isTq->end();

		ppsTq->begin();
		pps.runPrePass();
		ppsTime = ppsTq->end();

		bsTq->begin();
		bs.run();
		bsTime = bsTq->end();

		ppsTq->begin();
		pps.runPostPass();
		ppsTime += ppsTq->end();
	}
	else
	{
		ms.run();
		is.run();
		pps.runPrePass();
		bs.run();
		pps.runPostPass();
	}

	++framesNum;
}


//==============================================================================
// drawQuad                                                                    =
//==============================================================================
void Renderer::drawQuad()
{
	quadVao.bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	quadVao.unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}


//==============================================================================
// unproject                                                                   =
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
// createFai                                                                   =
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
// calcPlanes                                                                  =
//==============================================================================
void Renderer::calcPlanes(const Vec2& cameraRange, Vec2& planes)
{
	float zNear = cameraRange.x();
	float zFar = cameraRange.y();

	planes.x() = zFar / (zNear - zFar);
	planes.y() = (zFar * zNear) / (zNear -zFar);
}


//==============================================================================
// calcLimitsOfNearPlane                                                       =
//==============================================================================
void Renderer::calcLimitsOfNearPlane(const PerspectiveCamera& pcam,
	Vec2& limitsOfNearPlane)
{
	limitsOfNearPlane.y() = pcam.getZNear() * tan(0.5 * pcam.getFovY());
	limitsOfNearPlane.x() = limitsOfNearPlane.y() *
		(pcam.getFovX() / pcam.getFovY());
}

#include "anki/renderer/Renderer.h"
#include "anki/util/Exception.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Scene.h"

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

	// a few sanity checks
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
		{-1.0, -1.0}, {1.0, -1.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	static const U16 quadVertIndeces[2][3] = 
		{{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	quadVertIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces),
		quadVertIndeces, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(
		&quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, 0);
	quadVao.attachElementArrayBufferVbo(&quadVertIndecesVbo);
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

	tiler.updateTiles(scene->getActiveCamera());
	ms.run();
	tiler.runMinMax(ms.getDepthFai());
	is.run();
	bs.run();
	pps.run();

	ANKI_CHECK_GL_ERROR();
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
void Renderer::drawQuadMultiple(U times)
{
	quadVao.bind();
#if ANKI_GL == ANKI_GL_DESKTOP
	const U max_times = 16;
	Array<GLsizei, max_times> count;
	Array<const GLvoid*, max_times> indices;

	for(U i = 0; i < times; i++)
	{
		count[i] = 2 * 3;
		indices[i] = nullptr;
	}

	glMultiDrawElements(
		GL_TRIANGLES, &count[0], GL_UNSIGNED_SHORT, &indices[0], times);
#else
	for(U i = 0; i < times; i++)
	{
		glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	}
#endif
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
void Renderer::createFai(U32 width, U32 height, int internalFormat,
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
	init.filteringType = Texture::TFT_NEAREST;
	init.repeat = false;
	init.anisotropyLevel = 0;

	fai.create(init);
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
Bool Renderer::doVisibilityTests(const CollisionShape& cs) const
{
	return tiler.testAll(cs, true);
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

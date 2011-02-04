#include "Renderer.h"
#include "Camera.h"
#include "RendererInitializer.h"
#include "Exception.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Renderer::Renderer():
	ms(*this),
	is(*this),
	pps(*this),
	bs(*this),
	width(640),
	height(480),
	sceneDrawer(*this)
{}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Renderer::init(const RendererInitializer& initializer)
{
	// set from the initializer
	width = initializer.width;
	height = initializer.height;

	aspectRatio = float(width)/height;
	framesNum = 0;

	// a few sanity checks
	if(width < 10 || height < 10)
	{
		throw EXCEPTION("Incorrect sizes");
	}

	// init the stages. Careful with the order!!!!!!!!!!
	ms.init(initializer);
	is.init(initializer);
	pps.init(initializer);
	bs.init(initializer);

	// quad VBOs and VAO
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
	quadPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords), quadVertCoords, GL_STATIC_DRAW);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	quadVertIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces), quadVertIndeces, GL_STATIC_DRAW);

	quadVao.create();
	quadVao.attachArrayBufferVbo(quadPositionsVbo, 0, 2, GL_FLOAT, false, 0, NULL);
	quadVao.attachElementArrayBufferVbo(quadVertIndecesVbo);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void Renderer::render(Camera& cam_)
{
	cam = &cam_;

	viewProjectionMat = cam->getProjectionMatrix() * cam->getViewMatrix();

	ms.run();
	is.run();
	pps.runPrePass();
	bs.run();
	pps.runPostPass();

	++framesNum;
}


//======================================================================================================================
// drawQuad                                                                                                            =
//======================================================================================================================
void Renderer::drawQuad()
{
	quadVao.bind();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
	quadVao.unbind();
	ON_GL_FAIL_THROW_EXCEPTION();
}


//======================================================================================================================
// unproject                                                                                                           =
//======================================================================================================================
Vec3 Renderer::unproject(const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat,
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

	Vec4 final = invPm * vec;
	final /= final.w();
	return Vec3(final);
}


//======================================================================================================================
// ortho                                                                                                               =
//======================================================================================================================
Mat4 Renderer::ortho(float left, float right, float bottom, float top, float near, float far)
{
	float difx = right-left;
	float dify = top-bottom;
	float difz = far-near;
	float tx = -(right+left) / difx;
	float ty = -(top+bottom) / dify;
	float tz = -(far+near) / difz;
	Mat4 m;

	m(0, 0) = 2.0 / difx;
	m(0, 1) = 0.0;
	m(0, 2) = 0.0;
	m(0, 3) = tx;
	m(1, 0) = 0.0;
	m(1, 1) = 2.0 / dify;
	m(1, 2) = 0.0;
	m(1, 3) = ty;
	m(2, 0) = 0.0;
	m(2, 1) = 0.0;
	m(2, 2) = -2.0 / difz;
	m(2, 3) = tz;
	m(3, 0) = 0.0;
	m(3, 1) = 0.0;
	m(3, 2) = 0.0;
	m(3, 3) = 1.0;

	return m;
}


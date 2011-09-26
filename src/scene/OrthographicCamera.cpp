#include "OrthographicCamera.h"


//==============================================================================
// setAll                                                                      =
//==============================================================================
void OrthographicCamera::setAll(float left_, float right_, float top_,
	float bottom_, float zNear_, float zFar_)
{
	left = left_;
	right = right_;
	top = top_;
	bottom = bottom_;
	zNear = zNear_;
	zFar = zFar_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


//==============================================================================
// calcLSpaceFrustumPlanes                                                     =
//==============================================================================
void OrthographicCamera::calcLSpaceFrustumPlanes()
{
	lspaceFrustumPlanes[FP_LEFT] = Plane(Vec3(1.0, 0.0, 0.0), left);
	lspaceFrustumPlanes[FP_RIGHT] = Plane(Vec3(-1.0, 0.0, 0.0), -right);
	lspaceFrustumPlanes[FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), zNear);
	lspaceFrustumPlanes[FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), -zFar);
	lspaceFrustumPlanes[FP_TOP] = Plane(Vec3(0.0, -1.0, 0.0), -top);
	lspaceFrustumPlanes[FP_BOTTOM] = Plane(Vec3(0.0, 1.0, 0.0), bottom);
}


//==============================================================================
// ortho                                                                       =
//==============================================================================
Mat4 OrthographicCamera::ortho(float left, float right, float bottom,
	float top, float near, float far)
{
	float difx = right - left;
	float dify = top - bottom;
	float difz = far - near;
	float tx = -(right + left) / difx;
	float ty = -(top + bottom) / dify;
	float tz = -(far + near) / difz;
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


//==============================================================================
// calcProjectionMatrix                                                        =
//==============================================================================
void OrthographicCamera::calcProjectionMatrix()
{
	projectionMat = ortho(left, right, bottom, top, zNear, zFar);
	invProjectionMat = projectionMat.getInverse();
}

#include "OrthographicCamera.h"


//==============================================================================
// setAll                                                                      =
//==============================================================================
void OrthographicCamera::setAll(float left_, float right_, float top_, float bottom_, float zNear_, float zFar_)
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
	lspaceFrustumPlanes[FP_LEFT] = Col::Plane(Vec3(1.0, 0.0, 0.0), left);
	lspaceFrustumPlanes[FP_RIGHT] = Col::Plane(Vec3(-1.0, 0.0, 0.0), -right);
	lspaceFrustumPlanes[FP_NEAR] = Col::Plane(Vec3(0.0, 0.0, -1.0), zNear);
	lspaceFrustumPlanes[FP_FAR] = Col::Plane(Vec3(0.0, 0.0, 1.0), -zFar);
	lspaceFrustumPlanes[FP_TOP] = Col::Plane(Vec3(0.0, -1.0, 0.0), -top);
	lspaceFrustumPlanes[FP_BOTTOM] = Col::Plane(Vec3(0.0, 1.0, 0.0), bottom);
}


//==============================================================================
// ortho                                                                       =
//==============================================================================
Mat4 OrthographicCamera::ortho(float left, float right, float bottom, float top, float near, float far)
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


//==============================================================================
// getExtremePoints                                                            =
//==============================================================================
void OrthographicCamera::getExtremePoints(Vec3* pointsArr, uint& pointsNum) const
{
	pointsArr[0] = Vec3(right, top, -zNear);
	pointsArr[1] = Vec3(left, top, -zNear);
	pointsArr[2] = Vec3(left, bottom, -zNear);
	pointsArr[3] = Vec3(right, bottom, -zNear);
	pointsArr[4] = Vec3(right, top, -zFar);
	pointsArr[5] = Vec3(left, top, -zFar);
	pointsArr[6] = Vec3(left, bottom, -zFar);
	pointsArr[7] = Vec3(right, bottom, -zFar);

	pointsNum = 8;
}


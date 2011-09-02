#include "PerspectiveCamera.h"


//==============================================================================
// setAll                                                                      =
//==============================================================================
void PerspectiveCamera::setAll(float fovx_, float fovy_, float znear_,
	float zfar_)
{
	fovX = fovx_;
	fovY = fovy_;
	zNear = znear_;
	zFar = zfar_;
	calcProjectionMatrix();
	calcLSpaceFrustumPlanes();
}


//==============================================================================
// calcProjectionMatrix                                                        =
//==============================================================================
void PerspectiveCamera::calcProjectionMatrix()
{
	float f = 1.0 / tan(fovY * 0.5); // f = cot(fovY/2)

	projectionMat(0, 0) = f * fovY / fovX; // = f/aspectRatio;
	projectionMat(0, 1) = 0.0;
	projectionMat(0, 2) = 0.0;
	projectionMat(0, 3) = 0.0;
	projectionMat(1, 0) = 0.0;
	projectionMat(1, 1) = f;
	projectionMat(1, 2) = 0.0;
	projectionMat(1, 3) = 0.0;
	projectionMat(2, 0) = 0.0;
	projectionMat(2, 1) = 0.0;
	projectionMat(2, 2) = (zFar + zNear) / ( zNear - zFar);
	projectionMat(2, 3) = (2.0 * zFar * zNear) / (zNear - zFar);
	projectionMat(3, 0) = 0.0;
	projectionMat(3, 1) = 0.0;
	projectionMat(3, 2) = -1.0;
	projectionMat(3, 3) = 0.0;

	invProjectionMat = projectionMat.getInverse();
}


//==============================================================================
// calcLSpaceFrustumPlanes                                                     =
//==============================================================================
void PerspectiveCamera::calcLSpaceFrustumPlanes()
{
	float c, s; // cos & sine

	Math::sinCos(Math::PI + fovX / 2, s, c);
	// right
	lspaceFrustumPlanes[FP_RIGHT] = Plane(Vec3(c, 0.0, s), 0.0);
	// left
	lspaceFrustumPlanes[FP_LEFT] = Plane(Vec3(-c, 0.0, s), 0.0);

	Math::sinCos((3 * Math::PI - fovY) * 0.5, s, c);
	// top
	lspaceFrustumPlanes[FP_TOP] = Plane(Vec3(0.0, s, c), 0.0);
	// bottom
	lspaceFrustumPlanes[FP_BOTTOM] = Plane(Vec3(0.0, -s, c), 0.0);

	// near
	lspaceFrustumPlanes[FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), zNear);
	// far
	lspaceFrustumPlanes[FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), -zFar);
}


//==============================================================================
// getExtremePoints                                                            =
//==============================================================================
void PerspectiveCamera::getExtremePoints(Vec3* points, uint& pointsNum) const
{
	float x = getZFar() / tan((Math::PI - getFovX()) / 2.0);
	float y = tan(getFovY() / 2.0) * getZFar();
	float z = -getZFar();

	// the actual points in local space
	points[0] = Vec3(x, y, z); // top right
	points[1] = Vec3(-x, y, z); // top left
	points[2] = Vec3(-x, -y, z); // bottom left
	points[3] = Vec3(x, -y, z); // bottom right
	points[4] = getWorldTransform().getOrigin(); // eye (already in world space)

	// transform them to the given camera's world space (exept the eye)
	for(uint i = 0; i < 4; i++)
	{
		points[i].transform(getWorldTransform());
	}

	pointsNum = 5;
}

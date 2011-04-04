#include "OrthographicCamera.h"


//======================================================================================================================
// calcLSpaceFrustumPlanes                                                                                             =
//======================================================================================================================
void OrthographicCamera::calcLSpaceFrustumPlanes()
{
	lspaceFrustumPlanes[FP_LEFT] = Plane(Vec3(1.0, 0.0, 0.0), left);
	lspaceFrustumPlanes[FP_RIGHT] = Plane(Vec3(-1.0, 0.0, 0.0), -right);
	lspaceFrustumPlanes[FP_NEAR] = Plane(Vec3(0.0, 0.0, -1.0), znear);
	lspaceFrustumPlanes[FP_FAR] = Plane(Vec3(0.0, 0.0, 1.0), -zfar);
	lspaceFrustumPlanes[FP_TOP] = Plane(Vec3(0.0, -1.0, 0.0), -top);
	lspaceFrustumPlanes[FP_BOTTOM] = Plane(Vec3(0.0, 1.0, 0.0), bottom);
}

#include <boost/foreach.hpp>
#include "Camera.h"


//==============================================================================
// lookAtPoint                                                                 =
//==============================================================================
void Camera::lookAtPoint(const Vec3& point)
{
	const Vec3& j = Vec3(0.0, 1.0, 0.0);
	Vec3 vdir = (point - getLocalTransform().getOrigin()).getNormalized();
	Vec3 vup = j - vdir * j.dot(vdir);
	Vec3 vside = vdir.cross(vup);
	getLocalTransform().getRotation().setColumns(vside, vup, -vdir);
}


//==============================================================================
// updateWSpaceFrustumPlanes                                                   =
//==============================================================================
void Camera::updateWSpaceFrustumPlanes()
{
	for(uint i = 0; i < FP_NUM; i++)
	{
		wspaceFrustumPlanes[i] =
			lspaceFrustumPlanes[i].getTransformed(getWorldTransform());
	}
}


//==============================================================================
// insideFrustum                                                               =
//==============================================================================
bool Camera::insideFrustum(const cln::CollisionShape& bvol) const
{
	BOOST_FOREACH(const cln::Plane& plane, wspaceFrustumPlanes)
	{
		if(bvol.testPlane(plane) < 0.0)
		{
			return false;
		}
	}

	return true;
}


//==============================================================================
// insideFrustum                                                               =
//==============================================================================
bool Camera::insideFrustum(const Camera& cam) const
{
	const uint MAX_EXTREME_POINTS_NUM = 10;
	Vec3 points[MAX_EXTREME_POINTS_NUM];
	uint pointsNum;

	cam.getExtremePoints(points, pointsNum);

	ASSERT(pointsNum < MAX_EXTREME_POINTS_NUM);

	// the collision code
	for(uint i = 0; i < 6; i++) // for the 6 planes
	{
		uint failed = 0;

		for(uint j = 0; j < pointsNum; j++) // for the n points
		{
			if(wspaceFrustumPlanes[i].test(points[j]) < 0.0)
			{
				++failed;
			}
		}
		if(failed == pointsNum)
		{
			// if all points are behind the plane then the cam is not in
			// frustum
			return false;
		}
	}

	return true;
}


//==============================================================================
// updateViewMatrix                                                            =
//==============================================================================
void Camera::updateViewMatrix()
{
	/*
	 * The point at which the camera looks:
	 * Vec3 viewpoint = translationLspace + z_axis;
	 * as we know the up vector, we can easily use gluLookAt:
	 * gluLookAt(translationLspace.x, translationLspace.x,
	 *           translationLspace.z, z_axis.x, z_axis.y, z_axis.z, y_axis.x,
	 *           y_axis.y, y_axis.z);
	*/

	// The view matrix is: Mview = camera.world_transform.Inverted().
	// But instead of inverting we do the following:
	Mat3 camInvertedRot = getWorldTransform().getRotation().getTransposed();
	camInvertedRot *= 1.0 / getWorldTransform().getScale();
	Vec3 camInvertedTsl = -(camInvertedRot * getWorldTransform().getOrigin());
	viewMat = Mat4(camInvertedTsl, camInvertedRot);
}


//==============================================================================
// moveUpdate                                                                  =
//==============================================================================
void Camera::moveUpdate()
{
	updateViewMatrix();
	updateWSpaceFrustumPlanes();
}


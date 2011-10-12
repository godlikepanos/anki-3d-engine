#include <boost/foreach.hpp>
#include "anki/scene/Camera.h"


namespace anki {


//==============================================================================
// Constructors & desrtuctor                                                   =
//==============================================================================

Camera::~Camera()
{}


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
bool Camera::insideFrustum(const CollisionShape& bvol) const
{
	BOOST_FOREACH(const Plane& plane, wspaceFrustumPlanes)
	{
		if(bvol.testPlane(plane) < 0.0)
		{
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


} // end namespace

#include "anki/scene/Camera.h"


namespace anki {


//==============================================================================
// Camera                                                                      =
//==============================================================================

//==============================================================================
Camera::~Camera()
{}


//==============================================================================
void Camera::lookAtPoint(const Vec3& point)
{
	const Vec3& j = Vec3(0.0, 1.0, 0.0);
	Vec3 vdir = (point - getLocalTransform().getOrigin()).getNormalized();
	Vec3 vup = j - vdir * j.dot(vdir);
	Vec3 vside = vdir.cross(vup);
	getLocalTransform().getRotation().setColumns(vside, vup, -vdir);
}


} // end namespace

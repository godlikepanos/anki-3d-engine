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


//==============================================================================
// PerspectiveCamera                                                           =
//==============================================================================

//==============================================================================
PerspectiveCamera::PerspectiveCamera(const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Camera(CT_PERSPECTIVE, name, scene, movableFlags, movParent, &frustum)
{
	Property<PerspectiveFrustum>& prop =
		addProperty("frustum", &frustum, PropertyBase::PF_READ_WRITE);
	ANKI_CONNECT(&prop, valueChanged, this, updateFrustumSlot);
}


//==============================================================================
// OrthographicCamera                                                          =
//==============================================================================

//==============================================================================
OrthographicCamera::OrthographicCamera(const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Camera(CT_ORTHOGRAPHIC, name, scene, movableFlags, movParent, &frustum)
{
	Property<OrthographicFrustum>& prop =
		addProperty("frustum", &frustum, PropertyBase::PF_READ_WRITE);
	ANKI_CONNECT(&prop, valueChanged, this, updateFrustumSlot);
}


} // end namespace

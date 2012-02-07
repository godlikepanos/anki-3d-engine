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

	Mat3 rot = getLocalTransform().getRotation();
	rot.setColumns(vside, vup, -vdir);
	setLocalRotation(rot);
}


//==============================================================================
// PerspectiveCamera                                                           =
//==============================================================================

//==============================================================================
PerspectiveCamera::PerspectiveCamera(const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Camera(CT_PERSPECTIVE, name, scene, movableFlags, movParent, &frustum)
{
	Property<PerspectiveFrustum>* prop =
		new ReadWritePointerProperty<PerspectiveFrustum>("frustum", &frustum);

	addNewProperty(prop);

	ANKI_CONNECT(prop, valueChanged, this, updateFrustumSlot);
}


//==============================================================================
// OrthographicCamera                                                          =
//==============================================================================

//==============================================================================
OrthographicCamera::OrthographicCamera(const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Camera(CT_ORTHOGRAPHIC, name, scene, movableFlags, movParent, &frustum)
{
	Property<OrthographicFrustum>* prop =
		new ReadWritePointerProperty<OrthographicFrustum>("frustum", &frustum);

	addNewProperty(prop);

	ANKI_CONNECT(prop, valueChanged, this, updateFrustumSlot);
}


} // end namespace

#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
// Camera                                                                      =
//==============================================================================

//==============================================================================
Camera::Camera(CameraType type_,
	const char* name, Scene* scene, // SceneNode
	U32 movableFlags, Movable* movParent, // Movable
	Frustum* frustum) // Spatial & Frustumable
	:	SceneNode(name, scene),
		Movable(movableFlags, movParent, *this, getSceneAllocator()),
		Spatial(frustum),
		Frustumable(frustum),
		type(type_)
{}

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
	: Camera(CT_PERSPECTIVE, name, scene, movableFlags, movParent, 
		&frustum)
{}

//==============================================================================
// OrthographicCamera                                                          =
//==============================================================================

//==============================================================================
OrthographicCamera::OrthographicCamera(const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Camera(CT_ORTHOGRAPHIC, name, scene, movableFlags, movParent, 
		&frustum)
{}

} // end namespace

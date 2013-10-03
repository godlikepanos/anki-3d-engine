#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
// Camera                                                                      =
//==============================================================================

//==============================================================================
Camera::Camera(
	const char* name, SceneGraph* scene, // SceneNode
	Frustum* frustum, // Spatial & Frustumable
	CameraType type_) 
	:	SceneNode(name, scene),
		MoveComponent(this),
		SpatialComponent(this, frustum),
		FrustumComponent(frustum),
		type(type_)
{
	sceneNodeProtected.moveC = this;
	sceneNodeProtected.spatialC = this;
	sceneNodeProtected.frustumC = this;
}

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
PerspectiveCamera::PerspectiveCamera(const char* name, SceneGraph* scene)
	: Camera(name, scene, &frustum, CT_PERSPECTIVE)
{}

//==============================================================================
// OrthographicCamera                                                          =
//==============================================================================

//==============================================================================
OrthographicCamera::OrthographicCamera(const char* name, SceneGraph* scene)
	: Camera(name, scene, &frustum, CT_ORTHOGRAPHIC)
{}

} // end namespace

#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
// Camera                                                                      =
//==============================================================================

//==============================================================================
Camera::Camera(
	const char* name, SceneGraph* scene, // SceneNode
	Frustum* frustum, // SpatialComponent & FrustumComponent
	CameraType type_) 
	: SceneNode(name, scene), type(type_)
{
	// Init components
	newComponent<MoveComponent>(this);
	newComponent<SpatialComponent>(this, frustum);
	newComponent<FrustumComponent>(this, frustum);
}

//==============================================================================
Camera::~Camera()
{}

//==============================================================================
void Camera::lookAtPoint(const Vec3& point)
{
	MoveComponent& move = getComponent<MoveComponent>();

	const Vec3& j = Vec3(0.0, 1.0, 0.0);
	Vec3 vdir = (point - move.getLocalTransform().getOrigin()).getNormalized();
	Vec3 vup = j - vdir * j.dot(vdir);
	Vec3 vside = vdir.cross(vup);

	Mat3 rot = move.getLocalTransform().getRotation();
	rot.setColumns(vside, vup, -vdir);
	move.setLocalRotation(rot);
}

//==============================================================================
void Camera::frustumUpdate()
{
	// Frustum
	FrustumComponent& fr = getComponent<FrustumComponent>();
	fr.setProjectionMatrix(fr.getFrustum().calculateProjectionMatrix());
	fr.setViewProjectionMatrix(fr.getProjectionMatrix() * fr.getViewMatrix());
	fr.markForUpdate();

	// Spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
void Camera::moveUpdate(MoveComponent& move)
{
	// Frustum
	FrustumComponent& fr = getComponent<FrustumComponent>();
	fr.setViewMatrix(Mat4(move.getWorldTransform().getInverse()));
	fr.setViewProjectionMatrix(fr.getProjectionMatrix() * fr.getViewMatrix());
	fr.setOrigin(move.getWorldTransform().getOrigin());
	fr.markForUpdate();
	fr.getFrustum().setTransform(move.getWorldTransform());

	// Spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.setOrigin(move.getWorldTransform().getOrigin());
	sp.markForUpdate();
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

} // end namespace anki

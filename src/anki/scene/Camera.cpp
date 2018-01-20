// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Camera.h>

namespace anki
{

/// Feedback component.
class CameraMoveFeedbackComponent : public SceneComponent
{
public:
	CameraMoveFeedbackComponent(Camera* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == getGlobalTimestamp())
		{
			Camera& cam = static_cast<Camera&>(node);
			cam.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

/// Feedback component.
class CameraFrustumFeedbackComponent : public SceneComponent
{
public:
	CameraFrustumFeedbackComponent(Camera* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		FrustumComponent& fr = node.getComponent<FrustumComponent>();
		if(fr.getTimestamp() == getGlobalTimestamp())
		{
			Camera& cam = static_cast<Camera&>(node);
			cam.onFrustumComponentUpdate(fr);
		}

		return Error::NONE;
	}
};

Camera::Camera(SceneGraph* scene, Type type, CString name)
	: SceneNode(scene, name)
	, m_type(type)
{
}

Error Camera::init(Frustum* frustum)
{
	// Move component
	newComponent<MoveComponent>(this);

	// Feedback component
	newComponent<CameraMoveFeedbackComponent>(this);

	// Frustum component
	FrustumComponent* frc = newComponent<FrustumComponent>(this, frustum);
	frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS
		| FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
		| FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS
		| FrustumComponentVisibilityTestFlag::REFLECTION_PROBES
		| FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES
		| FrustumComponentVisibilityTestFlag::OCCLUDERS
		| FrustumComponentVisibilityTestFlag::DECALS
		| FrustumComponentVisibilityTestFlag::EARLY_Z);

	// Feedback component #2
	newComponent<CameraFrustumFeedbackComponent>(this);

	// Spatial component
	newComponent<SpatialComponent>(this, frustum);

	return Error::NONE;
}

Camera::~Camera()
{
}

void Camera::lookAtPoint(const Vec3& point)
{
	MoveComponent& move = getComponent<MoveComponent>();

	Vec4 j = Vec4(0.0, 1.0, 0.0, 0.0);
	Vec4 vdir = (point.xyz0() - move.getLocalTransform().getOrigin()).getNormalized();
	Vec4 vup = j - vdir * j.dot(vdir);
	Vec4 vside = vdir.cross(vup);

	Mat3x4 rot = move.getLocalTransform().getRotation();
	rot.setColumns(vside.xyz(), vup.xyz(), (-vdir).xyz());
	move.setLocalRotation(rot);
}

void Camera::onFrustumComponentUpdate(FrustumComponent& fr)
{
	// Spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

void Camera::onMoveComponentUpdate(MoveComponent& move)
{
	// Frustum
	FrustumComponent& fr = getComponent<FrustumComponent>();
	fr.markTransformForUpdate();
	fr.getFrustum().resetTransform(move.getWorldTransform());

	// Spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
}

PerspectiveCamera::PerspectiveCamera(SceneGraph* scene, CString name)
	: Camera(scene, Type::PERSPECTIVE, name)
{
}

PerspectiveCamera::~PerspectiveCamera()
{
}

void PerspectiveCamera::setAll(F32 fovX, F32 fovY, F32 near, F32 far)
{
	m_frustum.setAll(fovX, fovY, near, far);
	getComponent<FrustumComponent>().markShapeForUpdate();
}

OrthographicCamera::OrthographicCamera(SceneGraph* scene, CString name)
	: Camera(scene, Type::ORTHOGRAPHIC, name)
{
}

OrthographicCamera::~OrthographicCamera()
{
}

} // end namespace anki

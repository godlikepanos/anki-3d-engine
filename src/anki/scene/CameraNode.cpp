// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/CameraNode.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/MoveComponent.h>

namespace anki
{

/// Feedback component.
class CameraMoveFeedbackComponent : public SceneComponent
{
public:
	CameraMoveFeedbackComponent(CameraNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == getGlobalTimestamp())
		{
			CameraNode& cam = static_cast<CameraNode&>(node);
			cam.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

/// Feedback component.
class CameraFrustumFeedbackComponent : public SceneComponent
{
public:
	CameraFrustumFeedbackComponent(CameraNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second, Second, Bool& updated) override
	{
		updated = false;

		FrustumComponent& fr = node.getComponent<FrustumComponent>();
		if(fr.getTimestamp() == getGlobalTimestamp())
		{
			CameraNode& cam = static_cast<CameraNode&>(node);
			cam.onFrustumComponentUpdate(fr);
		}

		return Error::NONE;
	}
};

CameraNode::CameraNode(SceneGraph* scene, Type type, CString name)
	: SceneNode(scene, name)
	, m_type(type)
{
}

Error CameraNode::init(Frustum* frustum)
{
	// Move component
	newComponent<MoveComponent>(this);

	// Feedback component
	newComponent<CameraMoveFeedbackComponent>(this);

	// Frustum component
	FrustumComponent* frc = newComponent<FrustumComponent>(this, frustum);
	frc->setEnabledVisibilityTests(
		FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
		| FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS
		| FrustumComponentVisibilityTestFlag::REFLECTION_PROBES | FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES
		| FrustumComponentVisibilityTestFlag::OCCLUDERS | FrustumComponentVisibilityTestFlag::DECALS
		| FrustumComponentVisibilityTestFlag::EARLY_Z);

	// Feedback component #2
	newComponent<CameraFrustumFeedbackComponent>(this);

	// Spatial component
	newComponent<SpatialComponent>(this, frustum);

	return Error::NONE;
}

CameraNode::~CameraNode()
{
}

void CameraNode::lookAtPoint(const Vec3& point)
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

void CameraNode::onFrustumComponentUpdate(FrustumComponent& fr)
{
	// Spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

void CameraNode::onMoveComponentUpdate(MoveComponent& move)
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

PerspectiveCameraNode::PerspectiveCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, Type::PERSPECTIVE, name)
{
}

PerspectiveCameraNode::~PerspectiveCameraNode()
{
}

void PerspectiveCameraNode::setAll(F32 fovX, F32 fovY, F32 near, F32 far)
{
	m_frustum.setAll(fovX, fovY, near, far);
	getComponent<FrustumComponent>().markShapeForUpdate();
}

OrthographicCameraNode::OrthographicCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, Type::ORTHOGRAPHIC, name)
{
}

OrthographicCameraNode::~OrthographicCameraNode()
{
}

} // end namespace anki

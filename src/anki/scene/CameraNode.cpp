// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/CameraNode.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>

namespace anki
{

/// Feedback component.
class CameraNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			CameraNode& cam = static_cast<CameraNode&>(node);
			cam.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

/// Feedback component.
class CameraNode::FrustumFeedbackComponent : public SceneComponent
{
public:
	FrustumFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		FrustumComponent& fr = node.getComponent<FrustumComponent>();
		if(fr.getTimestamp() == node.getGlobalTimestamp())
		{
			CameraNode& cam = static_cast<CameraNode&>(node);
			cam.onFrustumComponentUpdate(fr);
		}

		return Error::NONE;
	}
};

CameraNode::CameraNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

CameraNode::~CameraNode()
{
}

Error CameraNode::init(FrustumType frustumType)
{
	// Move component
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// Frustum component
	FrustumComponent* frc = newComponent<FrustumComponent>(this, frustumType);
	frc->setEnabledVisibilityTests(
		FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
		| FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS
		| FrustumComponentVisibilityTestFlag::REFLECTION_PROBES | FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES
		| FrustumComponentVisibilityTestFlag::OCCLUDERS | FrustumComponentVisibilityTestFlag::DECALS
		| FrustumComponentVisibilityTestFlag::FOG_DENSITY_COMPONENTS | FrustumComponentVisibilityTestFlag::EARLY_Z
		| FrustumComponentVisibilityTestFlag::ALL_SHADOWS_ENABLED);

	// Feedback component #2
	newComponent<FrustumFeedbackComponent>();

	// Spatial component
	SpatialComponent* spatialc;
	if(frustumType == FrustumType::PERSPECTIVE)
	{
		spatialc = newComponent<SpatialComponent>(this, &frc->getPerspectiveBoundingShape());
	}
	else
	{
		spatialc = newComponent<SpatialComponent>(this, &frc->getOrthographicBoundingShape());
	}

	spatialc->setUpdateOctreeBounds(false);

	return Error::NONE;
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
	fr.setTransform(move.getWorldTransform());

	// Spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	sp.markForUpdate();
}

PerspectiveCameraNode::PerspectiveCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, name)
{
}

PerspectiveCameraNode::~PerspectiveCameraNode()
{
}

OrthographicCameraNode::OrthographicCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, name)
{
}

OrthographicCameraNode::~OrthographicCameraNode()
{
}

} // end namespace anki

// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/CameraNode.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneGraph.h>

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

		MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
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

		FrustumComponent& fr = node.getFirstComponentOfType<FrustumComponent>();
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
	const FrustumComponentVisibilityTestFlag visibilityFlags =
		FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
		| FrustumComponentVisibilityTestFlag::LENS_FLARE_COMPONENTS
		| FrustumComponentVisibilityTestFlag::REFLECTION_PROBES | FrustumComponentVisibilityTestFlag::REFLECTION_PROXIES
		| FrustumComponentVisibilityTestFlag::OCCLUDERS | FrustumComponentVisibilityTestFlag::DECALS
		| FrustumComponentVisibilityTestFlag::FOG_DENSITY_COMPONENTS
		| FrustumComponentVisibilityTestFlag::GLOBAL_ILLUMINATION_PROBES | FrustumComponentVisibilityTestFlag::EARLY_Z
		| FrustumComponentVisibilityTestFlag::ALL_SHADOWS_ENABLED
		| FrustumComponentVisibilityTestFlag::GENERIC_COMPUTE_JOB_COMPONENTS;
	frc->setEnabledVisibilityTests(visibilityFlags);
	frc->setLodDistance(0, getSceneGraph().getConfig().m_maxLodDistances[0]);
	frc->setLodDistance(1, getSceneGraph().getConfig().m_maxLodDistances[1]);
	frc->setLodDistance(2, getSceneGraph().getConfig().m_maxLodDistances[2]);

	// Extended frustum for RT
	if(getSceneGraph().getConfig().m_rayTracedShadows)
	{
		FrustumComponent* rtFrustumComponent = newComponent<FrustumComponent>(this, FrustumType::ORTHOGRAPHIC);
		rtFrustumComponent->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::RAY_TRACING_SHADOWS);

		const F32 dist = getSceneGraph().getConfig().m_rayTracingExtendedFrustumDistance;

		rtFrustumComponent->setOrthographic(0.1f, dist * 2.0f, dist, -dist, dist, -dist);
		rtFrustumComponent->setLodDistance(0, getSceneGraph().getConfig().m_maxLodDistances[0]);
		rtFrustumComponent->setLodDistance(1, getSceneGraph().getConfig().m_maxLodDistances[1]);
		rtFrustumComponent->setLodDistance(2, getSceneGraph().getConfig().m_maxLodDistances[2]);
	}

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
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.markForUpdate();
}

void CameraNode::onMoveComponentUpdate(MoveComponent& move)
{
	const Transform& worldTransform = move.getWorldTransform();

	// Frustum
	U count = 0;
	const Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fc) {
		if(count == 0)
		{
			fc.setTransform(worldTransform);
		}
		else
		{
			// Extended RT frustum, re-align it so the frustum is positioned at the center of the camera eye
			ANKI_ASSERT(fc.getFrustumType() == FrustumType::ORTHOGRAPHIC);
			const F32 far = fc.getFar();
			Transform extendedFrustumTransform = Transform::getIdentity();
			Vec3 newOrigin = worldTransform.getOrigin().xyz();
			newOrigin.z() += far / 2.0f;
			extendedFrustumTransform.setOrigin(newOrigin.xyz0());
			fc.setTransform(extendedFrustumTransform);
		}

		++count;
		return Error::NONE;
	});
	(void)err;

	// Spatial
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
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

// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
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
	ANKI_SCENE_COMPONENT(CameraNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
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

ANKI_SCENE_COMPONENT_STATICS(CameraNode::MoveFeedbackComponent)

CameraNode::CameraNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

CameraNode::~CameraNode()
{
}

void CameraNode::initCommon(FrustumType frustumType)
{
	// Move component
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// Frustum component
	FrustumComponent* frc = newComponent<FrustumComponent>();
	frc->setFrustumType(frustumType);
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
		FrustumComponent* rtFrustumComponent = newComponent<FrustumComponent>();
		rtFrustumComponent->setFrustumType(FrustumType::ORTHOGRAPHIC);
		rtFrustumComponent->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::RAY_TRACING_SHADOWS);

		const F32 dist = getSceneGraph().getConfig().m_rayTracingExtendedFrustumDistance;

		rtFrustumComponent->setOrthographic(0.1f, dist * 2.0f, dist, -dist, dist, -dist);
		rtFrustumComponent->setLodDistance(0, getSceneGraph().getConfig().m_maxLodDistances[0]);
		rtFrustumComponent->setLodDistance(1, getSceneGraph().getConfig().m_maxLodDistances[1]);
		rtFrustumComponent->setLodDistance(2, getSceneGraph().getConfig().m_maxLodDistances[2]);
	}
}

void CameraNode::onMoveComponentUpdate(MoveComponent& move)
{
	const Transform& worldTransform = move.getWorldTransform();

	// Frustum
	U count = 0;
	const Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fc) {
		if(count == 0)
		{
			fc.setWorldTransform(worldTransform);
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
			fc.setWorldTransform(extendedFrustumTransform);
		}

		++count;
		return Error::NONE;
	});
	(void)err;
}

PerspectiveCameraNode::PerspectiveCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, name)
{
	initCommon(FrustumType::PERSPECTIVE);
}

PerspectiveCameraNode::~PerspectiveCameraNode()
{
}

OrthographicCameraNode::OrthographicCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, name)
{
	initCommon(FrustumType::ORTHOGRAPHIC);
}

OrthographicCameraNode::~OrthographicCameraNode()
{
}

} // end namespace anki

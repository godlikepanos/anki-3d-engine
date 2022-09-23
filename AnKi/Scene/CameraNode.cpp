// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/CameraNode.h>
#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

/// Feedback component.
class CameraNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(CameraNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = info.m_node->getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			CameraNode& cam = static_cast<CameraNode&>(*info.m_node);
			cam.onMoveComponentUpdate(move);
		}

		return Error::kNone;
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
	frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kAll
								   ^ FrustumComponentVisibilityTestFlag::kAllRayTracing);
	frc->setLodDistance(0, getConfig().getLod0MaxDistance());
	frc->setLodDistance(1, getConfig().getLod1MaxDistance());
	frc->setShadowCascadeCount(getConfig().getSceneShadowCascadeCount());

	// Extended frustum for RT
	if(getSceneGraph().getGrManager().getDeviceCapabilities().m_rayTracingEnabled
	   && getConfig().getSceneRayTracedShadows())
	{
		FrustumComponent* rtFrustumComponent = newComponent<FrustumComponent>();
		rtFrustumComponent->setFrustumType(FrustumType::kOrthographic);
		rtFrustumComponent->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kRayTracingShadows);

		const F32 dist = getConfig().getSceneRayTracingExtendedFrustumDistance();

		rtFrustumComponent->setOrthographic(0.1f, dist * 2.0f, dist, -dist, dist, -dist);
		rtFrustumComponent->setLodDistance(0, getConfig().getLod0MaxDistance());
		rtFrustumComponent->setLodDistance(1, getConfig().getLod1MaxDistance());
	}
}

void CameraNode::onMoveComponentUpdate(MoveComponent& move)
{
	const Transform& worldTransform = move.getWorldTransform();

	// Frustum
	U count = 0;
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fc) {
		if(count == 0)
		{
			fc.setWorldTransform(worldTransform);
		}
		else
		{
			// Extended RT frustum, re-align it so the frustum is positioned at the center of the camera eye
			ANKI_ASSERT(fc.getFrustumType() == FrustumType::kOrthographic);
			const F32 far = fc.getFar();
			Transform extendedFrustumTransform = Transform::getIdentity();
			Vec3 newOrigin = worldTransform.getOrigin().xyz();
			newOrigin.z() += far / 2.0f;
			extendedFrustumTransform.setOrigin(newOrigin.xyz0());
			fc.setWorldTransform(extendedFrustumTransform);
		}

		++count;
	});
}

PerspectiveCameraNode::PerspectiveCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, name)
{
	initCommon(FrustumType::kPerspective);
}

PerspectiveCameraNode::~PerspectiveCameraNode()
{
}

OrthographicCameraNode::OrthographicCameraNode(SceneGraph* scene, CString name)
	: CameraNode(scene, name)
{
	initCommon(FrustumType::kOrthographic);
}

OrthographicCameraNode::~OrthographicCameraNode()
{
}

} // end namespace anki

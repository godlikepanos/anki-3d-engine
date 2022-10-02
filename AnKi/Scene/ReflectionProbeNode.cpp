// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/ReflectionProbeNode.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

/// Feedback component
class ReflectionProbeNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ReflectionProbeNode::MoveFeedbackComponent)

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
			// Move updated
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(*info.m_node);
			dnode.onMoveUpdate(move);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeNode::MoveFeedbackComponent)

/// Feedback component
class ReflectionProbeNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ReflectionProbeNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		ReflectionProbeComponent& reflc = info.m_node->getFirstComponentOfType<ReflectionProbeComponent>();
		if(reflc.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(*info.m_node);
			dnode.onShapeUpdate(reflc);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeNode::ShapeFeedbackComponent)

ReflectionProbeNode::ReflectionProbeNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Move component first
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// The frustum components
	const F32 ang = toRad(90.0f);

	Mat3 rot;

	rot = Mat3(Euler(0.0f, -kPi / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi));
	m_frustumTransforms[0].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, kPi / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi));
	m_frustumTransforms[1].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(kPi / 2.0f, 0.0f, 0.0f));
	m_frustumTransforms[2].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(-kPi / 2.0f, 0.0f, 0.0f));
	m_frustumTransforms[3].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, kPi, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi));
	m_frustumTransforms[4].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, 0.0f, kPi));
	m_frustumTransforms[5].setRotation(Mat3x4(Vec3(0.0f), rot));

	for(U i = 0; i < 6; ++i)
	{
		m_frustumTransforms[i].setOrigin(Vec4(0.0f));
		m_frustumTransforms[i].setScale(1.0f);

		FrustumComponent* frc = newComponent<FrustumComponent>();
		frc->setFrustumType(FrustumType::kPerspective);
		frc->setPerspective(kClusterObjectFrustumNearPlane, 10.0f, ang, ang);
		frc->setWorldTransform(m_frustumTransforms[i]);
		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kNone);
		frc->setEffectiveShadowDistance(getConfig().getSceneReflectionProbeShadowEffectiveDistance());
		frc->setShadowCascadeCount(1);
	}

	// Reflection probe comp
	newComponent<ReflectionProbeComponent>();

	// Feedback
	newComponent<ShapeFeedbackComponent>();

	// Spatial component
	SpatialComponent* spatialc = newComponent<SpatialComponent>();
	spatialc->setUpdateOctreeBounds(false);
}

ReflectionProbeNode::~ReflectionProbeNode()
{
}

void ReflectionProbeNode::onMoveUpdate(MoveComponent& move)
{
	// Update frustum components
	U count = 0;
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
		Transform trf = m_frustumTransforms[count];
		trf.setOrigin(move.getWorldTransform().getOrigin());

		frc.setWorldTransform(trf);
		++count;
	});

	ANKI_ASSERT(count == 6);

	// Update the spatial comp
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());

	// Update the refl comp
	ReflectionProbeComponent& reflc = getFirstComponentOfType<ReflectionProbeComponent>();
	reflc.setWorldPosition(move.getWorldTransform().getOrigin().xyz());
}

void ReflectionProbeNode::onShapeUpdate(ReflectionProbeComponent& reflc)
{
	const Vec3 halfProbeSize = reflc.getBoxVolumeSize() / 2.0f;
	F32 effectiveDistance = max(halfProbeSize.x(), halfProbeSize.y());
	effectiveDistance = max(effectiveDistance, halfProbeSize.z());
	effectiveDistance = max(effectiveDistance, getConfig().getSceneReflectionProbeEffectiveDistance());

	// Update frustum components
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
		frc.setFar(effectiveDistance);
	});

	// Update the spatial comp
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setAabbWorldSpace(reflc.getAabbWorldSpace());
}

Error ReflectionProbeNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const ReflectionProbeComponent& reflc = getFirstComponentOfType<ReflectionProbeComponent>();

	constexpr FrustumComponentVisibilityTestFlag frustumTestFlags =
		FrustumComponentVisibilityTestFlag::kRenderComponents | FrustumComponentVisibilityTestFlag::kLights;

	const FrustumComponentVisibilityTestFlag testFlags =
		reflc.getMarkedForRendering() ? frustumTestFlags : FrustumComponentVisibilityTestFlag::kNone;

	iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) {
		frc.setEnabledVisibilityTests(testFlags);
	});

	return Error::kNone;
}

} // end namespace anki

// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProbeNode.h>
#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/renderer/LightShading.h>
#include <anki/shaders/include/ClusteredShadingTypes.h>

namespace anki
{

const FrustumComponentVisibilityTestFlag FRUSTUM_TEST_FLAGS =
	FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS
	| FrustumComponentVisibilityTestFlag::DIRECTIONAL_LIGHT_SHADOWS_1_CASCADE;

/// Feedback component
class ReflectionProbeNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ReflectionProbeNode::MoveFeedbackComponent)

public:
	MoveFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(node);
			dnode.onMoveUpdate(move);
		}

		return Error::NONE;
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

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		ReflectionProbeComponent& reflc = node.getFirstComponentOfType<ReflectionProbeComponent>();
		if(reflc.getTimestamp() == node.getGlobalTimestamp())
		{
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(node);
			dnode.onShapeUpdate(reflc);
		}

		return Error::NONE;
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

	rot = Mat3(Euler(0.0f, -PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_frustumTransforms[0].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_frustumTransforms[1].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(PI / 2.0f, 0.0f, 0.0f));
	m_frustumTransforms[2].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(-PI / 2.0f, 0.0f, 0.0f));
	m_frustumTransforms[3].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, PI, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_frustumTransforms[4].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, 0.0f, PI));
	m_frustumTransforms[5].setRotation(Mat3x4(Vec3(0.0f), rot));

	for(U i = 0; i < 6; ++i)
	{
		m_frustumTransforms[i].setOrigin(Vec4(0.0f));
		m_frustumTransforms[i].setScale(1.0f);

		FrustumComponent* frc = newComponent<FrustumComponent>();
		frc->setFrustumType(FrustumType::PERSPECTIVE);
		frc->setPerspective(LIGHT_FRUSTUM_NEAR_PLANE, 10.0f, ang, ang);
		frc->setWorldTransform(m_frustumTransforms[i]);
		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
		frc->setEffectiveShadowDistance(getSceneGraph().getConfig().m_reflectionProbeShadowEffectiveDistance);
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
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		Transform trf = m_frustumTransforms[count];
		trf.setOrigin(move.getWorldTransform().getOrigin());

		frc.setWorldTransform(trf);
		++count;

		return Error::NONE;
	});

	ANKI_ASSERT(count == 6);
	(void)err;

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
	effectiveDistance = max(effectiveDistance, getSceneGraph().getConfig().m_reflectionProbeEffectiveDistance);

	// Update frustum components
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		frc.setFar(effectiveDistance);
		return Error::NONE;
	});
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setAabbWorldSpace(reflc.getAabbWorldSpace());
}

Error ReflectionProbeNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const ReflectionProbeComponent& reflc = getFirstComponentOfType<ReflectionProbeComponent>();

	const FrustumComponentVisibilityTestFlag testFlags =
		reflc.getMarkedForRendering() ? FRUSTUM_TEST_FLAGS : FrustumComponentVisibilityTestFlag::NONE;

	const Error err = iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) -> Error {
		frc.setEnabledVisibilityTests(testFlags);
		return Error::NONE;
	});
	(void)err;

	return Error::NONE;
}

} // end namespace anki

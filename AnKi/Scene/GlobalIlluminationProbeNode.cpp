// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/GlobalIlluminationProbeNode.h>
#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

/// Feedback component
class GlobalIlluminationProbeNode::MoveFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GlobalIlluminationProbeNode::MoveFeedbackComponent)

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
			GlobalIlluminationProbeNode& dnode = static_cast<GlobalIlluminationProbeNode&>(*info.m_node);
			dnode.onMoveUpdate(move);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GlobalIlluminationProbeNode::MoveFeedbackComponent)

/// Feedback component
class GlobalIlluminationProbeNode::ShapeFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(GlobalIlluminationProbeNode::ShapeFeedbackComponent)

public:
	ShapeFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(),
						 false // Not feedback component. Can't be skipped because of getMarkedForRendering()
		)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		GlobalIlluminationProbeComponent& probec =
			info.m_node->getFirstComponentOfType<GlobalIlluminationProbeComponent>();
		if(probec.getTimestamp() == info.m_node->getGlobalTimestamp() || probec.getMarkedForRendering())
		{
			// Move updated
			GlobalIlluminationProbeNode& dnode = static_cast<GlobalIlluminationProbeNode&>(*info.m_node);
			dnode.onShapeUpdateOrProbeNeedsRendering();
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(GlobalIlluminationProbeNode::ShapeFeedbackComponent)

GlobalIlluminationProbeNode::GlobalIlluminationProbeNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	// Move component first
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// GI probe comp
	newComponent<GlobalIlluminationProbeComponent>();

	// Second feedback component
	newComponent<ShapeFeedbackComponent>();

	// The frustum components
	constexpr F32 ang = toRad(90.0f);
	const F32 zNear = kClusterObjectFrustumNearPlane;

	Mat3 rot;
	rot = Mat3(Euler(0.0f, -kPi / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi));
	m_cubeFaceTransforms[0].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, kPi / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi));
	m_cubeFaceTransforms[1].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(kPi / 2.0f, 0.0f, 0.0f));
	m_cubeFaceTransforms[2].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(-kPi / 2.0f, 0.0f, 0.0f));
	m_cubeFaceTransforms[3].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, kPi, 0.0f)) * Mat3(Euler(0.0f, 0.0f, kPi));
	m_cubeFaceTransforms[4].setRotation(Mat3x4(Vec3(0.0f), rot));
	rot = Mat3(Euler(0.0f, 0.0f, kPi));
	m_cubeFaceTransforms[5].setRotation(Mat3x4(Vec3(0.0f), rot));

	for(U i = 0; i < 6; ++i)
	{
		m_cubeFaceTransforms[i].setOrigin(Vec4(0.0f));
		m_cubeFaceTransforms[i].setScale(1.0f);

		FrustumComponent* frc = newComponent<FrustumComponent>();
		frc->setFrustumType(FrustumType::kPerspective);
		const F32 tempEffectiveDistance = 1.0f;
		frc->setPerspective(zNear, tempEffectiveDistance, ang, ang);
		frc->setWorldTransform(m_cubeFaceTransforms[i]);
		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kNone);
		frc->setEffectiveShadowDistance(getConfig().getSceneReflectionProbeShadowEffectiveDistance());
		frc->setShadowCascadeCount(1);
	}

	// Spatial component
	SpatialComponent* spatialc = newComponent<SpatialComponent>();
	spatialc->setUpdateOctreeBounds(false);
}

GlobalIlluminationProbeNode::~GlobalIlluminationProbeNode()
{
}

void GlobalIlluminationProbeNode::onMoveUpdate(MoveComponent& move)
{
	GlobalIlluminationProbeComponent& gic = getFirstComponentOfType<GlobalIlluminationProbeComponent>();
	gic.setWorldPosition(move.getWorldTransform().getOrigin().xyz());

	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());
}

void GlobalIlluminationProbeNode::onShapeUpdateOrProbeNeedsRendering()
{
	GlobalIlluminationProbeComponent& gic = getFirstComponentOfType<GlobalIlluminationProbeComponent>();

	// Update the frustum component if the shape needs rendering
	if(gic.getMarkedForRendering())
	{
		// Compute effective distance
		F32 effectiveDistance = max(gic.getBoxVolumeSize().x(), gic.getBoxVolumeSize().y());
		effectiveDistance = max(effectiveDistance, gic.getBoxVolumeSize().z());
		effectiveDistance = max(effectiveDistance, getConfig().getSceneReflectionProbeEffectiveDistance());

		// Update frustum components
		U count = 0;
		iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
			Transform trf = m_cubeFaceTransforms[count];
			trf.setOrigin(gic.getRenderPosition().xyz0());

			frc.setWorldTransform(trf);
			frc.setFar(effectiveDistance);
			++count;
		});

		ANKI_ASSERT(count == 6);
	}

	// Update the spatial comp
	const Bool shapeWasUpdated = gic.getTimestamp() == getGlobalTimestamp();
	if(shapeWasUpdated)
	{
		// Update only when the shape was actually update

		SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
		sp.setAabbWorldSpace(gic.getAabbWorldSpace());
	}
}

Error GlobalIlluminationProbeNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const GlobalIlluminationProbeComponent& gic = getFirstComponentOfType<GlobalIlluminationProbeComponent>();

	constexpr FrustumComponentVisibilityTestFlag kFrustumFlags =
		FrustumComponentVisibilityTestFlag::kRenderComponents | FrustumComponentVisibilityTestFlag::kLights;

	const FrustumComponentVisibilityTestFlag testFlags =
		(gic.getMarkedForRendering()) ? kFrustumFlags : FrustumComponentVisibilityTestFlag::kNone;

	iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) {
		frc.setEnabledVisibilityTests(testFlags);
	});

	return Error::kNone;
}

} // end namespace anki

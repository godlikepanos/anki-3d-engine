// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProbeNode.h>
#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SceneGraph.h>
#include <anki/renderer/LightShading.h>
#include <shaders/glsl_cpp_common/ClusteredShading.h>

namespace anki
{

const FrustumComponentVisibilityTestFlag FRUSTUM_TEST_FLAGS =
	FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS;

/// Feedback component
class ReflectionProbeNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProbeNode& dnode = static_cast<ReflectionProbeNode&>(node);
			dnode.onMoveUpdate(move);
		}

		return Error::NONE;
	}
};

ReflectionProbeNode::~ReflectionProbeNode()
{
}

Error ReflectionProbeNode::init(const Vec4& aabbMinLSpace, const Vec4& aabbMaxLSpace)
{
	// Compute effective distance
	F32 effectiveDistance = aabbMaxLSpace.x() - aabbMinLSpace.x();
	effectiveDistance = max(effectiveDistance, aabbMaxLSpace.y() - aabbMinLSpace.y());
	effectiveDistance = max(effectiveDistance, aabbMaxLSpace.z() - aabbMinLSpace.z());
	effectiveDistance = max(effectiveDistance, EFFECTIVE_DISTANCE);

	// Move component first
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MoveFeedbackComponent>();

	// The frustum components
	const F32 ang = toRad(90.0f);
	const F32 zNear = LIGHT_FRUSTUM_NEAR_PLANE;

	Mat3 rot;

	rot = Mat3(Euler(0.0f, -PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[0].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(0.0f, PI / 2.0f, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[1].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(PI / 2.0f, 0.0f, 0.0f));
	m_cubeSides[2].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(-PI / 2.0f, 0.0f, 0.0f));
	m_cubeSides[3].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(0.0f, PI, 0.0f)) * Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[4].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(0.0f, 0.0f, PI));
	m_cubeSides[5].m_localTrf.setRotation(Mat3x4(rot));

	for(U i = 0; i < 6; ++i)
	{
		m_cubeSides[i].m_localTrf.setOrigin(Vec4(0.0f));
		m_cubeSides[i].m_localTrf.setScale(1.0f);

		m_cubeSides[i].m_frustum.setAll(ang, ang, zNear, effectiveDistance);
		m_cubeSides[i].m_frustum.resetTransform(m_cubeSides[i].m_localTrf);

		FrustumComponent* frc = newComponent<FrustumComponent>(this, &m_cubeSides[i].m_frustum);

		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
	}

	// Spatial component
	m_aabbMinLSpace = aabbMinLSpace.xyz();
	m_aabbMaxLSpace = aabbMaxLSpace.xyz();
	m_spatialAabb.setMin(aabbMinLSpace);
	m_spatialAabb.setMax(aabbMaxLSpace);
	newComponent<SpatialComponent>(this, &m_spatialAabb);

	// Reflection probe comp
	ReflectionProbeComponent* reflc = newComponent<ReflectionProbeComponent>(getSceneGraph().getNewUuid());
	reflc->setPosition(Vec4(0.0f));
	reflc->setBoundingBox(aabbMinLSpace, aabbMaxLSpace);

	return Error::NONE;
}

void ReflectionProbeNode::onMoveUpdate(MoveComponent& move)
{
	// Update frustum components
	U count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		Transform trf = m_cubeSides[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		frc.getFrustum().resetTransform(trf);
		frc.markTransformForUpdate();
		++count;

		return Error::NONE;
	});

	ANKI_ASSERT(count == 6);
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	const Vec3 aabbMinWSpace = m_aabbMinLSpace + move.getWorldTransform().getOrigin().xyz();
	const Vec3 aabbMaxWSpace = m_aabbMaxLSpace + move.getWorldTransform().getOrigin().xyz();
	m_spatialAabb.setMin(aabbMinWSpace);
	m_spatialAabb.setMax(aabbMaxWSpace);

	// Update the refl comp
	ReflectionProbeComponent& reflc = getComponent<ReflectionProbeComponent>();
	reflc.setPosition(move.getWorldTransform().getOrigin());
	reflc.setBoundingBox(aabbMinWSpace.xyz0(), aabbMaxWSpace.xyz0());
}

Error ReflectionProbeNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const ReflectionProbeComponent& reflc = getComponent<ReflectionProbeComponent>();

	FrustumComponentVisibilityTestFlag testFlags =
		reflc.getMarkedForRendering() ? FRUSTUM_TEST_FLAGS : FrustumComponentVisibilityTestFlag::NONE;

	Error err = iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) -> Error {
		frc.setEnabledVisibilityTests(testFlags);
		return Error::NONE;
	});
	(void)err;

	return Error::NONE;
}

} // end namespace anki

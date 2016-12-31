// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProbe.h>
#include <anki/scene/ReflectionProbeComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SceneGraph.h>
#include <anki/renderer/Is.h>

namespace anki
{

const FrustumComponentVisibilityTestFlag FRUSTUM_TEST_FLAGS =
	FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS | FrustumComponentVisibilityTestFlag::LIGHT_COMPONENTS;

/// Feedback component
class ReflectionProbeMoveFeedbackComponent : public SceneComponent
{
public:
	ReflectionProbeMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProbe& dnode = static_cast<ReflectionProbe&>(node);
			dnode.onMoveUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

ReflectionProbe::~ReflectionProbe()
{
}

Error ReflectionProbe::init(F32 radius)
{
	SceneComponent* comp;

	// Move component first
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<ReflectionProbeMoveFeedbackComponent>(this);
	addComponent(comp, true);

	// The frustum components
	const F32 ang = toRad(90.0);
	const F32 zNear = FRUSTUM_NEAR_PLANE;

	Mat3 rot;

	rot = Mat3(Euler(0.0, -PI / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
	m_cubeSides[0].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(0.0, PI / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
	m_cubeSides[1].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(PI / 2.0, 0.0, 0.0));
	m_cubeSides[2].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(-PI / 2.0, 0.0, 0.0));
	m_cubeSides[3].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(0.0, PI, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
	m_cubeSides[4].m_localTrf.setRotation(Mat3x4(rot));
	rot = Mat3(Euler(0.0, 0.0, PI));
	m_cubeSides[5].m_localTrf.setRotation(Mat3x4(rot));

	for(U i = 0; i < 6; ++i)
	{
		m_cubeSides[i].m_localTrf.setOrigin(Vec4(0.0));
		m_cubeSides[i].m_localTrf.setScale(1.0);

		m_cubeSides[i].m_frustum.setAll(ang, ang, zNear, EFFECTIVE_DISTANCE);
		m_cubeSides[i].m_frustum.resetTransform(m_cubeSides[i].m_localTrf);

		FrustumComponent* frc = getSceneAllocator().newInstance<FrustumComponent>(this, &m_cubeSides[i].m_frustum);

		frc->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);

		addComponent(frc, true);
	}

	// Spatial component
	m_spatialSphere.setCenter(Vec4(0.0));
	m_spatialSphere.setRadius(radius);
	comp = getSceneAllocator().newInstance<SpatialComponent>(this, &m_spatialSphere);
	addComponent(comp, true);

	// Reflection probe comp
	ReflectionProbeComponent* reflc = getSceneAllocator().newInstance<ReflectionProbeComponent>(this);
	reflc->setRadius(radius);
	addComponent(reflc, true);

	return ErrorCode::NONE;
}

void ReflectionProbe::onMoveUpdate(MoveComponent& move)
{
	// Update frustum components
	U count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		Transform trf = m_cubeSides[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		frc.getFrustum().resetTransform(trf);
		frc.markTransformForUpdate();
		++count;

		return ErrorCode::NONE;
	});

	ANKI_ASSERT(count == 6);
	(void)err;

	// Update the spatial comp
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
	m_spatialSphere.setCenter(move.getWorldTransform().getOrigin());

	// Update the refl comp
	ReflectionProbeComponent& reflc = getComponent<ReflectionProbeComponent>();
	reflc.setPosition(move.getWorldTransform().getOrigin());
}

Error ReflectionProbe::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	// Check the reflection probe component and if it's marked for rendering enable the frustum components
	const ReflectionProbeComponent& reflc = getComponent<ReflectionProbeComponent>();

	FrustumComponentVisibilityTestFlag testFlags =
		reflc.getMarkedForRendering() ? FRUSTUM_TEST_FLAGS : FrustumComponentVisibilityTestFlag::NONE;

	Error err = iterateComponentsOfType<FrustumComponent>([testFlags](FrustumComponent& frc) -> Error {
		frc.setEnabledVisibilityTests(testFlags);
		return ErrorCode::NONE;
	});
	(void)err;

	return ErrorCode::NONE;
}

} // end namespace anki

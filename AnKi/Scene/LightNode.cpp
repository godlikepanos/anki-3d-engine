// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/LightNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SpatialComponent.h>
#include <AnKi/Scene/Components/FrustumComponent.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

/// Feedback component.
class LightNode::OnMovedFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LightNode::OnMovedFeedbackComponent)

public:
	OnMovedFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = info.m_node->getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			// Move updated
			static_cast<LightNode&>(*info.m_node).onMoved(move);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(LightNode::OnMovedFeedbackComponent)

/// Feedback component.
class LightNode::OnLightShapeUpdatedFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LightNode::OnLightShapeUpdatedFeedbackComponent)

public:
	OnLightShapeUpdatedFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		LightComponent& light = info.m_node->getFirstComponentOfType<LightComponent>();
		if(light.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			// Shape updated
			static_cast<LightNode&>(*info.m_node).onLightShapeUpdated(light);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(LightNode::OnLightShapeUpdatedFeedbackComponent)

LightNode::LightNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

LightNode::~LightNode()
{
}

void LightNode::frameUpdateCommon()
{
	// Update frustum comps shadow info
	const LightComponent& lc = getFirstComponentOfType<LightComponent>();
	const Bool castsShadow = lc.getShadowEnabled();

	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) {
		if(castsShadow)
		{
			frc.setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kShadowCasterRenderComponents);
		}
		else
		{
			frc.setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kNone);
		}
	});
}

void LightNode::onMoveUpdateCommon(const MoveComponent& move)
{
	// Update the spatial
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());

	// Update the lens flare
	LensFlareComponent& lf = getFirstComponentOfType<LensFlareComponent>();
	lf.setWorldPosition(move.getWorldTransform().getOrigin().xyz());

	// Update light component
	getFirstComponentOfType<LightComponent>().setWorldTransform(move.getWorldTransform());
}

PointLightNode::PointLightNode(SceneGraph* scene, CString name)
	: LightNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<OnMovedFeedbackComponent>();

	LightComponent* lc = newComponent<LightComponent>();
	lc->setLightComponentType(LightComponentType::kPoint);

	newComponent<LensFlareComponent>();
	newComponent<OnLightShapeUpdatedFeedbackComponent>();
	newComponent<SpatialComponent>();
}

PointLightNode::~PointLightNode()
{
	m_shadowData.destroy(getMemoryPool());
}

void PointLightNode::onMoved(const MoveComponent& move)
{
	onMoveUpdateCommon(move);

	// Update the frustums
	U32 count = 0;
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) {
		Transform trf = m_shadowData[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		fr.setWorldTransform(trf);
		++count;
	});
}

void PointLightNode::onLightShapeUpdated(LightComponent& light)
{
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) {
		fr.setFar(light.getRadius());
	});

	SpatialComponent& spatialc = getFirstComponentOfType<SpatialComponent>();
	spatialc.setSphereWorldSpace(Sphere(light.getWorldTransform().getOrigin(), light.getRadius()));
}

Error PointLightNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	// Lazily init
	const LightComponent& lightc = getFirstComponentOfType<LightComponent>();
	if(lightc.getShadowEnabled() && m_shadowData.isEmpty())
	{
		m_shadowData.create(getMemoryPool(), 6);

		const F32 ang = toRad(90.0f);
		const F32 dist = lightc.getRadius();
		const F32 zNear = kClusterObjectFrustumNearPlane;

		Mat3 rot;

		rot = Mat3(Euler(0.0, -kPi / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, kPi));
		m_shadowData[0].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(0.0, kPi / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, kPi));
		m_shadowData[1].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(kPi / 2.0, 0.0, 0.0));
		m_shadowData[2].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(-kPi / 2.0, 0.0, 0.0));
		m_shadowData[3].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(0.0, kPi, 0.0)) * Mat3(Euler(0.0, 0.0, kPi));
		m_shadowData[4].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(0.0, 0.0, kPi));
		m_shadowData[5].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));

		const Vec4& origin = getFirstComponentOfType<MoveComponent>().getWorldTransform().getOrigin();
		for(U32 i = 0; i < 6; i++)
		{
			Transform trf = m_shadowData[i].m_localTrf;
			trf.setOrigin(origin);

			FrustumComponent* frc = newComponent<FrustumComponent>();
			frc->setFrustumType(FrustumType::kPerspective);
			frc->setPerspective(zNear, dist, ang, ang);
			frc->setWorldTransform(trf);
		}
	}

	frameUpdateCommon();

	return Error::kNone;
}

class SpotLightNode::OnFrustumUpdatedFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SpotLightNode::OnFrustumUpdatedFeedbackComponent)

public:
	OnFrustumUpdatedFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;

		FrustumComponent& frc = info.m_node->getFirstComponentOfType<FrustumComponent>();
		if(frc.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			// Shape updated
			static_cast<SpotLightNode&>(*info.m_node).onFrustumUpdated(frc);
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(SpotLightNode::OnFrustumUpdatedFeedbackComponent)

SpotLightNode::SpotLightNode(SceneGraph* scene, CString name)
	: LightNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<OnMovedFeedbackComponent>();

	LightComponent* lc = newComponent<LightComponent>();
	lc->setLightComponentType(LightComponentType::kSpot);

	newComponent<LensFlareComponent>();

	newComponent<OnLightShapeUpdatedFeedbackComponent>();

	FrustumComponent* fr = newComponent<FrustumComponent>();
	fr->setFrustumType(FrustumType::kPerspective);
	fr->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::kNone);

	newComponent<OnFrustumUpdatedFeedbackComponent>();

	newComponent<SpatialComponent>();
}

void SpotLightNode::onMoved(const MoveComponent& move)
{
	// Update the frustums
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) {
		fr.setWorldTransform(move.getWorldTransform());
	});

	onMoveUpdateCommon(move);
}

void SpotLightNode::onLightShapeUpdated(LightComponent& light)
{
	FrustumComponent& frc = getFirstComponentOfType<FrustumComponent>();
	frc.setPerspective(kClusterObjectFrustumNearPlane, light.getDistance(), light.getOuterAngle(),
					   light.getOuterAngle());
}

void SpotLightNode::onFrustumUpdated(FrustumComponent& frc)
{
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setConvexHullWorldSpace(frc.getPerspectiveBoundingShapeWorldSpace());
}

Error SpotLightNode::frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
{
	frameUpdateCommon();
	return Error::kNone;
}

class DirectionalLightNode::FeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(DirectionalLightNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override
	{
		updated = false;
		const MoveComponent& move = info.m_node->getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == info.m_node->getGlobalTimestamp())
		{
			// Move updated
			LightComponent& lightc = info.m_node->getFirstComponentOfType<LightComponent>();
			lightc.setWorldTransform(move.getWorldTransform());

			SpatialComponent& spatialc = info.m_node->getFirstComponentOfType<SpatialComponent>();
			spatialc.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());
		}

		return Error::kNone;
	}
};

ANKI_SCENE_COMPONENT_STATICS(DirectionalLightNode::FeedbackComponent)

DirectionalLightNode::DirectionalLightNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent>();

	LightComponent* lc = newComponent<LightComponent>();
	lc->setLightComponentType(LightComponentType::kDirectional);

	SpatialComponent* spatialc = newComponent<SpatialComponent>();

	// Make the spatial always visible
	spatialc->setAlwaysVisible(true);
}

} // end namespace anki

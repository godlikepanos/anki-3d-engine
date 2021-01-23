// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/LightNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/shaders/include/ClusteredShadingTypes.h>

namespace anki
{

/// Feedback component.
class LightNode::OnMovedFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(LightNode::OnMovedFeedbackComponent)

public:
	OnMovedFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			static_cast<LightNode&>(node).onMoved(move);
		}

		return Error::NONE;
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

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		LightComponent& light = node.getFirstComponentOfType<LightComponent>();
		if(light.getTimestamp() == node.getGlobalTimestamp())
		{
			// Shape updated
			static_cast<LightNode&>(node).onLightShapeUpdated(light);
		}

		return Error::NONE;
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

	const Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
		if(castsShadow)
		{
			frc.setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::SHADOW_CASTERS);
		}
		else
		{
			frc.setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
		}

		return Error::NONE;
	});
	(void)err;
}

void LightNode::onMoveUpdateCommon(const MoveComponent& move)
{
	// Update the spatial
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());

	// Update the lens flare
	LensFlareComponent* lf = tryGetFirstComponentOfType<LensFlareComponent>();
	if(lf)
	{
		lf->setWorldPosition(move.getWorldTransform().getOrigin().xyz());
	}

	// Update light component
	getFirstComponentOfType<LightComponent>().setWorldTransform(move.getWorldTransform());
}

PointLightNode::PointLightNode(SceneGraph* scene, CString name)
	: LightNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<OnMovedFeedbackComponent>();

	LightComponent* lc = newComponent<LightComponent>();
	lc->setLightComponentType(LightComponentType::POINT);

	newComponent<OnLightShapeUpdatedFeedbackComponent>();
	newComponent<SpatialComponent>();
}

PointLightNode::~PointLightNode()
{
	m_shadowData.destroy(getAllocator());
}

void PointLightNode::onMoved(const MoveComponent& move)
{
	onMoveUpdateCommon(move);

	// Update the frustums
	U32 count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		Transform trf = m_shadowData[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		fr.setWorldTransform(trf);
		++count;

		return Error::NONE;
	});
	(void)err;
}

void PointLightNode::onLightShapeUpdated(LightComponent& light)
{
	const Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.setFar(light.getRadius());
		return Error::NONE;
	});
	(void)err;

	SpatialComponent& spatialc = getFirstComponentOfType<SpatialComponent>();
	spatialc.setSphereWorldSpace(Sphere(light.getWorldTransform().getOrigin(), light.getRadius()));
}

Error PointLightNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	// Lazily init
	const LightComponent& lightc = getFirstComponentOfType<LightComponent>();
	if(lightc.getShadowEnabled() && m_shadowData.isEmpty())
	{
		m_shadowData.create(getAllocator(), 6);

		const F32 ang = toRad(90.0f);
		const F32 dist = lightc.getRadius();
		const F32 zNear = LIGHT_FRUSTUM_NEAR_PLANE;

		Mat3 rot;

		rot = Mat3(Euler(0.0, -PI / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[0].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(0.0, PI / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[1].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(PI / 2.0, 0.0, 0.0));
		m_shadowData[2].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(-PI / 2.0, 0.0, 0.0));
		m_shadowData[3].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(0.0, PI, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[4].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));
		rot = Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[5].m_localTrf.setRotation(Mat3x4(Vec3(0.0f), rot));

		const Vec4& origin = getFirstComponentOfType<MoveComponent>().getWorldTransform().getOrigin();
		for(U32 i = 0; i < 6; i++)
		{
			Transform trf = m_shadowData[i].m_localTrf;
			trf.setOrigin(origin);

			FrustumComponent* frc = newComponent<FrustumComponent>();
			frc->setFrustumType(FrustumType::PERSPECTIVE);
			frc->setPerspective(zNear, dist, ang, ang);
			frc->setWorldTransform(trf);
		}
	}

	frameUpdateCommon();

	return Error::NONE;
}

class SpotLightNode::OnFrustumUpdatedFeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(SpotLightNode::OnFrustumUpdatedFeedbackComponent)

public:
	OnFrustumUpdatedFeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		FrustumComponent& frc = node.getFirstComponentOfType<FrustumComponent>();
		if(frc.getTimestamp() == node.getGlobalTimestamp())
		{
			// Shape updated
			static_cast<SpotLightNode&>(node).onFrustumUpdated(frc);
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(SpotLightNode::OnFrustumUpdatedFeedbackComponent)

SpotLightNode::SpotLightNode(SceneGraph* scene, CString name)
	: LightNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<OnMovedFeedbackComponent>();

	LightComponent* lc = newComponent<LightComponent>();
	lc->setLightComponentType(LightComponentType::SPOT);

	newComponent<OnLightShapeUpdatedFeedbackComponent>();

	FrustumComponent* fr = newComponent<FrustumComponent>();
	fr->setFrustumType(FrustumType::PERSPECTIVE);
	fr->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);

	newComponent<OnFrustumUpdatedFeedbackComponent>();

	newComponent<SpatialComponent>();
}

void SpotLightNode::onMoved(const MoveComponent& move)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.setWorldTransform(move.getWorldTransform());
		return Error::NONE;
	});

	(void)err;

	onMoveUpdateCommon(move);
}

void SpotLightNode::onLightShapeUpdated(LightComponent& light)
{
	FrustumComponent& frc = getFirstComponentOfType<FrustumComponent>();
	frc.setPerspective(LIGHT_FRUSTUM_NEAR_PLANE, light.getDistance(), light.getOuterAngle(), light.getOuterAngle());
}

void SpotLightNode::onFrustumUpdated(FrustumComponent& frc)
{
	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.setConvexHullWorldSpace(frc.getPerspectiveBoundingShapeWorldSpace());
}

Error SpotLightNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	frameUpdateCommon();
	return Error::NONE;
}

class DirectionalLightNode::FeedbackComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(DirectionalLightNode::FeedbackComponent)

public:
	FeedbackComponent(SceneNode* node)
		: SceneComponent(node, getStaticClassId(), true)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		const MoveComponent& move = node.getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			LightComponent& lightc = node.getFirstComponentOfType<LightComponent>();
			lightc.setWorldTransform(move.getWorldTransform());

			SpatialComponent& spatialc = node.getFirstComponentOfType<SpatialComponent>();
			spatialc.setSpatialOrigin(move.getWorldTransform().getOrigin().xyz());
		}

		return Error::NONE;
	}
};

ANKI_SCENE_COMPONENT_STATICS(DirectionalLightNode::FeedbackComponent)

DirectionalLightNode::DirectionalLightNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent>();

	LightComponent* lc = newComponent<LightComponent>();
	lc->setLightComponentType(LightComponentType::DIRECTIONAL);

	SpatialComponent* spatialc = newComponent<SpatialComponent>();

	// Make the bounding box large enough so it will always be visible. Because of that don't update the octree bounds
	Aabb boundingBox;
	boundingBox.setMin(getSceneGraph().getSceneMin());
	boundingBox.setMax(getSceneGraph().getSceneMax());
	spatialc->setAabbWorldSpace(boundingBox);
	spatialc->setUpdateOctreeBounds(false);
}

} // end namespace anki

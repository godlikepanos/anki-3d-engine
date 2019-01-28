// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/LightNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <shaders/glsl_cpp_common/ClusteredShading.h>

namespace anki
{

/// Feedback component.
class LightNode::MovedFeedbackComponent : public SceneComponent
{
public:
	MovedFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			static_cast<LightNode&>(node).onMoveUpdate(move);
		}

		return Error::NONE;
	}
};

/// Feedback component.
class LightNode::LightChangedFeedbackComponent : public SceneComponent
{
public:
	LightChangedFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		LightComponent& light = node.getComponent<LightComponent>();
		if(light.getTimestamp() == node.getGlobalTimestamp())
		{
			// Shape updated
			static_cast<LightNode&>(node).onShapeUpdate(light);
		}

		return Error::NONE;
	}
};

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
	const LightComponent& lc = getComponent<LightComponent>();
	const Bool castsShadow = lc.getShadowEnabled();

	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& frc) -> Error {
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
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());

	// Update the lens flare
	LensFlareComponent* lf = tryGetComponent<LensFlareComponent>();
	if(lf)
	{
		lf->setWorldPosition(move.getWorldTransform().getOrigin());
	}

	// Update light component
	getComponent<LightComponent>().updateWorldTransform(move.getWorldTransform());
}

Error LightNode::loadLensFlare(const CString& filename)
{
	ANKI_ASSERT(tryGetComponent<LensFlareComponent>() == nullptr);

	LensFlareComponent* flareComp = newComponent<LensFlareComponent>(this);

	Error err = flareComp->init(filename);
	if(err)
	{
		ANKI_ASSERT(!"TODO: Remove component");
		return err;
	}

	return Error::NONE;
}

PointLightNode::PointLightNode(SceneGraph* scene, CString name)
	: LightNode(scene, name)
{
}

PointLightNode::~PointLightNode()
{
	m_shadowData.destroy(getAllocator());
}

Error PointLightNode::init()
{
	// Move component
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MovedFeedbackComponent>();

	// Light component
	newComponent<LightComponent>(LightComponentType::POINT, getSceneGraph().getNewUuid());

	// Feedback component
	newComponent<LightChangedFeedbackComponent>();

	// Spatial component
	newComponent<SpatialComponent>(this, &m_sphereW);

	return Error::NONE;
}

void PointLightNode::onMoveUpdate(const MoveComponent& move)
{
	onMoveUpdateCommon(move);

	// Update the frustums
	U count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		Transform trf = m_shadowData[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		fr.setTransform(trf);
		++count;

		return Error::NONE;
	});

	(void)err;

	m_sphereW.setCenter(move.getWorldTransform().getOrigin());
}

void PointLightNode::onShapeUpdate(LightComponent& light)
{
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.setFar(light.getRadius());
		return Error::NONE;
	});
	(void)err;

	m_sphereW.setRadius(light.getRadius());
}

Error PointLightNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	if(getComponent<LightComponent>().getShadowEnabled() && m_shadowData.isEmpty())
	{
		m_shadowData.create(getAllocator(), 6);

		const F32 ang = toRad(90.0);
		const F32 dist = m_sphereW.getRadius();
		const F32 zNear = LIGHT_FRUSTUM_NEAR_PLANE;

		Mat3 rot;

		rot = Mat3(Euler(0.0, -PI / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[0].m_localTrf.setRotation(Mat3x4(rot));
		rot = Mat3(Euler(0.0, PI / 2.0, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[1].m_localTrf.setRotation(Mat3x4(rot));
		rot = Mat3(Euler(PI / 2.0, 0.0, 0.0));
		m_shadowData[2].m_localTrf.setRotation(Mat3x4(rot));
		rot = Mat3(Euler(-PI / 2.0, 0.0, 0.0));
		m_shadowData[3].m_localTrf.setRotation(Mat3x4(rot));
		rot = Mat3(Euler(0.0, PI, 0.0)) * Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[4].m_localTrf.setRotation(Mat3x4(rot));
		rot = Mat3(Euler(0.0, 0.0, PI));
		m_shadowData[5].m_localTrf.setRotation(Mat3x4(rot));

		const Vec4& origin = getComponent<MoveComponent>().getWorldTransform().getOrigin();
		for(U i = 0; i < 6; i++)
		{
			Transform trf = m_shadowData[i].m_localTrf;
			trf.setOrigin(origin);

			FrustumComponent* frc = newComponent<FrustumComponent>(this, FrustumType::PERSPECTIVE);
			frc->setPerspective(zNear, dist, ang, ang);
			frc->setTransform(trf);
		}
	}

	frameUpdateCommon();

	return Error::NONE;
}

SpotLightNode::SpotLightNode(SceneGraph* scene, CString name)
	: LightNode(scene, name)
{
}

Error SpotLightNode::init()
{
	// Move component
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MovedFeedbackComponent>();

	// Light component
	newComponent<LightComponent>(LightComponentType::POINT, getSceneGraph().getNewUuid());

	// Feedback component
	newComponent<LightChangedFeedbackComponent>();

	// Frustum component
	FrustumComponent* fr = newComponent<FrustumComponent>(this, FrustumType::PERSPECTIVE);
	fr->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);

	// Spatial component
	newComponent<SpatialComponent>(this, &fr->getPerspectiveBoundingShape());

	return Error::NONE;
}

void SpotLightNode::onMoveUpdate(const MoveComponent& move)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.setTransform(move.getWorldTransform());
		return Error::NONE;
	});

	(void)err;

	onMoveUpdateCommon(move);
}

void SpotLightNode::onShapeUpdate(LightComponent& light)
{
	// Update the frustum first
	FrustumComponent& frc = getComponent<FrustumComponent>();
	frc.setPerspective(LIGHT_FRUSTUM_NEAR_PLANE, light.getDistance(), light.getOuterAngle(), light.getOuterAngle());

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

Error SpotLightNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	frameUpdateCommon();
	return Error::NONE;
}

class DirectionalLightNode::FeedbackComponent : public SceneComponent
{
public:
	FeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		const MoveComponent& move = node.getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			LightComponent& lightc = node.getComponent<LightComponent>();
			lightc.updateWorldTransform(move.getWorldTransform());

			SpatialComponent& spatialc = node.getComponent<SpatialComponent>();
			spatialc.setSpatialOrigin(move.getWorldTransform().getOrigin());
			spatialc.markForUpdate();
		}

		return Error::NONE;
	}
};

DirectionalLightNode::DirectionalLightNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

Error DirectionalLightNode::init()
{
	newComponent<MoveComponent>();
	newComponent<FeedbackComponent>();
	newComponent<LightComponent>(LightComponentType::DIRECTIONAL, getSceneGraph().getNewUuid());
	SpatialComponent* spatialc = newComponent<SpatialComponent>(this, &m_boundingBox);

	// Make the bounding box large enough so it will always be visible. Because of that don't update the octree bounds
	m_boundingBox.setMin(getSceneGraph().getSceneMin());
	m_boundingBox.setMax(getSceneGraph().getSceneMax());
	spatialc->setUpdateOctreeBounds(false);

	return Error::NONE;
}

} // end namespace anki

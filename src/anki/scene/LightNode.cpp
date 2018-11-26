// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
		LightNode& lnode = static_cast<LightNode&>(node);

		const MoveComponent& move = node.getComponentAt<MoveComponent>(0);
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			lnode.onMoveUpdate(move);
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
		LightNode& lnode = static_cast<LightNode&>(node);

		LightComponent& light = node.getComponent<LightComponent>();
		if(light.getTimestamp() == node.getGlobalTimestamp())
		{
			// Shape updated
			lnode.onShapeUpdate(light);
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

Error LightNode::init(LightComponentType type, CollisionShape* shape)
{
	// Move component
	newComponent<MoveComponent>();

	// Feedback component
	newComponent<MovedFeedbackComponent>();

	// Light component
	newComponent<LightComponent>(type, getSceneGraph().getNewUuid());

	// Feedback component
	newComponent<LightChangedFeedbackComponent>();

	// Spatial component
	newComponent<SpatialComponent>(this, shape);

	return Error::NONE;
}

void LightNode::frameUpdateCommon()
{
	// Update frustum comps shadow info
	const LightComponent& lc = getComponent<LightComponent>();
	Bool castsShadow = lc.getShadowEnabled();

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

void LightNode::onShapeUpdateCommon(LightComponent& light)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.markShapeForUpdate();
		return Error::NONE;
	});

	(void)err;

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
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
	return LightNode::init(LightComponentType::POINT, &m_sphereW);
}

void PointLightNode::onMoveUpdate(const MoveComponent& move)
{
	onMoveUpdateCommon(move);

	// Update the frustums
	U count = 0;
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		Transform trf = m_shadowData[count].m_localTrf;
		trf.setOrigin(move.getWorldTransform().getOrigin());

		fr.getFrustum().resetTransform(trf);
		fr.markTransformForUpdate();
		++count;

		return Error::NONE;
	});

	(void)err;

	m_sphereW.setCenter(move.getWorldTransform().getOrigin());
}

void PointLightNode::onShapeUpdate(LightComponent& light)
{
	for(ShadowCombo& c : m_shadowData)
	{
		c.m_frustum.setFar(light.getRadius());
	}

	m_sphereW.setRadius(light.getRadius());

	onShapeUpdateCommon(light);
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
			m_shadowData[i].m_frustum.setAll(ang, ang, zNear, dist);

			Transform trf = m_shadowData[i].m_localTrf;
			trf.setOrigin(origin);
			m_shadowData[i].m_frustum.resetTransform(trf);

			newComponent<FrustumComponent>(this, &m_shadowData[i].m_frustum);
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
	ANKI_CHECK(LightNode::init(LightComponentType::SPOT, &m_frustum));

	FrustumComponent* fr = newComponent<FrustumComponent>(this, &m_frustum);
	fr->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);

	return Error::NONE;
}

void SpotLightNode::onMoveUpdate(const MoveComponent& move)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.markTransformForUpdate();
		fr.getFrustum().resetTransform(move.getWorldTransform());

		return Error::NONE;
	});

	(void)err;

	onMoveUpdateCommon(move);
}

void SpotLightNode::onShapeUpdate(LightComponent& light)
{
	onShapeUpdateCommon(light);
	m_frustum.setAll(light.getOuterAngle(), light.getOuterAngle(), LIGHT_FRUSTUM_NEAR_PLANE, light.getDistance());
}

Error SpotLightNode::frameUpdate(Second prevUpdateTime, Second crntTime)
{
	frameUpdateCommon();
	return Error::NONE;
}

} // end namespace anki

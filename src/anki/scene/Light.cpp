// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Light.h>
#include <anki/scene/LensFlareComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/FrustumComponent.h>

namespace anki
{

/// Feedback component.
class LightFeedbackComponent : public SceneComponent
{
public:
	LightFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponentType::NONE, node)
	{
	}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;
		Light& lnode = static_cast<Light&>(node);

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			lnode.onMoveUpdate(move);
		}

		LightComponent& light = node.getComponent<LightComponent>();
		if(light.getTimestamp() == node.getGlobalTimestamp())
		{
			// Shape updated
			lnode.onShapeUpdate(light);
		}

		return ErrorCode::NONE;
	}
};

Light::Light(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

Light::~Light()
{
}

Error Light::init(LightComponentType type, CollisionShape* shape)
{
	SceneComponent* comp;

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Light component
	comp = getSceneAllocator().newInstance<LightComponent>(this, type);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().newInstance<LightFeedbackComponent>(this);
	addComponent(comp, true);

	// Spatial component
	comp = getSceneAllocator().newInstance<SpatialComponent>(this, shape);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

void Light::frameUpdateCommon()
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

		return ErrorCode::NONE;
	});
	(void)err;
}

void Light::onMoveUpdateCommon(MoveComponent& move)
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
}

void Light::onShapeUpdateCommon(LightComponent& light)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.markShapeForUpdate();
		return ErrorCode::NONE;
	});

	(void)err;

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

Error Light::loadLensFlare(const CString& filename)
{
	ANKI_ASSERT(tryGetComponent<LensFlareComponent>() == nullptr);

	LensFlareComponent* flareComp = getSceneAllocator().newInstance<LensFlareComponent>(this);

	Error err = flareComp->init(filename);
	if(err)
	{
		getSceneAllocator().deleteInstance(flareComp);
		return err;
	}

	addComponent(flareComp, true);

	return ErrorCode::NONE;
}

PointLight::PointLight(SceneGraph* scene, CString name)
	: Light(scene, name)
{
}

PointLight::~PointLight()
{
	m_shadowData.destroy(getSceneAllocator());
}

Error PointLight::init()
{
	return Light::init(LightComponentType::POINT, &m_sphereW);
}

void PointLight::onMoveUpdate(MoveComponent& move)
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

		return ErrorCode::NONE;
	});

	(void)err;

	m_sphereW.setCenter(move.getWorldTransform().getOrigin());
}

void PointLight::onShapeUpdate(LightComponent& light)
{
	for(ShadowCombo& c : m_shadowData)
	{
		c.m_frustum.setFar(light.getRadius());
	}

	m_sphereW.setRadius(light.getRadius());

	onShapeUpdateCommon(light);
}

Error PointLight::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	if(getComponent<LightComponent>().getShadowEnabled() && m_shadowData.isEmpty())
	{
		m_shadowData.create(getSceneAllocator(), 6);

		const F32 ang = toRad(90.0);
		const F32 dist = m_sphereW.getRadius();
		const F32 zNear = LightComponent::FRUSTUM_NEAR_PLANE;

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

			FrustumComponent* comp =
				getSceneAllocator().newInstance<FrustumComponent>(this, &m_shadowData[i].m_frustum);

			addComponent(comp, true);
		}
	}

	frameUpdateCommon();

	return ErrorCode::NONE;
}

SpotLight::SpotLight(SceneGraph* scene, CString name)
	: Light(scene, name)
{
}

Error SpotLight::init()
{
	ANKI_CHECK(Light::init(LightComponentType::SPOT, &m_frustum));

	FrustumComponent* fr = getSceneAllocator().newInstance<FrustumComponent>(this, &m_frustum);
	fr->setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);

	addComponent(fr, true);

	return ErrorCode::NONE;
}

void SpotLight::onMoveUpdate(MoveComponent& move)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr) -> Error {
		fr.markTransformForUpdate();
		fr.getFrustum().resetTransform(move.getWorldTransform());

		return ErrorCode::NONE;
	});

	(void)err;

	onMoveUpdateCommon(move);
}

void SpotLight::onShapeUpdate(LightComponent& light)
{
	onShapeUpdateCommon(light);
	m_frustum.setAll(
		light.getOuterAngle(), light.getOuterAngle(), LightComponent::FRUSTUM_NEAR_PLANE, light.getDistance());
}

Error SpotLight::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	frameUpdateCommon();
	return ErrorCode::NONE;
}

} // end namespace anki

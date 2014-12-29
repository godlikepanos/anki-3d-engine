// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Light.h"
#include "anki/scene/LensFlareComponent.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/FrustumComponent.h"

namespace anki {

//==============================================================================
// LightComponent                                                              =
//==============================================================================

//==============================================================================
LightComponent::LightComponent(SceneNode* node, LightType type)
:	SceneComponent(Type::LIGHT, node),
	m_type(type)
{
	setInnerAngle(toRad(45.0));
	setOuterAngle(toRad(30.0));
	m_radius = 1.0;
}

//==============================================================================
Error LightComponent::update(SceneNode&, F32, F32, Bool& updated)
{
	if(m_dirty)
	{
		updated = true;
		m_dirty = false;
	}

	return ErrorCode::NONE;
}

//==============================================================================
// LightFeedbackComponent                                                      =
//==============================================================================

/// Feedback component.
class LightFeedbackComponent: public SceneComponent
{
public:
	LightFeedbackComponent(SceneNode* node)
	:	SceneComponent(SceneComponent::Type::NONE, node)
	{}

	Error update(SceneNode& node, F32, F32, Bool& updated) override
	{
		updated = false;
		Light& lnode = static_cast<Light&>(node);

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == getGlobTimestamp())
		{
			// Move updated
			lnode.onMoveUpdate(move);
		}

		LightComponent& light = node.getComponent<LightComponent>();
		if(light.getTimestamp() == getGlobTimestamp())
		{
			// Shape updated
			lnode.onShapeUpdate(light);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(SceneGraph* scene)
:	SceneNode(scene)
{}

//==============================================================================
Error Light::create(const CString& name, 
	LightComponent::LightType type,
	CollisionShape* shape)
{
	Error err = SceneNode::create(name);
	if(err) return err;

	SceneComponent* comp;

	// Move component
	comp = getSceneAllocator().newInstance<MoveComponent>(this);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY; 
	comp->setAutomaticCleanup(true);

	err = addComponent(comp);
	if(err) return err;

	// Light component
	comp = getSceneAllocator().newInstance<LightComponent>(this, type);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY; 
	comp->setAutomaticCleanup(true);

	err = addComponent(comp);
	if(err) return err;

	// Feedback component
	comp = getSceneAllocator().newInstance<LightFeedbackComponent>(this);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY; 
	comp->setAutomaticCleanup(true);

	err = addComponent(comp);
	if(err) return err;

	// Spatial component
	comp = getSceneAllocator().newInstance<SpatialComponent>(this, shape);
	if(comp == nullptr) return ErrorCode::OUT_OF_MEMORY; 
	comp->setAutomaticCleanup(true);

	err = addComponent(comp);
	if(err) return err;

	return err;
}

//==============================================================================
Light::~Light()
{}

//==============================================================================
void Light::onMoveUpdateCommon(MoveComponent& move)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& fr) -> Error
	{
		fr.markTransformForUpdate();
		fr.getFrustum().resetTransform(move.getWorldTransform());

		return ErrorCode::NONE;
	});

	(void)err;

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

//==============================================================================
void Light::onShapeUpdateCommon(LightComponent& light)
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& fr) -> Error
	{
		fr.markShapeForUpdate();
		return ErrorCode::NONE;
	});

	(void)err;

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
Error Light::loadLensFlare(const CString& filename)
{
	ANKI_ASSERT(tryGetComponent<LensFlareComponent>() == nullptr);
	Error err = ErrorCode::NONE;

	LensFlareComponent* flareComp = 
		getSceneAllocator().newInstance<LensFlareComponent>(this);
	
	if(flareComp)
	{
		err = flareComp->create(filename);
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	if(!err)
	{
		err = addComponent(flareComp);
	}

	if(!err)
	{
		flareComp->setAutomaticCleanup(true);
	}

	// Clean up on any error
	if(err && flareComp)
	{
		getSceneAllocator().deleteInstance(flareComp);
	}

	return err;
}

//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(SceneGraph* scene)
:	Light(scene)
{}

//==============================================================================
Error PointLight::create(const CString& name)
{
	return Light::create(name, LightComponent::LightType::POINT, &m_sphereW);
}

//==============================================================================
void PointLight::onMoveUpdate(MoveComponent& move)
{
	onMoveUpdateCommon(move);
	m_sphereW.setCenter(move.getWorldTransform().getOrigin());
}

//==============================================================================
void PointLight::onShapeUpdate(LightComponent& light)
{
	onShapeUpdateCommon(light);
	m_sphereW.setRadius(light.getRadius());
}

//==============================================================================
Error PointLight::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
#if 0
	if(getShadowEnabled() && m_shadowData == nullptr)
	{
		m_shadowData = getSceneAllocator().newInstance<ShadowData>(this);
		if(m_shadowData == nullptr)
		{
			return ErrorCode::OUT_OF_MEMORY;
		}

		const F32 ang = toRad(90.0);
		F32 dist = m_sphereW.getRadius();

		for(U i = 0; i < 6; i++)
		{
			m_shadowData->m_frustums[i].setAll(ang, ang, 0.1, dist);
			m_shadowData->m_localTrfs[i] = Transform::getIdentity();
		}

		auto& trfs = m_shadowData->m_localTrfs;
		Vec3 axis = Vec3(0.0, 1.0, 0.0);
		trfs[1].setRotation(Mat3x4(Mat3(Axisang(ang, axis))));
		trfs[2].setRotation(Mat3x4(Mat3(Axisang(ang * 2.0, axis))));
		trfs[3].setRotation(Mat3x4(Mat3(Axisang(ang * 3.0, axis))));

		axis = Vec3(1.0, 0.0, 0.0);
		trfs[4].setRotation(Mat3x4(Mat3(Axisang(ang, axis))));
		trfs[5].setRotation(Mat3x4(Mat3(Axisang(-ang, axis))));
	}
#endif

	return ErrorCode::NONE;
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(SceneGraph* scene)
:	Light(scene)
{}

//==============================================================================
Error SpotLight::create(const CString& name)
{
	Error err = Light::create(
		name, LightComponent::LightType::SPOT, &m_frustum);
	if(err) return err;

	FrustumComponent* fr = 
		getSceneAllocator().newInstance<FrustumComponent>(this, &m_frustum);
	if(fr == nullptr) return ErrorCode::OUT_OF_MEMORY;
	
	err = addComponent(fr, true);
	if(err) return err;

	return err;
}

//==============================================================================
void SpotLight::onMoveUpdate(MoveComponent& move)
{
	onMoveUpdateCommon(move);
}

//==============================================================================
void SpotLight::onShapeUpdate(LightComponent& light)
{
	onShapeUpdateCommon(light);
	m_frustum.setAll(
		light.getOuterAngle(), light.getOuterAngle(), 
		0.5, light.getDistance());
}

} // end namespace anki

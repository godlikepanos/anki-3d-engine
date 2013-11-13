#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(
	const char* name, SceneGraph* scene, // SceneNode
	CollisionShape* cs, // Spatial
	LightType t) // Self
	: SceneNode(name, scene), type(t)
{
	newComponent<MoveComponent>(this);
	newComponent<SpatialComponent>(this, cs);
	newComponent<LightComponent>(this);
}

//==============================================================================
Light::~Light()
{}

//==============================================================================
void Light::frustumUpdate()
{
	// Update the frustums
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr)
	{
		fr.setProjectionMatrix(fr.getFrustum().calculateProjectionMatrix());
		fr.setViewProjectionMatrix(
			fr.getProjectionMatrix() * fr.getViewMatrix());
		fr.markForUpdate();
	});

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
void Light::moveUpdate(MoveComponent& move)
{
	// Update the frustums
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr)
	{
		fr.setProjectionMatrix(fr.getFrustum().calculateProjectionMatrix());
		fr.setViewProjectionMatrix(
			fr.getProjectionMatrix() * fr.getViewMatrix());
		fr.setOrigin(move.getWorldTransform().getOrigin());
		fr.getFrustum().setTransform(move.getWorldTransform());

		fr.markForUpdate();
	});

	// Update the spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.setOrigin(move.getWorldTransform().getOrigin());
	sp.markForUpdate();
}

//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(const char* name, SceneGraph* scene)
	:	Light(name, scene, &sphereW, LT_POINT)
{}

//==============================================================================
void PointLight::componentUpdated(SceneComponent& comp, 
	SceneComponent::UpdateType)
{
	if(comp.getTypeId() == SceneComponent::getTypeIdOf<MoveComponent>())
	{
		MoveComponent& move = static_cast<MoveComponent&>(comp);
		sphereW.setCenter(move.getWorldTransform().getOrigin());
		moveUpdate(move);
	}
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const char* name, SceneGraph* scene)
	: Light(name, scene, &frustum, LT_SPOT)
{
	newComponent<FrustumComponent>(this, &frustum);

	const F32 ang = toRad(45.0);
	setOuterAngle(ang / 2.0);
	const F32 dist = 1.0;

	// Fix frustum
	//
	frustum.setAll(ang, ang, 0.1, dist);
	frustumUpdate();
}

//==============================================================================
void SpotLight::componentUpdated(SceneComponent& comp,
	SceneComponent::UpdateType)
{
	if(comp.getTypeId() == SceneComponent::getTypeIdOf<MoveComponent>())
	{
		MoveComponent& move = static_cast<MoveComponent&>(comp);
		frustum.setTransform(move.getWorldTransform());
		moveUpdate(move);
	}
}

} // end namespace anki

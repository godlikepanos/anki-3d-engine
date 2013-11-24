#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(
	const char* name, SceneGraph* scene, // SceneNode
	LightType t) // Self
	:	SceneNode(name, scene),
		LightComponent(this),
		MoveComponent(this),
		SpatialComponent(this),
		type(t)
{
	// Init components
	addComponent(static_cast<LightComponent*>(this));
	addComponent(static_cast<MoveComponent*>(this));
	addComponent(static_cast<SpatialComponent*>(this));
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
		fr.setViewMatrix(Mat4(move.getWorldTransform().getInverse()));
		fr.setViewProjectionMatrix(
			fr.getProjectionMatrix() * fr.getViewMatrix());

		fr.getFrustum().setTransform(move.getWorldTransform());

		fr.markForUpdate();
	});

	// Update the spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(const char* name, SceneGraph* scene)
	:	Light(name, scene, LT_POINT)
{}

//==============================================================================
void PointLight::componentUpdated(SceneComponent& comp, 
	SceneComponent::UpdateType)
{
	if(comp.getType() == MoveComponent::getGlobType())
	{
		MoveComponent& move = comp.downCast<MoveComponent>();
		sphereW.setCenter(move.getWorldTransform().getOrigin());
		moveUpdate(move);
	}
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const char* name, SceneGraph* scene)
	:	Light(name, scene, LT_SPOT),
		FrustumComponent(this)
{
	// Init components
	addComponent(static_cast<FrustumComponent*>(this));

	const F32 ang = toRad(45.0);
	const F32 dist = 1.0;

	frustum.setAll(ang, ang, 0.1, dist);
}

//==============================================================================
void SpotLight::componentUpdated(SceneComponent& comp,
	SceneComponent::UpdateType)
{
	if(comp.getType() == MoveComponent::getGlobType())
	{
		MoveComponent& move = comp.downCast<MoveComponent>();
		frustum.setTransform(move.getWorldTransform());
		moveUpdate(move);
	}
}

} // end namespace anki

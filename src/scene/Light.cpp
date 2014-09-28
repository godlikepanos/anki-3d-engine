// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// LightComponent                                                              =
//==============================================================================

//==============================================================================
LightComponent::LightComponent(Light* node)
:	SceneComponent(Type::LIGHT, node)
{}

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(
	const CString& name, SceneGraph* scene, // SceneNode
	Type t) // Self
:	SceneNode(name, scene),
	LightComponent(this),
	MoveComponent(this),
	SpatialComponent(this),
	m_type(t)
{
	// Register components
	addComponent(static_cast<MoveComponent*>(this));
	addComponent(static_cast<SpatialComponent*>(this));
	addComponent(static_cast<LightComponent*>(this));
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
	});

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
void Light::onMoveComponentUpdateCommon()
{
	MoveComponent& move = *this;

	// Update the frustums
	iterateComponentsOfType<FrustumComponent>([&](FrustumComponent& fr)
	{
		fr.setProjectionMatrix(fr.getFrustum().calculateProjectionMatrix());
		fr.setViewMatrix(Mat4(move.getWorldTransform().getInverse()));
		fr.setViewProjectionMatrix(
			fr.getProjectionMatrix() * fr.getViewMatrix());

		fr.getFrustum().resetTransform(move.getWorldTransform());

		fr.markForUpdate();
	});

	// Update the spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
void Light::loadLensFlare(const CString& filename)
{
	ANKI_ASSERT(!hasLensFlare());
	m_flaresTex.load(filename, &getResourceManager());
}

//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(const CString& name, SceneGraph* scene)
:	Light(name, scene, Light::Type::POINT)
{}

//==============================================================================
void PointLight::onMoveComponentUpdate(SceneNode&, F32, F32)
{
	m_sphereW.setCenter(getWorldTransform().getOrigin());
	onMoveComponentUpdateCommon();
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const CString& name, SceneGraph* scene)
:	Light(name, scene, Light::Type::SPOT),
	FrustumComponent(this, &m_frustum)
{
	// Init components
	addComponent(static_cast<FrustumComponent*>(this));

	const F32 ang = toRad(45.0);
	const F32 dist = 1.0;

	m_frustum.setAll(ang, ang, 0.1, dist);
}

//==============================================================================
void SpotLight::onMoveComponentUpdate(SceneNode&, F32, F32)
{
	m_frustum.resetTransform(getWorldTransform());
	onMoveComponentUpdateCommon();
}

//==============================================================================
void SpotLight::loadTexture(const CString& filename)
{
	m_tex.load(filename, &getResourceManager());
}

} // end namespace anki

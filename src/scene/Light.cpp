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
void PointLight::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	if(getShadowEnabled() && m_shadowData == nullptr)
	{
		m_shadowData = getSceneAllocator().newInstance<ShadowData>(this);

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

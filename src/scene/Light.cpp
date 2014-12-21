// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Light.h"
#include "anki/scene/LensFlareComponent.h"

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
Light::Light(SceneGraph* scene, Type t)
:	SceneNode(scene),
	LightComponent(this),
	MoveComponent(this),
	SpatialComponent(this),
	m_type(t)
{}

//==============================================================================
Error Light::create(const CString& name)
{
	Error err = SceneNode::create(name);
	
	if(!err)
	{
		err = addComponent(static_cast<MoveComponent*>(this));
	}

	if(!err)
	{
		err = addComponent(static_cast<SpatialComponent*>(this));
	}

	if(!err)
	{
		err = addComponent(static_cast<LightComponent*>(this));
	}

	return err;
}

//==============================================================================
Light::~Light()
{
	LensFlareComponent* flareComp = tryGetComponent<LensFlareComponent>();
	if(flareComp)
	{
		getSceneAllocator().deleteInstance(flareComp);
	}
}

//==============================================================================
void Light::frustumUpdate()
{
	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& fr) -> Error
	{
		fr.setProjectionMatrix(fr.getFrustum().calculateProjectionMatrix());
		fr.setViewProjectionMatrix(
			fr.getProjectionMatrix() * fr.getViewMatrix());

		return ErrorCode::NONE;
	});

	(void)err;

	// Mark the spatial for update
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();
}

//==============================================================================
void Light::onMoveComponentUpdateCommon()
{
	MoveComponent& move = *this;

	// Update the frustums
	Error err = iterateComponentsOfType<FrustumComponent>(
		[&](FrustumComponent& fr) -> Error
	{
		fr.setProjectionMatrix(fr.getFrustum().calculateProjectionMatrix());
		fr.setViewMatrix(Mat4(move.getWorldTransform().getInverse()));
		fr.setViewProjectionMatrix(
			fr.getProjectionMatrix() * fr.getViewMatrix());

		fr.getFrustum().resetTransform(move.getWorldTransform());

		fr.markForUpdate();

		return ErrorCode::NONE;
	});

	(void)err;

	// Update the spatial
	SpatialComponent& sp = getComponent<SpatialComponent>();
	sp.markForUpdate();

	// Update the lens flare
	LensFlareComponent* lf = tryGetComponent<LensFlareComponent>();
	if(lf)
	{
		lf->setWorldPosition(move.getWorldTransform().getOrigin());
	}
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
:	Light(scene, Light::Type::POINT)
{}

//==============================================================================
Error PointLight::create(const CString& name)
{
	return Light::create(name);
}

//==============================================================================
Error PointLight::onMoveComponentUpdate(SceneNode&, F32, F32)
{
	m_sphereW.setCenter(getWorldTransform().getOrigin());
	onMoveComponentUpdateCommon();
	return ErrorCode::NONE;
}

//==============================================================================
Error PointLight::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
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

	return ErrorCode::NONE;
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(SceneGraph* scene)
:	Light(scene, Light::Type::SPOT),
	FrustumComponent(this, &m_frustum)
{}

//==============================================================================
Error SpotLight::create(const CString& name)
{
	Error err = Light::create(name);

	if(!err)
	{
		err = addComponent(static_cast<FrustumComponent*>(this));
	}

	if(!err)
	{
		const F32 ang = toRad(45.0);
		const F32 dist = 1.0;
		m_frustum.setAll(ang, ang, 0.1, dist);
	}

	return err;
}

//==============================================================================
Error SpotLight::onMoveComponentUpdate(SceneNode&, F32, F32)
{
	m_frustum.resetTransform(getWorldTransform());
	onMoveComponentUpdateCommon();
	return ErrorCode::NONE;
}

//==============================================================================
Error SpotLight::loadTexture(const CString& filename)
{
	return m_tex.load(filename, &getResourceManager());
}

} // end namespace anki

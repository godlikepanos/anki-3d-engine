// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/Forward.h"
#include "anki/resource/Resource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Light component. It's a dummy component used to identify lights
class LightComponent: public SceneComponent
{
public:
	LightComponent(Light* node);

	static constexpr Type getClassType()
	{
		return Type::LIGHT;
	}

private:
	Vec4 m_diffColor = Vec4(0.5);
	Vec4 m_specColor = Vec4(0.5);

	Bool8 m_shadow = false;
	U8 m_shadowMapIndex = 0xFF; ///< Used by the renderer
};

/// Light scene node. It can be spot or point
///
/// Explaining the lighting model:
/// @code
/// Final intensity:                If = Ia + Id + Is
/// Ambient intensity:              Ia = Al * Am
/// Ambient intensity of light:     Al
/// Ambient intensity of material:  Am
/// Diffuse intensity:              Id = Dl * Dm * LambertTerm
/// Diffuse intensity of light:     Dl
/// Diffuse intensity of material:  Dm
/// LambertTerm:                    max(Normal dot Light, 0.0)
/// Specular intensity:             Is = Sm * Sl * pow(max(R dot E, 0.0), f)
/// Specular intensity of light:    Sl
/// Specular intensity of material: Sm
/// @endcode
class Light: public SceneNode, public LightComponent, public MoveComponent, 
	public SpatialComponent
{
public:
	enum class Type: U8
	{
		POINT,
		SPOT,
		COUNT
	};

	Light(SceneGraph* scene, Type t);

	virtual ~Light();

	ANKI_USE_RESULT Error create(const CString& name);

	Type getLightType() const
	{
		return m_type;
	}

	const Vec4& getDiffuseColor() const
	{
		return m_color;
	}

	Vec4& getDiffuseColor()
	{
		return m_color;
	}

	void setDiffuseColor(const Vec4& x)
	{
		m_color = x;
	}

	const Vec4& getSpecularColor() const
	{
		return m_specColor;
	}

	Vec4& getSpecularColor()
	{
		return m_specColor;
	}

	void setSpecularColor(const Vec4& x)
	{
		m_specColor = x;
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	void setShadowEnabled(const Bool x)
	{
		m_shadow = x;
	}

	U getShadowMapIndex() const
	{
		return static_cast<U>(m_shadowMapIndex);
	}

	void setShadowMapIndex(const U i)
	{
		ANKI_ASSERT(i < 0xFF);
		m_shadowMapIndex = static_cast<U8>(i);
	}

	ANKI_USE_RESULT Error loadLensFlare(const CString& filename);

	/// @name SpatialComponent virtuals
	/// @{
	Vec4 getSpatialOrigin()
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

protected:
	/// One of the frustums got updated
	void frustumUpdate();

	/// Called when moved
	void onMoveComponentUpdateCommon();

private:
	Type m_type;
	Vec4 m_color = Vec4(1.0);
	Vec4 m_specColor = Vec4(1.0);

	Bool8 m_shadow = false;
	U8 m_shadowMapIndex = 0xFF; ///< Used by the renderer
};

/// Point light
class PointLight: public Light
{
public:
	PointLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

	F32 getRadius() const
	{
		return m_sphereW.getRadius();
	}

	void setRadius(const F32 x)
	{
		m_sphereW.setRadius(x);
		frustumUpdate();
	}

	const Sphere& getSphere() const
	{
		return m_sphereW;
	}

	/// @name SceneNode virtuals
	/// @{
	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;
	/// @}

	/// @name MoveComponent virtuals
	/// @{
	ANKI_USE_RESULT Error onMoveComponentUpdate(SceneNode&, F32, F32) override;
	/// @}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return m_sphereW;
	}
	/// @}

public:
	class ShadowData
	{
	public:
		ShadowData(SceneNode* node)
		:	m_frustumComps{{
				{node, &m_frustums[0]}, {node, &m_frustums[1]},
				{node, &m_frustums[2]}, {node, &m_frustums[3]},
				{node, &m_frustums[4]}, {node, &m_frustums[5]}}}
		{}

		Array<PerspectiveFrustum, 6> m_frustums;
		Array<FrustumComponent, 6> m_frustumComps;
		Array<Transform, 6> m_localTrfs;
	};

	Sphere m_sphereW = Sphere(Vec4(0.0), 1.0);
	ShadowData* m_shadowData = nullptr;
};

/// Spot light
class SpotLight: public Light, public FrustumComponent
{
public:
	SpotLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

	GlTextureHandle& getTexture()
	{
		return m_tex->getGlTexture();
	}

	const GlTextureHandle& getTexture() const
	{
		return m_tex->getGlTexture();
	}

	F32 getOuterAngle() const
	{
		return m_frustum.getFovX();
	}

	void setOuterAngle(F32 x)
	{
		m_frustum.setFovX(x);
		m_frustum.setFovY(x);
		m_cosOuterAngle = cos(x / 2.0);
		frustumUpdate();
	}

	F32 getOuterAngleCos() const
	{
		return m_cosOuterAngle;
	}

	void setInnerAngle(F32 ang)
	{
		m_cosInnerAngle = cos(ang / 2.0);
	}

	F32 getInnerAngleCos() const
	{
		return m_cosInnerAngle;
	}

	F32 getDistance() const
	{
		return m_frustum.getFar();
	}

	void setDistance(F32 f)
	{
		m_frustum.setFar(f);
		frustumUpdate();
	}

	const PerspectiveFrustum& getFrustum() const
	{
		return m_frustum;
	}

	/// @name MoveComponent virtuals
	/// @{
	ANKI_USE_RESULT Error onMoveComponentUpdate(SceneNode&, F32, F32) override;
	/// @}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return m_frustum;
	}
	/// @}

	ANKI_USE_RESULT Error loadTexture(const CString& filename);

private:
	PerspectiveFrustum m_frustum;
	TextureResourcePointer m_tex;
	F32 m_cosOuterAngle;
	F32 m_cosInnerAngle;
};

/// @}

} // end namespace anki

#endif

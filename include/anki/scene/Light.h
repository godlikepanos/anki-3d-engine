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

/// Light component. It's a dummy component used to identify lights
class LightComponent: public SceneComponent
{
public:
	LightComponent(Light* node);

	static constexpr Type getClassType()
	{
		return LIGHT_COMPONENT;
	}
};

/// XXX
class FlareBatch
{
public:
	static constexpr U MAX_FLARES = 10;

	enum FlareFlag
	{
		POSITION_LIGHT = 1 << 0,
		POSITION_FLOATING = 1 << 1
	};

private:
	/// A 2D array texture with the flare textures
	TextureResourcePointer flaresTex;

	/// The size of each flare
	Array<Vec2, MAX_FLARES> size;

	Array<Vec2, MAX_FLARES> stretchMultiplier;

	F32 flaresAlpha = 1.0;
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
	enum LightType
	{
		LT_POINT,
		LT_SPOT,
		LT_NUM
	};

	/// @name Constructors
	/// @{
	Light(
		const char* name, SceneGraph* scene, // SceneNode
		LightType t); // Self
	/// @}

	virtual ~Light();

	/// @name Accessors
	/// @{
	LightType getLightType() const
	{
		return type;
	}

	const Vec4& getDiffuseColor() const
	{
		return color;
	}
	Vec4& getDiffuseColor()
	{
		return color;
	}
	void setDiffuseColor(const Vec4& x)
	{
		color = x;
	}

	const Vec4& getSpecularColor() const
	{
		return specColor;
	}
	Vec4& getSpecularColor()
	{
		return specColor;
	}
	void setSpecularColor(const Vec4& x)
	{
		specColor = x;
	}

	Bool getShadowEnabled() const
	{
		return shadow;
	}
	void setShadowEnabled(const Bool x)
	{
		shadow = x;
	}

	U getShadowMapIndex() const
	{
		return (U)shadowMapIndex;
	}
	void setShadowMapIndex(const U i)
	{
		ANKI_ASSERT(i < 0xFF);
		shadowMapIndex = (U8)i;
	}

	Bool hasLensFlare() const
	{
		return flaresTex.isLoaded();
	}

	const GlTextureHandle& getLensFlareTexture() const
	{
		ANKI_ASSERT(hasLensFlare());
		return flaresTex->getGlTexture();
	}
	const Vec2& getLensFlaresSize() const
	{
		return flaresSize;
	}
	void setLensFlaresSize(const Vec2& val)
	{
		flaresSize = val;
	}
	const Vec2& getLensFlaresStretchMultiplier() const
	{
		return flaresStretchMultiplier;
	}
	void setLensFlaresStretchMultiplier(const Vec2& val)
	{
		flaresStretchMultiplier = val;
	}
	F32 getLensFlaresAlpha() const
	{
		return flaresAlpha;
	}
	void setLensFlaresAlpha(F32 val)
	{
		flaresAlpha = val;
	}
	/// @}

	void loadLensFlare(const char* filename)
	{
		ANKI_ASSERT(!hasLensFlare());
		flaresTex.load(filename);
	}

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
	void moveUpdate(MoveComponent& move);

private:
	LightType type;
	Vec4 color = Vec4(1.0);
	Vec4 specColor = Vec4(1.0);

	/// @name Flare struff
	/// @{
	TextureResourcePointer flaresTex;
	Vec2 flaresSize = Vec2(0.2);
	Vec2 flaresStretchMultiplier = Vec2(1.0);
	F32 flaresAlpha = 1.0;
	/// @}

	Bool8 shadow = false;
	U8 shadowMapIndex = 0xFF; ///< Used by the renderer
};

/// Point light
class PointLight: public Light
{
public:
	/// @name Constructors/Destructor
	/// @{
	PointLight(const char* name, SceneGraph* scene);
	/// @}

	/// @name Accessors
	/// @{
	F32 getRadius() const
	{
		return sphereW.getRadius();
	}
	void setRadius(const F32 x)
	{
		sphereW.setRadius(x);
		frustumUpdate();
	}

	const Sphere& getSphere() const
	{
		return sphereW;
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{
	void componentUpdated(SceneComponent& comp, 
		SceneComponent::UpdateType uptype) override;
	/// @}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return sphereW;
	}
	/// @}

public:
	Sphere sphereW = Sphere(Vec4(0.0), 1.0);
};

/// Spot light
class SpotLight: public Light, public FrustumComponent
{
public:
	/// @name Constructors/Destructor
	/// @{
	SpotLight(const char* name, SceneGraph* scene);
	/// @}

	/// @name Accessors
	/// @{
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
	/// @}

	/// @name SceneNode virtuals
	/// @{
	void componentUpdated(SceneComponent& comp,
		SceneComponent::UpdateType uptype) override;
	/// @}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return m_frustum;
	}
	/// @}

	void loadTexture(const char* filename)
	{
		m_tex.load(filename);
	}

private:
	PerspectiveFrustum m_frustum;
	TextureResourcePointer m_tex;
	F32 m_cosOuterAngle;
	F32 m_cosInnerAngle;
};

} // end namespace anki

#endif

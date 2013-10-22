#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/resource/Resource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

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
class Light: public SceneNode, public MoveComponent, public SpatialComponent
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
		CollisionShape* cs, // SpatialComponent
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

	const Texture& getLensFlareTexture() const
	{
		ANKI_ASSERT(hasLensFlare());
		return *flaresTex;
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

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::frameUpdate
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, int frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
	}
	/// @}

	void loadLensFlare(const char* filename)
	{
		ANKI_ASSERT(!hasLensFlare());
		flaresTex.load(filename);
	}

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
		SpatialComponent::markForUpdate();
	}

	const Sphere& getSphere() const
	{
		return sphereW;
	}
	/// @}

	/// @name MoveComponent virtuals
	/// @{

	/// Overrides MoveComponent::moveUpdate(). This does:
	/// - Update the collision shape
	void moveUpdate()
	{
		sphereW.setCenter(getWorldTransform().getOrigin());
		SpatialComponent::markForUpdate();
	}
	/// @}

public:
	Sphere sphereW = Sphere(Vec3(0.0), 1.0);
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
	Texture& getTexture()
	{
		return *tex;
	}
	const Texture& getTexture() const
	{
		return *tex;
	}

	F32 getOuterAngle() const
	{
		return frustum.getFovX();
	}
	void setOuterAngle(F32 x)
	{
		frustum.setFovX(x);
		frustum.setFovY(x);
		cosOuterAngle = cos(x / 2.0);
		frustumUpdate();
	}

	F32 getOuterAngleCos() const
	{
		return cosOuterAngle;
	}

	void setInnerAngle(F32 ang)
	{
		cosInnerAngle = cos(ang / 2.0);
	}
	F32 getInnerAngleCos() const
	{
		return cosInnerAngle;
	}

	F32 getDistance() const
	{
		return frustum.getFar();
	}
	void setDistance(F32 f)
	{
		frustum.setFar(f);
		frustumUpdate();
	}

	const PerspectiveFrustum& getFrustum() const
	{
		return frustum;
	}
	/// @}

	/// @name MoveComponent virtuals
	/// @{

	/// Overrides MoveComponent::moveUpdate(). This does:
	/// - Update the collision shape
	void moveUpdate()
	{
		frustum.setTransform(getWorldTransform());
		viewMat = Mat4(getWorldTransform().getInverse());
		viewProjectionMat = projectionMat * viewMat;

		SpatialComponent::markForUpdate();
	}
	/// @}

	/// @name FrustumComponent virtuals
	/// @{

	/// Override FrustumComponent::getFrustumOrigin()
	const Vec3& getFrustumOrigin() const
	{
		return getWorldTransform().getOrigin();
	}
	/// @}

	void loadTexture(const char* filename)
	{
		tex.load(filename);
	}

private:
	PerspectiveFrustum frustum;
	TextureResourcePointer tex;
	F32 cosOuterAngle;
	F32 cosInnerAngle;

	void frustumUpdate()
	{
		projectionMat = frustum.calculateProjectionMatrix();
		viewProjectionMat = projectionMat * viewMat;

		SpatialComponent::markForUpdate();
		FrustumComponent::markForUpdate();
	}
};

} // end namespace anki

#endif
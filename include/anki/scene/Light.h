#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Spatial.h"
#include "anki/resource/Resource.h"
#include "anki/resource/TextureResource.h"

namespace anki {

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
class Light: public SceneNode, public Movable, public Spatial
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
	Light(LightType t, // Light
		const char* name, SceneGraph* scene, // Scene
		U32 movableFlags, Movable* movParent, // Movable
		CollisionShape* cs); // Spatial
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

	bool getShadowEnabled() const
	{
		return shadow;
	}
	bool& getShadowEnabled()
	{
		return shadow;
	}
	void setShadowEnabled(const bool x)
	{
		shadow = x;
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

private:
	LightType type;
	Vec4 color = Vec4(1.0);
	Vec4 specColor = Vec4(1.0);
	bool shadow = false;
};

/// Point light
class PointLight: public Light
{
public:
	/// @name Constructors/Destructor
	/// @{
	PointLight(const char* name, SceneGraph* scene,
		U32 movableFlags, Movable* movParent);
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
		spatialMarkForUpdate();
	}

	const Sphere& getSphere() const
	{
		return sphereW;
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
		sphereW.setCenter(getWorldTransform().getOrigin());
		spatialMarkForUpdate();
	}
	/// @}

public:
	Sphere sphereW = Sphere(Vec3(0.0), 2.0);
};

/// Spot light
class SpotLight: public Light, public Frustumable
{
public:
	ANKI_HAS_SLOTS(SpotLight)

	/// @name Constructors/Destructor
	/// @{
	SpotLight(const char* name, SceneGraph* scene,
		U32 movableFlags, Movable* movParent);
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

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
		frustum.setTransform(getWorldTransform());
		viewMat = Mat4(getWorldTransform().getInverse());
		viewProjectionMat = projectionMat * viewMat;

		spatialMarkForUpdate();
	}
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Override Frustumable::getFrustumableOrigin()
	const Vec3& getFrustumableOrigin() const
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

		spatialMarkForUpdate();
		frustumableMarkUpdated();
	}
};

} // end namespace anki

#endif

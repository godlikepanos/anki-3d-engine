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
		const char* name, Scene* scene, // Scene
		uint32_t movableFlags, Movable* movParent, // Movable
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
	Movable* getMovable()
	{
		return this;
	}

	Spatial* getSpatial()
	{
		return this;
	}

	Light* getLight()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate
	void frameUpdate(float prevUpdateTime, float crntTime, int frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
	}
	/// @}

private:
	LightType type;
	Vec4 color = Vec4(1.0);
	Vec4 specColor = Vec4(1.0);
	bool shadow = true;
};

/// Point light
class PointLight: public Light
{
public:
	/// @name Constructors/Destructor
	/// @{
	PointLight(const char* name, Scene* scene,
		uint32_t movableFlags, Movable* movParent);
	/// @}

	/// @name Accessors
	/// @{
	float getRadius() const
	{
		return sphereW.getRadius();
	}
	void setRadius(const float x)
	{
		sphereW.setRadius(x);
		spatialMarkUpdated();
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
		spatialMarkUpdated();
	}
	/// @}

public:
	Sphere sphereW = Sphere(Vec3(0.0), 2.0);
};

/// Spot light
class SpotLight: public Light, public PerspectiveFrustumable
{
public:
	ANKI_HAS_SLOTS(SpotLight)

	/// @name Constructors/Destructor
	/// @{
	SpotLight(const char* name, Scene* scene,
		uint32_t movableFlags, Movable* movParent);
	/// @}

	/// @name Accessors
	/// @{
	const Mat4& getViewMatrix() const
	{
		return viewMat;
	}

	const Mat4& getProjectionMatrix() const
	{
		return projectionMat;
	}

	Texture& getTexture()
	{
		return *tex;
	}
	const Texture& getTexture() const
	{
		return *tex;
	}

	float getFov() const
	{
		return getFovX();
	}
	void setFov(float x)
	{
		setFovX(x);
		setFovY(x);
	}

	float getDistance() const
	{
		return getFar();
	}
	void setDistance(float f)
	{
		setFar(f);
	}

	const PerspectiveFrustum& getFrustum() const
	{
		return frustum;
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getFrustumable()
	Frustumable* getFrustumable()
	{
		return this;
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
		spatialMarkUpdated();
	}
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Overrides Frustumable::frustumUpdate(). Calculate the projection
	/// matrix
	void frustumUpdate()
	{
		Frustumable::frustumUpdate();
		projectionMat = frustum.calculateProjectionMatrix();
		spatialMarkUpdated();
	}
	/// @}

	void loadTexture(const char* filename)
	{
		tex.load(filename);
	}

private:
	PerspectiveFrustum frustum;
	Mat4 projectionMat;
	Mat4 viewMat;
	TextureResourcePointer tex;
	ReadWriteProperty<float>* fovProp; ///< Have it here for updates
	ReadWriteProperty<float>* distProp; ///< Have it here for updates

	void updateFar(const float& f)
	{
		frustum.setFar(f);
	}
	ANKI_SLOT(updateFar, const float&)

	void updateFov(const float& f)
	{
		frustum.setFovX(f);
		frustum.setFovY(f);
	}
	ANKI_SLOT(updateFov, const float&)
};

} // end namespace anki

#endif

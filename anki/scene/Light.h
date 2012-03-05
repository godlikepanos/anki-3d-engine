#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Spatial.h"
#include "anki/resource/Material.h"
#include "anki/resource/Resource.h"


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
class Light: public SceneNode, public Movable, public Renderable,
	public Spatial
{
public:
	enum LightType
	{
		LT_POINT,
		LT_SPOT
	};

	/// @name Constructors
	/// @{
	Light(LightType t, const char* mtl, // Light
		const char* name, Scene* scene, // Scene
		uint movableFlags, Movable* movParent, // Movable
		CollisionShape* cs); // Spatial
	/// @}

	virtual ~Light();

	/// @name Accessors
	/// @{
	LightType getLightType() const
	{
		return type;
	}
	/// @}

	/// @name SceneNode virtuals
	/// @{
	Movable* getMovable()
	{
		return this;
	}

	Renderable* getRenderable()
	{
		return this;
	}

	Spatial* getSpatial()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate
	void frameUpdate(float prevUpdateTime, float crntTime, int frame)
	{
		SceneNode::frameUpdate(prevUpdateTime, crntTime, frame);
		Movable::update();
	}
	/// @}

	/// @name Renderable virtuals
	/// @{
	const ModelPatchBase& getModelPatchBase() const
	{
		ANKI_ASSERT(0 && "Lights don't support it");
		throw ""; // Make the compiler not to complain about return val
	}

	Material& getMaterial()
	{
		return *mtl;
	}

	Light* getLight()
	{
		return this;
	}
	/// @}

private:
	LightType type;
	MaterialResourcePointer mtl;
};


/// Point light
class PointLight: public Light
{
public:
	ANKI_OBSERVING(PointLight)

	/// @name Constructors/Destructor
	/// @{
	PointLight(const char* fmtl,
		const char* name, Scene* scene,
		uint movableFlags, Movable* movParent);
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
		sphereW = sphereL.getTransformed(getWorldTransform());
	}
	/// @}

public:
	Sphere sphereW;
	Sphere sphereL;

	void updateRadius(const float& r)
	{
		sphereL.setRadius(r);
	}
	ANKI_SLOT(updateRadius, const float&)
};


/// Spot light
class SpotLight: public Light, public Frustumable
{
public:
	ANKI_OBSERVING(SpotLight)

	/// @name Constructors/Destructor
	/// @{
	SpotLight(const char* fmtl,
		const char* name, Scene* scene,
		uint movableFlags, Movable* movParent);
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::moveUpdate(). This does:
	/// - Update the collision shape
	void movableUpdate()
	{
		Movable::movableUpdate();
		frustum.transform(getWorldTransform());
		viewMat = Mat4(getWorldTransform().getInverse());
	}
	/// @}

	/// @name Frustumable virtuals
	/// @{

	/// Implements Frustumable::frustumUpdate(). Calculate the projection
	/// matrix
	void frustumUpdate()
	{
		projectionMat = getFrustum().calculateProjectionMatrix();
	}
	/// @}

private:
	PerspectiveFrustum frustum;
	Mat4 projectionMat;
	Mat4 viewMat;

	void updateZFar(const float& f)
	{
		frustum.setZFar(r);
	}
	ANKI_SLOT(updateZFar, const float&)

	void updateFov(const float& f)
	{
		frustum.setFovX(f);
		frustum.setFovY(f);
	}
	ANKI_SLOT(updateFov, const float&)
};


} // end namespace


#endif

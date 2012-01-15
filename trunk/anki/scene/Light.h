#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Spartial.h"


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
	public Spartial
{
public:
	enum LightType
	{
		LT_POINT,
		LT_SPOT
	};

	/// @name Constructors
	/// @{
	Light(LightType t,
		const char* name, Scene* scene,
		uint movableFlags, Movable* movParent,
		CollisionShape* cs,
		const char* modelFname)
		: SceneNode(name, scene), Movable(movableFlags, movParent),
			Spartial(cs), type(t)
	{
		//smoModel
	}
	/// @}

	virtual ~Light();

	/// @name Accessors
	/// @{
	LightType getLightType() const
	{
		return type;
	}
	/// @}

	/// @name Virtuals
	/// @{
	Movable* getMovable()
	{
		return this;
	}

	Renderable* getRenderable()
	{
		return this;
	}

	Spartial* getSpartial()
	{
		return this;
	}

	const Vao& getVao(const PassLevelKey&)
	{
		//mesh.
	}

	MaterialRuntime& getMaterialRuntime()
	{
		return *mtlr;
	}

	const Transform& getWorldTransform(const PassLevelKey&)
	{
		return getWorldTransform();
	}

	const Transform& getPreviousWorldTransform(const PassLevelKey&)
	{
		return getPreviousWorldTransform();
	}
	/// @}

private:
	LightType type;
	boost::scoped_ptr<MaterialRuntime> mtlr;
	ModelResourcePointer smoModel; ///< Model for SMO pases
};


/// Point light
class PointLight: public Light
{
public:

private:
};


/// Spot light
class SpotLight: public Light, public Frustumable
{
public:

private:
};


} // end namespace


#endif

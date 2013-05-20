#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(
	const char* name, SceneGraph* scene, SceneNode* parent, // Scene
	U32 movableFlags, // Movable
	CollisionShape* cs, // Spatial
	LightType t) // Self
	:	SceneNode(name, scene, parent),
		Movable(movableFlags, this),
		Spatial(cs, getSceneAllocator()),
		type(t)
{
	sceneNodeProtected.movable = this;
	sceneNodeProtected.spatial = this;
	sceneNodeProtected.light = this;

	addNewProperty(new ReadWritePointerProperty<Vec4>("color", &color));

	addNewProperty(
		new ReadWritePointerProperty<Vec4>("specularColor", &specColor));

	addNewProperty(new ReadWritePointerProperty<bool>("shadow", &shadow));
}

//==============================================================================
Light::~Light()
{}

//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(
	const char* name, SceneGraph* scene, SceneNode* parent,
	U32 movableFlags)
	: Light(name, scene, parent, movableFlags, &sphereW, LT_POINT)
{
	F32& r = sphereW.getRadius();
	addNewProperty(new ReadWritePointerProperty<F32>("radius", &r));
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(
	const char* name, SceneGraph* scene, SceneNode* parent,
	U32 movableFlags)
	: 	Light(name, scene, parent, movableFlags, &frustum, LT_SPOT),
		Frustumable(&frustum)
{
	sceneNodeProtected.frustumable = this;

	const F32 ang = toRad(45.0);
	setOuterAngle(ang / 2.0);
	const F32 dist = 1.0;

	// Fix frustum
	//
	frustum.setAll(ang, ang, 0.1, dist);
	frustumUpdate();
}

} // end namespace anki

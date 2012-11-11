#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(LightType t, // Light
	const char* name, Scene* scene, // Scene
	U32 movableFlags, Movable* movParent, // Movable
	CollisionShape* cs) // Spatial
	: SceneNode(name, scene),
		Movable(movableFlags, movParent, *this),
		Spatial(this, cs), type(t)
{
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
PointLight::PointLight(const char* name, Scene* scene,
	U32 movableFlags, Movable* movParent)
	: Light(LT_POINT, name, scene, movableFlags, movParent, &sphereW)
{
	F32& r = sphereW.getRadius();
	addNewProperty(new ReadWritePointerProperty<F32>("radius", &r));
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const char* name, Scene* scene,
	U32 movableFlags, Movable* movParent)
	: Light(LT_SPOT, name, scene, movableFlags, movParent, &frustum),
		Frustumable(&frustum)
{
	const F32 ang = toRad(45.0);
	setOuterAngle(ang / 2.0);
	const F32 dist = 1.0;

	// Fix frustum
	//
	frustum.setAll(ang, ang, 0.1, dist);
	frustumUpdate();
}

} // end namespace anki

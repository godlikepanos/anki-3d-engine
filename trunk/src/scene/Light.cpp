#include "anki/scene/Light.h"
#include "anki/resource/LightRsrc.h"

namespace anki {

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(LightType t, // Light
	const char* name, Scene* scene, // Scene
	uint32_t movableFlags, Movable* movParent, // Movable
	CollisionShape* cs) // Spatial
	: SceneNode(name, scene),
		Movable(movableFlags, movParent, *this),
		Spatial(cs), type(t)
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
	uint32_t movableFlags, Movable* movParent)
	: Light(LT_POINT, name, scene, movableFlags, movParent, &sphereW)
{
	sphereL.setCenter(Vec3(0.0));
	sphereL.setRadius(1.0);

	float& r = sphereL.getRadius();
	addNewProperty(new ReadWritePointerProperty<float>("radius", &r));
}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const char* name, Scene* scene,
	uint32_t movableFlags, Movable* movParent)
	: Light(LT_SPOT, name, scene, movableFlags, movParent, &frustum),
		Frustumable(&frustum)
{
	// Fov
	//
	float ang = Math::toRad(45.0);
	fovProp = new ReadWriteProperty<float>("fov", ang);
	addNewProperty(fovProp);
	ANKI_CONNECT(fovProp, valueChanged, this, updateFov);

	// Distance
	//
	float dist = 10.0;
	distProp = new ReadWriteProperty<float>("distance", dist);
	addNewProperty(distProp);
	ANKI_CONNECT(distProp, valueChanged, this, updateZFar);

	// Fix frustum
	//
	frustum.setAll(ang, ang, 0.1, dist);
}

} // end namespace

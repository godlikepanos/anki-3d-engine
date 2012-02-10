#include "anki/scene/Light.h"
#include "anki/resource/LightRsrc.h"


namespace anki {


//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(LightType t, const char* fmtl, // Light
	const char* name, Scene* scene, // Scene
	uint movableFlags, Movable* movParent, // Movable
	CollisionShape* cs) // Spatial
	: SceneNode(name, scene),
		Movable(movableFlags, movParent, *this),
		Spatial(cs), type(t)
{
	mtl.load(fmtl);
	Renderable::init(*this);
}


//==============================================================================
Light::~Light()
{}


//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(const char* fmtl,
	const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Light(LT_POINT, fmtl, name, scene, movableFlags, movParent, &sphereW)
{
	PropertyBase& pbase = findPropertyBaseByName("radius");
	Property<float>& prop = pbase.upCast<Property<float> >();
	ANKI_CONNECT(&prop, valueChanged, this, updateRadius);

	sphereL.setCenter(Vec3(0.0));
	sphereL.setRadius(prop.getValue());
}


//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const char* fmtl,
	const char* name, Scene* scene,
	uint movableFlags, Movable* movParent)
	: Light(LT_SPOT, fmtl, name, scene, movableFlags, movParent, &frustum),
		Frustumable(&frustum)
{}


} // end namespace

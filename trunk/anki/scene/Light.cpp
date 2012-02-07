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
	: SceneNode(name, scene), Movable(movableFlags, movParent),
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
	Property<PerspectiveFrustum>* prop =
		new ReadWritePointerProperty<PerspectiveFrustum>("frustum", &frustum);

	addNewProperty(prop);

	ANKI_CONNECT(prop, valueChanged, this, updateFrustumSlot);
}


} // end namespace

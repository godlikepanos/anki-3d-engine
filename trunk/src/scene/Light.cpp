#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
// Light                                                                       =
//==============================================================================

//==============================================================================
Light::Light(
	const char* name, SceneGraph* scene, // SceneNode
	CollisionShape* cs, // Spatial
	LightType t) // Self
	:	SceneNode(name, scene),
		MoveComponent(this),
		SpatialComponent(this, cs),
		type(t)
{
	sceneNodeProtected.moveC = this;
	sceneNodeProtected.spatialC = this;
	sceneNodeProtected.lightC = this;
}

//==============================================================================
Light::~Light()
{}

//==============================================================================
// PointLight                                                                  =
//==============================================================================

//==============================================================================
PointLight::PointLight(const char* name, SceneGraph* scene)
	:	Light(name, scene, &sphereW, LT_POINT)
{}

//==============================================================================
// SpotLight                                                                   =
//==============================================================================

//==============================================================================
SpotLight::SpotLight(const char* name, SceneGraph* scene)
	:	Light(name, scene, &frustum, LT_SPOT),
		FrustumComponent(&frustum)
{
	sceneNodeProtected.frustumC = this;

	const F32 ang = toRad(45.0);
	setOuterAngle(ang / 2.0);
	const F32 dist = 1.0;

	// Fix frustum
	//
	frustum.setAll(ang, ang, 0.1, dist);
	frustumUpdate();
}

} // end namespace anki

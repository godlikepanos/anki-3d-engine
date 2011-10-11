#ifndef ANKI_SCENE_POINT_LIGHT_H
#define ANKI_SCENE_POINT_LIGHT_H

#include "anki/scene/Light.h"
#include "anki/collision/Sphere.h"


/// Point light. Defined by its radius
class PointLight: public Light
{
	public:
		PointLight(Scene& scene, ulong flags, SceneNode* parent);

		/// @name Accessors
		/// @{
		float getRadius() const
		{
			return radius;
		}
		void setRadius(float x)
		{
			radius = x;
			lspaceCShape = Sphere(Vec3(0.0), radius);
		}
		/// @}

		void init(const char* filename);

		/// @copydoc SceneNode::getVisibilityCollisionShapeWorldSpace
		const CollisionShape*
			getVisibilityCollisionShapeWorldSpace() const
		{
			return &wspaceCShape;
		}

		void moveUpdate()
		{
			wspaceCShape = lspaceCShape.getTransformed(getWorldTransform());
		}

	private:
		float radius;
		Sphere lspaceCShape;
		Sphere wspaceCShape;
};


inline PointLight::PointLight(Scene& scene, ulong flags, SceneNode* parent)
:	Light(LT_POINT, scene, flags, parent)
{}


inline void PointLight::init(const char* filename)
{
	Light::init(filename);
	if(lightData->getType() != LightRsrc::LT_POINT)
	{
		throw ANKI_EXCEPTION("Light data is wrong type");
	}
	radius = lightData->getRadius();
	lspaceCShape = Sphere(Vec3(0.0), radius);
}


#endif

#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Light.h"


/// Point light. Defined by its radius
class PointLight: public Light
{
	public:
		PointLight(bool inheritParentTrfFlag, SceneNode* parent);
		GETTER_SETTER(float, radius, getRadius, setRadius)
		void init(const char* filename);

	private:
		float radius;
};


inline PointLight::PointLight(bool inheritParentTrfFlag, SceneNode* parent)
:	Light(LT_POINT, inheritParentTrfFlag, parent)
{}


inline void PointLight::init(const char* filename)
{
	Light::init(filename);
	if(lightData->getType() != LightRsrc::LT_POINT)
	{
		throw EXCEPTION("Light data is wrong type");
	}
	radius = lightData->getRadius();
}


#endif

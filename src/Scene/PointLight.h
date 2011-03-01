#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Light.h"


/// Point light. Defined by its radius
class PointLight: public Light
{
	public:
		PointLight(SceneNode* parent = NULL): Light(LT_POINT, parent) {}
		GETTER_SETTER(float, radius, getRadius, setRadius)
		void init(const char* filename);

	private:
		float radius;
};


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

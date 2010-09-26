#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Common.h"
#include "Light.h"


/// Point light. Defined by its radius
class PointLight: public Light
{
	PROPERTY_RW(float, radius, setRadius, getRadius)

	public:
		PointLight(SceneNode* parent = NULL): Light(LT_POINT, parent) {}
		void init(const char* filename);
};


inline void PointLight::init(const char* filename)
{
	Light::init(filename);
	if(lightData->getType() != LightData::LT_POINT)
	{
		ERROR("Light data is wrong type");
		return;
	}
	radius = lightData->getRadius();
}


#endif

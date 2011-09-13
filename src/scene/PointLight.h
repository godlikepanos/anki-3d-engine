#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Light.h"


/// Point light. Defined by its radius
class PointLight: public Light
{
	public:
		PointLight(Scene& scene, ulong flags, SceneNode* parent);

		float getRadius() const {return radius;}
		float& getRadius() {return radius;}
		void setRadius(float x) {radius = x;}

		void init(const char* filename);

	private:
		float radius;
};


inline PointLight::PointLight(Scene& scene, ulong flags, SceneNode* parent)
:	Light(LT_POINT, scene, flags, parent)
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

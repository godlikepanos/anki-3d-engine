#include "SpotLight.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void SpotLight::init(const char* filename)
{
	Light::init(filename);
	if(lightData->getType() != LightData::LT_SPOT)
	{
		ERROR("Light data is wrong type");
		return;
	}
	camera = new Camera(this);
	camera->setAll(lightData->getFovX(), lightData->getFovY(), 0.02, lightData->getDistance());
}

#include "SpotLight.h"


//==============================================================================
// init                                                                        =
//==============================================================================
void SpotLight::init(const char* filename)
{
	Light::init(filename);
	if(lightData->getType() != LightRsrc::LT_SPOT)
	{
		throw EXCEPTION("Light data is wrong type");
	}
	camera = new PerspectiveCamera(getScene(), SNF_NONE, this);
	camera->setAll(lightData->getFovX(), lightData->getFovY(), 0.02,
		lightData->getDistance());
}

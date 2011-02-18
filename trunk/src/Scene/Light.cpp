#include "Light.h"
#include "LightRsrc.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Light::init(const char* filename)
{
	lightData.loadRsrc(filename);

	diffuseCol = lightData->getDiffuseCol();
	specularCol = lightData->getSpecularCol();
	castsShadow_ = lightData->castsShadow();
}

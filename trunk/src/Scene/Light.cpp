#include "Light.h"
#include "Resources/LightRsrc.h"


//==============================================================================
// init                                                                        =
//==============================================================================
void Light::init(const char* filename)
{
	lightData.loadRsrc(filename);

	diffuseCol = lightData->getDiffuseCol();
	specularCol = lightData->getSpecularCol();
	castsShadowFlag = lightData->castsShadow();
}

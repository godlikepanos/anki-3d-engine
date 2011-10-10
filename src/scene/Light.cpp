#include "Light.h"
#include "resource/LightRsrc.h"


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Light::~Light()
{}


//==============================================================================
// init                                                                        =
//==============================================================================
void Light::init(const char* filename)
{
	lightData.loadRsrc(filename);

	diffuseCol = lightData->getDiffuseColor();
	specularCol = lightData->getSpecularColor();
	castsShadowFlag = lightData->getCastShadow();
}

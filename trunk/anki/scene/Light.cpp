#include "anki/scene/Light.h"
#include "anki/resource/LightRsrc.h"


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

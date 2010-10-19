#include "Light.h"
#include "LightData.h"
#include "App.h"
#include "MainRenderer.h"


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


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void Light::render()
{
	app->getMainRenderer().dbg.drawSphere(0.1, getWorldTransform(), Vec4(lightData->getDiffuseCol(), 1.0));
}


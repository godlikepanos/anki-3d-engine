#include "Light.h"
#include "collision.h"
#include "LightData.h"
#include "App.h"
#include "MainRenderer.h"


//======================================================================================================================
// init [PointLight]                                                                                                  =
//======================================================================================================================
void PointLight::init(const char* filename)
{
	lightData.loadRsrc(filename);
	radius = lightData->getRadius();
}


//======================================================================================================================
// init [SpotLight]                                                                                                    =
//======================================================================================================================
void SpotLight::init(const char* filename)
{
	lightData.loadRsrc(filename);
	camera->setAll(lightData->getFovX(), lightData->getFovY(), 0.2, lightData->getDistance());
	castsShadow = lightData->castsShadow();

	if(lightData->getTexture() == NULL)
	{
		ERROR("Light properties \"" << lightData->getRsrcName() << "\" do not have a texture");
		return;
	}
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void Light::render()
{
	app->getMainRenderer().dbg.drawSphere(0.1, getWorldTransform(), Vec4(lightData->getDiffuseColor(), 1.0));
	//Dbg::drawSphere(0.1, Transform::getIdentity(), Vec4(lightProps->getDiffuseColor(), 1.0));
}


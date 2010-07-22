#include "Light.h"
#include "collision.h"
#include "LightProps.h"
#include "App.h"
#include "MainRenderer.h"


//======================================================================================================================
// init [PointLight]                                                                                                  =
//======================================================================================================================
void PointLight::init(const char* filename)
{
	lightProps.loadRsrc(filename);
	radius = lightProps->getRadius();
}


//======================================================================================================================
// init [SpotLight]                                                                                                    =
//======================================================================================================================
void SpotLight::init(const char* filename)
{
	lightProps.loadRsrc(filename);
	camera.setAll(lightProps->getFovX(), lightProps->getFovY(), 0.2, lightProps->getDistance());
	castsShadow = lightProps->castsShadow();
	if(lightProps->getTexture() == NULL)
	{
		ERROR("Light properties \"" << lightProps->getRsrcName() << "\" do not have a texture");
		return;
	}
}


//======================================================================================================================
// deinit                                                                                                              =
//======================================================================================================================
void Light::deinit()
{
	//RsrcMngr::lightProps.unload(lightProps);
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void Light::render()
{
	Renderer::Dbg::setColor(Vec4(lightProps->getDiffuseColor(), 1.0));
	Renderer::Dbg::setModelMat(Mat4(getWorldTransform()));
	Renderer::Dbg::drawSphere(8, 0.1);
}


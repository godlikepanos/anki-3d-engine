#include "Light.h"
#include "collision.h"
#include "LightProps.h"
#include "App.h"
#include "MainRenderer.h"


//======================================================================================================================
// init [PointLight]                                                                                                =
//======================================================================================================================
void PointLight::init( const char* filename )
{
	lightProps = Rsrc::lightProps.load( filename );
	radius = lightProps->getRadius();
}


//======================================================================================================================
// init [SpotLight]                                                                                                 =
//======================================================================================================================
void SpotLight::init( const char* filename )
{
	lightProps = Rsrc::lightProps.load( filename );
	camera.setAll( lightProps->getFovX(), lightProps->getFovY(), 0.2, lightProps->getDistance() );
	castsShadow = lightProps->castsShadow();
	if( lightProps->getTexture() == NULL )
	{
		ERROR( "Light properties \"" << lightProps->getRsrcName() << "\" do not have a texture" );
		return;
	}
}


//======================================================================================================================
// deinit                                                                                                              =
//======================================================================================================================
void Light::deinit()
{
	Rsrc::lightProps.unload( lightProps );
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void PointLight::render()
{
	app->getMainRenderer()->dbg.renderSphere( getWorldTransform(), Vec4( lightProps->getDiffuseColor(), 1.0 ), 8 );
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void SpotLight::render()
{
	app->getMainRenderer()->dbg.renderSphere( getWorldTransform(), Vec4( lightProps->getDiffuseColor(), 1.0 ), 8 );
}

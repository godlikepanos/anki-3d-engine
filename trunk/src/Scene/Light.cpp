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
// renderSphere                                                                                                        =
//======================================================================================================================
static void renderSphere( const Mat4& tsl, const Vec3& col )
{
	glPushMatrix();

	/// @todo move to GL 3.x
	app->getMainRenderer()->multMatrix( tsl );
	app->getMainRenderer()->dbg.renderSphere( 1.0/8.0, 8, col );

	glPopMatrix();
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void PointLight::render()
{
	renderSphere( Mat4(getWorldTransform()), lightProps->getDiffuseColor() );
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void SpotLight::render()
{
	renderSphere( Mat4(getWorldTransform()), lightProps->getDiffuseColor() );
}

#include "Controller.h"
#include "Scene.h"
#include "App.h"

Controller::Controller( Type type_ ): 
	type(type_) 
{
	DEBUG_ERR( app->getScene() == NULL );
	app->getScene()->registerController( this );
}

Controller::~Controller()
{
	DEBUG_ERR( app->getScene() == NULL );
	app->getScene()->unregisterController( this );
}

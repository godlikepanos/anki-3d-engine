#include "Controller.h"
#include "Scene.h"
#include "App.h"

Controller::Controller( Type type_ ): 
	type(type_) 
{
	DEBUG_ERR( app->scene == NULL );
	app->scene->registerController( this );
}

Controller::~Controller()
{
	DEBUG_ERR( app->scene == NULL );
	app->scene->unregisterController( this );
}

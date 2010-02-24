#include "Controller.h"
#include "Scene.h"

Controller::Controller( Type type_ ): 
	type(type_) 
{
	scene::registerController( this );
}

Controller::~Controller()
{
	scene::unregisterController( this );
}

#include "Controller.h"
#include "Scene.h"

Controller::Controller( Type type_ ): 
	type(type_) 
{
	Scene::registerController( this );
}

Controller::~Controller()
{
	Scene::unregisterController( this );
}

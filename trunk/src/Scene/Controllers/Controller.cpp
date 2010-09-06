#include "Controller.h"
#include "Scene.h"
#include "App.h"

Controller::Controller(Type type_): 
	type(type_) 
{
	app->getScene().registerController(this);
}

Controller::~Controller()
{
	app->getScene().unregisterController(this);
}

#include "Controller.h"
#include "Scene.h"
#include "App.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Controller::Controller(ControllerType type_):
	type(type_) 
{
	AppSingleton::getInstance().getScene().registerController(this);
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
Controller::~Controller()
{
	AppSingleton::getInstance().getScene().unregisterController(this);
}

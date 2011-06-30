#include "Controller.h"
#include "Scene.h"
#include "App.h"
#include "Core/Globals.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Controller::Controller(ControllerType type_, SceneNode& node):
	controlledNode(node),
	type(type_)
{
	SceneSingleton::getInstance().registerController(this);
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Controller::~Controller()
{
	SceneSingleton::getInstance().unregisterController(this);
}

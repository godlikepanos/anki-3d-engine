#include "Controller.h"
#include "Scene.h"
#include "core/App.h"
#include "core/Globals.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Controller::Controller(ControllerType type_, SceneNode& node):
	controlledNode(node),
	type(type_)
{
	SceneSingleton::get().registerController(this);
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Controller::~Controller()
{
	SceneSingleton::get().unregisterController(this);
}

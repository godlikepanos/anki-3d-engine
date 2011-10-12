#include "anki/scene/Controller.h"
#include "anki/scene/Scene.h"
#include "anki/core/App.h"
#include "anki/core/Globals.h"


namespace anki {


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


} // end namespace

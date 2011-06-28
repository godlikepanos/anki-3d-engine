#include "ScriptingCommon.h"
#include "EventManager.h"

#include "EventSceneColor.h"
#include "EventMainRendererPpsHdr.h"


WRAP(EventManager)
{
	class_<Event::Manager, noncopyable>("EventManager", no_init)
		.def("createEvent", &Event::Manager::createEvent<Event::SceneColor>,
			return_value_policy<reference_existing_object>())
		.def("createEvent", &Event::Manager::createEvent<Event::MainRendererPpsHdr>,
			return_value_policy<reference_existing_object>())
	;
}

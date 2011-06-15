#include "ScriptingCommon.h"
#include "EventManager.h"

#include "EventSceneColor.h"
#include "EventMainRendererPpsHdr.h"


WRAP(EventManager)
{
	class_<Event::Manager, noncopyable>("EventManager", no_init)
		.def("createEvent", &Event::Manager::createEvent<Event::SceneColor>)
		.def("createEvent", &Event::Manager::createEvent<Event::MainRendererPpsHdr>)
	;
}

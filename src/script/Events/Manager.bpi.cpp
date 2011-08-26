#include "ScriptingCommon.h"
#include "event/Manager.h"

#include "event/SceneColor.h"
#include "event/MainRendererPpsHdr.h"


WRAP(EventManager)
{
	using namespace event;

	class_<Manager, noncopyable>("EventManager", no_init)
		.def("createEvent", &Manager::createEvent<SceneColor>,
			return_value_policy<reference_existing_object>())
		.def("createEvent",
			&Manager::createEvent<MainRendererPpsHdr>,
			return_value_policy<reference_existing_object>())
	;
}

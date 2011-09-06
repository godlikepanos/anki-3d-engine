#include "ScriptCommon.h"
#include "event/EventManager.h"

#include "event/SceneColorEvent.h"
#include "event/MainRendererPpsHdrEvent.h"


WRAP(EventManager)
{
	class_<EventManager, noncopyable>("EventManager", no_init)
		.def("createEvent", &EventManager::createEvent<SceneColorEvent>,
			return_value_policy<reference_existing_object>())
		.def("createEvent",
			&EventManager::createEvent<MainRendererPpsHdrEvent>,
			return_value_policy<reference_existing_object>())
	;
}

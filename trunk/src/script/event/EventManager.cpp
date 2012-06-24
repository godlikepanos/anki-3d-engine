#include "anki/script/Common.h"
#include "anki/event/EventManager.h"

#include "anki/event/SceneColorEvent.h"
#include "anki/event/MainRendererPpsHdrEvent.h"

ANKI_WRAP(EventManager)
{
	class_<EventManager, noncopyable>("EventManager", no_init)
		.def("createEvent", &EventManager::createEvent<SceneColorEvent>,
			return_value_policy<reference_existing_object>())
		.def("createEvent",
			&EventManager::createEvent<MainRendererPpsHdrEvent>,
			return_value_policy<reference_existing_object>())
	;
}

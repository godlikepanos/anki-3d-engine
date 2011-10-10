#include "anki/script/ScriptCommon.h"
#include "anki/event/MainRendererPpsHdrEvent.h"


WRAP(MainRendererPpsHdrEvent)
{
	class_<MainRendererPpsHdrEvent>("MainRendererPpsHdrEvent", no_init)
		.def(init<float, float, float, uint, float>())
		.def(init<const MainRendererPpsHdrEvent&>())
	;
}

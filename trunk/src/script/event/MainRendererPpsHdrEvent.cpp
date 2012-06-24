#include "anki/script/Common.h"
#include "anki/event/MainRendererPpsHdrEvent.h"

ANKI_WRAP(MainRendererPpsHdrEvent)
{
	class_<MainRendererPpsHdrEvent>("MainRendererPpsHdrEvent", no_init)
		.def(init<float, float, float, uint, float>())
		.def(init<const MainRendererPpsHdrEvent&>())
	;
}

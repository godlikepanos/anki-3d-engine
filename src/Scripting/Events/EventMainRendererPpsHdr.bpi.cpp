#include "ScriptingCommon.h"
#include "EventMainRendererPpsHdr.h"


WRAP(EventMainRendererPpsHdr)
{
	class_<Event::MainRendererPpsHdr>("EventMainRendererPpsHdr", no_init)
		.def(init<uint, int, float, uint, float>())
		.def(init<const Event::MainRendererPpsHdr&>())
	;
}

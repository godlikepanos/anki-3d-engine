#include "ScriptingCommon.h"
#include "EventMainRendererPpsHdr.h"


WRAP(EventMainRendererPpsHdr)
{
	class_<Event::MainRendererPpsHdr>("EventMainRendererPpsHdr", no_init)
		.def(init<uint, uint, float, uint, float>())
		.def(init<const Event::MainRendererPpsHdr&>())
	;
}

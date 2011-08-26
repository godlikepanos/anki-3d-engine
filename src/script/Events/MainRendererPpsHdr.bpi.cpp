#include "ScriptingCommon.h"
#include "event/MainRendererPpsHdr.h"


WRAP(EventMainRendererPpsHdr)
{
	using namespace event;

	class_<MainRendererPpsHdr>("EventMainRendererPpsHdr", no_init)
		.def(init<float, float, float, uint, float>())
		.def(init<const MainRendererPpsHdr&>())
	;
}

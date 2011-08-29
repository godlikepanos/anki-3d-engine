#include "Common.h"
#include "event/MainRendererPpsHdr.h"


WRAP(EventMainRendererPpsHdr)
{
	using namespace event;

	class_<MainRendererPpsHdr>("event_MainRendererPpsHdr", no_init)
		.def(init<float, float, float, uint, float>())
		.def(init<const MainRendererPpsHdr&>())
	;
}

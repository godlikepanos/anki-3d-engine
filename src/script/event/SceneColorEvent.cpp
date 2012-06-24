#include "anki/script/Common.h"
#include "anki/event/SceneColorEvent.h"

ANKI_WRAP(SceneColorEvent)
{
	class_<SceneColorEvent>("SceneColorEvent", no_init)
		.def(init<float, float, const Vec3&>())
		.def(init<const SceneColorEvent&>())
	;
}

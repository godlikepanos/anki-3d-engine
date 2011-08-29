#include "Common.h"
#include "event/SceneColor.h"


WRAP(EventSceneColor)
{
	using namespace event;

	class_<SceneColor>("event_EventSceneColor", no_init)
		.def(init<float, float, const Vec3&>())
		.def(init<const SceneColor&>())
	;
}

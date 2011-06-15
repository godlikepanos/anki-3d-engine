#include "ScriptingCommon.h"
#include "EventSceneColor.h"


WRAP(EventSceneColor)
{
	class_<Event::SceneColor>("EventSceneColor", no_init)
		.def(init<uint, int, const Vec3&>())
		.def(init<const Event::SceneColor&>())
	;
}

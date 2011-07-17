#include "ScriptingCommon.h"
#include "Events/SceneColor.h"


WRAP(EventSceneColor)
{
	class_<Event::SceneColor>("EventSceneColor", no_init)
		.def(init<float, float, const Vec3&>())
		.def(init<const Event::SceneColor&>())
	;
}

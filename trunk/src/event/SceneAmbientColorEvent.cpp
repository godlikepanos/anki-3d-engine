#include "anki/event/SceneAmbientColorEvent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
SceneAmbientColorEvent::SceneAmbientColorEvent(F32 startTime, F32 duration, 
	EventManager* manager, const Vec3& finalColor_, SceneGraph* scene_)
	:	Event(startTime, duration, manager), 
		finalColor(finalColor_), 
		scene(scene_)
{
	ANKI_ASSERT(scene);
	originalColor = scene->getAmbientColor();
}

//==============================================================================
void SceneAmbientColorEvent::update(F32 /*prevUpdateTime*/, F32 crntTime)
{
	scene->setAmbientColor(
		interpolate(originalColor, finalColor, getDelta(crntTime)));
}

} // end namespace anki

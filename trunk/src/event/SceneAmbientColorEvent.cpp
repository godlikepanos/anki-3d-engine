#include "anki/event/SceneAmbientColorEvent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
SceneAmbientColorEvent::SceneAmbientColorEvent(EventManager* manager,
	F32 startTime, F32 duration, const Vec4& finalColor_)
	:	Event(manager, startTime, duration), 
		finalColor(finalColor_)
{
	originalColor = getEventManager().getScene().getAmbientColor();
}

//==============================================================================
void SceneAmbientColorEvent::update(F32 /*prevUpdateTime*/, F32 crntTime)
{
	getEventManager().getScene().setAmbientColor(
		interpolate(originalColor, finalColor, getDelta(crntTime)));
}

} // end namespace anki

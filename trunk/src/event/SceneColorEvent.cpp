#include "anki/event/SceneColorEvent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
SceneColorEvent::SceneColorEvent(float startTime, float duration,
	const Vec3& finalColor_)
	: Event(ET_SCENE_COLOR, startTime, duration), finalColor(finalColor_)
{
	originalColor = SceneGraphSingleton::get().getAmbientColor();
}

//==============================================================================
SceneColorEvent::SceneColorEvent(const SceneColorEvent& b)
	: Event(ET_SCENE_COLOR, 0.0, 0.0)
{
	*this = b;
}

//==============================================================================
SceneColorEvent& SceneColorEvent::operator=(const SceneColorEvent& b)
{
	Event::operator=(b);
	originalColor = b.originalColor;
	finalColor = b.finalColor;
	return *this;
}

//==============================================================================
void SceneColorEvent::updateSp(float /*prevUpdateTime*/, float crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / float(getDuration()); // delta as persentage

	SceneGraphSingleton::get().setAmbientColor(
		interpolate(originalColor, finalColor, dp));
}

} // end namespace anki

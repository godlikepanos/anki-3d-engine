#include "SceneColorEvent.h"
#include "scene/Scene.h"
#include "core/Globals.h"
#include "core/Logger.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneColorEvent::SceneColorEvent(float startTime, float duration,
	const Vec3& finalColor_)
:	Event(SCENE_COLOR, startTime, duration),
	finalColor(finalColor_)
{
	originalColor = SceneSingleton::get().getAmbientColor();
}


//==============================================================================
// Constructor copy                                                            =
//==============================================================================
SceneColorEvent::SceneColorEvent(const SceneColorEvent& b)
:	Event(SCENE_COLOR, 0.0, 0.0)
{
	*this = b;
}


//==============================================================================
// operator=                                                                   =
//==============================================================================
SceneColorEvent& SceneColorEvent::operator=(const SceneColorEvent& b)
{
	Event::operator=(b);
	originalColor = b.originalColor;
	finalColor = b.finalColor;
	return *this;
}


//==============================================================================
// updateSp                                                                    =
//==============================================================================
void SceneColorEvent::updateSp(float /*prevUpdateTime*/, float crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / float(getDuration()); // delta as persentage

	SceneSingleton::get().setAmbientColor(
		interpolate(originalColor, finalColor, dp));
}

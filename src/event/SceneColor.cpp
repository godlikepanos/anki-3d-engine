#include "SceneColor.h"
#include "scene/Scene.h"
#include "core/Globals.h"
#include "core/Logger.h"


namespace event {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneColor::SceneColor(float startTime, float duration,
	const Vec3& finalColor_)
:	Event(SCENE_COLOR, startTime, duration),
	finalColor(finalColor_)
{
	originalColor = SceneSingleton::get().getAmbientCol();
}


//==============================================================================
// Constructor copy                                                            =
//==============================================================================
SceneColor::SceneColor(const SceneColor& b)
:	Event(SCENE_COLOR, 0.0, 0.0)
{
	*this = b;
}


//==============================================================================
// operator=                                                                   =
//==============================================================================
SceneColor& SceneColor::operator=(const SceneColor& b)
{
	Event::operator=(b);
	originalColor = b.originalColor;
	finalColor = b.finalColor;
	return *this;
}


//==============================================================================
// updateSp                                                                    =
//==============================================================================
void SceneColor::updateSp(float /*prevUpdateTime*/, float crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / float(getDuration()); // delta as persentage

	SceneSingleton::get().setAmbientCol(
		interpolate(originalColor, finalColor, dp));
}


} // end namespace

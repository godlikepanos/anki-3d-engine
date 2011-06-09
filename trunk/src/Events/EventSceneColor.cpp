#include "EventSceneColor.h"
#include "Scene.h"
#include "Globals.h"


namespace Event {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
SceneColor::SceneColor(uint startTime, int duration, const Vec3& finalColor_):
	Event(SCENE_COLOR, startTime, duration),
	finalColor(finalColor_)
{
	originalColor = SceneSingleton::getInstance().getAmbientCol();
}


//======================================================================================================================
// Constructor copy                                                                                                    =
//======================================================================================================================
SceneColor::SceneColor(const SceneColor& b):
	Event(SCENE_COLOR, 0.0, 0.0)
{
	*this = b;
}


//======================================================================================================================
// operator=                                                                                                           =
//======================================================================================================================
SceneColor& SceneColor::operator=(const SceneColor& b)
{
	Event::operator=(b);
	finalColor = b.finalColor;
	return *this;
}


//======================================================================================================================
// updateSp                                                                                                            =
//======================================================================================================================
void SceneColor::updateSp(uint /*prevUpdateTime*/, uint crntTime)
{
	float d = crntTime - getStartTime(); // delta
	float dp = d / getDuration(); // delta as persentage

	Vec3 col = originalColor * (1.0 - dp) + finalColor * dp;
	SceneSingleton::getInstance().setAmbientCol(col);
}


} // end namespace

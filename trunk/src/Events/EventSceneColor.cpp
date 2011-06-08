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
{}



//======================================================================================================================
// operator=                                                                                                           =
//======================================================================================================================
SceneColor& SceneColor::operator=(const SceneColor& b)
{
	Event::operator=(b);
	finalColor = b.finalColor;
}


//======================================================================================================================
// updateSp                                                                                                            =
//======================================================================================================================
void SceneColor::updateSp(uint timeUpdate)
{
	SceneColor();
}


} // end namespace

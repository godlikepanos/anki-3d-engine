// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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
	originalColor = getSceneGraph().getAmbientColor();
}

//==============================================================================
void SceneAmbientColorEvent::update(F32 /*prevUpdateTime*/, F32 crntTime)
{
	getSceneGraph().setAmbientColor(
		interpolate(originalColor, finalColor, getDelta(crntTime)));
}

} // end namespace anki

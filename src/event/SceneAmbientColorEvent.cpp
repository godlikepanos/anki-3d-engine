// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/SceneAmbientColorEvent.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
Error SceneAmbientColorEvent::create(EventManager* manager,
	F32 startTime, F32 duration, const Vec4& finalColor)
{
	Error err = Event::create(manager, startTime, duration);

	if(!err)
	{
		m_finalColor = finalColor;
		m_originalColor = getSceneGraph().getAmbientColor();
	}

	return err;
}

//==============================================================================
Error SceneAmbientColorEvent::update(F32 /*prevUpdateTime*/, F32 crntTime)
{
	getSceneGraph().setAmbientColor(
		linearInterpolate(m_originalColor, m_finalColor, getDelta(crntTime)));

	return ErrorCode::NONE;
}

} // end namespace anki

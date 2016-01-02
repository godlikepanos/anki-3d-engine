// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_JITTER_EVENT_MOVE_EVENT_H
#define ANKI_JITTER_EVENT_MOVE_EVENT_H

#include <anki/event/Event.h>
#include <anki/Math.h>

namespace anki {

/// @addtogroup event
/// @{

/// An event for simple movable animations
class JitterMoveEvent: public Event
{
public:
	/// Constructor
	JitterMoveEvent(EventManager* manager)
		: Event(manager)
	{}

	ANKI_USE_RESULT Error init(F32 startTime, F32 duration,
		SceneNode* movableSceneNode);

	/// Implements Event::update
	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime);

	void setPositionLimits(const Vec4& posMin, const Vec4& posMax);

private:
	Vec4 m_originalPos;
	Vec4 m_newPos;
};
/// @}

} // end namespace anki

#endif

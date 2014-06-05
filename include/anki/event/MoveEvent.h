// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_EVENT_MOVE_EVENT_H
#define ANKI_EVENT_MOVE_EVENT_H

#include "anki/event/Event.h"
#include "anki/Math.h"

namespace anki {

// Forward
class SceneNode;

/// @addtogroup Events
/// @{

/// Helper class
struct MoveEventData
{
	Vec3 posMin;
	Vec3 posMax;
};

/// An event for simple movable animations
class MoveEvent: public Event, private MoveEventData
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Constructor
	MoveEvent(EventManager* manager, F32 startTime, F32 duration,
		SceneNode* movableSceneNode, const MoveEventData& data);
	/// @}

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime);

private:
	Vec3 originalPos;
	Vec3 newPos;
};
/// @}

} // end namespace anki

#endif

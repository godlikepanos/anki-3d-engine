#ifndef ANKI_EVENT_MOVABLE_EVENT_H
#define ANKI_EVENT_MOVABLE_EVENT_H

#include "anki/event/Event.h"
#include "anki/Math.h"

namespace anki {

// Forward
class SceneNode;

/// @addtogroup Events
/// @{

/// Helper class
struct MovableEventData
{
	Vec3 posMin;
	Vec3 posMax;
};

/// An event for simple movable animations
class MovableEvent: public Event, private MovableEventData
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Constructor
	MovableEvent(EventManager* manager, F32 startTime, F32 duration,
		U8 flags, SceneNode* movableSceneNode, const MovableEventData& data);
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

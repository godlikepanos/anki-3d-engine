#ifndef ANKI_EVENT_MOVABLE_EVENT_H
#define ANKI_EVENT_MOVABLE_EVENT_H

#include "anki/event/Event.h"
#include "anki/math/Math.h"

namespace anki {

// Forward
class SceneNode;

/// Helper class
struct MovableEventData
{
	SceneNode* movableSceneNode = nullptr;
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
	MovableEvent(F32 startTime, F32 duration, EventManager* manager,
		U8 flags, const MovableEventData& data);
	/// @}

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime);

private:
	Vec3 originalPos;
	Vec3 newPos;
};

} // end namespace anki

#endif

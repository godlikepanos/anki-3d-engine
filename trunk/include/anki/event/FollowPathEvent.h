#ifndef ANKI_EVENT_FOLLOW_PATH_EVENT_H
#define ANKI_EVENT_FOLLOW_PATH_EVENT_H

#include "anki/event/Event.h"
#include "anki/scene/SceneNode.h"

namespace anki {

#if 0

class FollowPathEvent: public Event
{
public:
	/// @name Constructors/Destructor
	/// @{

	/// Constructor
	FollowPathEvent(
		EventManager* manager, F32 startTime, F32 duration, U8 flags, // Event
		SceneNode* movableSceneNode, Path* path, F32 distPerTime);
	/// @}

	/// Implements Event::update
	void update(F32 prevUpdateTime, F32 crntTime);

private:
	F32 distPerTime;
	SceneNode* movableSceneNode;
	Path* path;
};

#endif

} // end namespace anki

#endif

// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/event/Event.h>
#include <anki/scene/SceneNode.h>

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


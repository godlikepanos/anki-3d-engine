// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/event/Event.h"
#include "anki/event/EventManager.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
Event::Event(EventManager* manager, F32 startTime_, F32 duration_, 
	SceneNode* node, U8 flags)
	:	SceneObject(EVENT_TYPE, node, &manager->getSceneGraph()),
		Bitset<U8>(flags), 
		startTime(startTime_),
		duration(duration_)
{}

//==============================================================================
Event::~Event()
{}

//==============================================================================
EventManager& Event::getEventManager()
{
	return getSceneGraph().getEventManager();
}

//==============================================================================
const EventManager& Event::getEventManager() const
{
	return getSceneGraph().getEventManager();
}

//==============================================================================
SceneNode* Event::getSceneNode()
{
	SceneObject* parent = getParent();
	if(parent)
	{
		return &parent->downCast<SceneNode>();
	}

	return nullptr;
}

//==============================================================================
const SceneNode* Event::getSceneNode() const
{
	const SceneObject* parent = getParent();
	if(parent)
	{
		return &parent->downCast<SceneNode>();
	}

	return nullptr;
}

//==============================================================================
F32 Event::getDelta(F32 crntTime) const
{
	F32 d = crntTime - startTime; // delta
	F32 dp = d / duration; // delta as persentage
	return dp;
}

} // end namespace anki

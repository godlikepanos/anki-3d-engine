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
Event::Event()
{}

//==============================================================================
Event::~Event()
{}

//==============================================================================
Error Event::create(
	EventManager* manager, F32 startTime, F32 duration, 
	SceneNode* node, Flag flags)
{
	ANKI_ASSERT(manager);

	m_flags = flags;
	m_manager = manager;
	m_startTime = startTime;
	m_duration = duration;
	m_node = node;

	return ErrorCode::NONE;
}

//==============================================================================
void Event::setMarkedForDeletion()
{
	if(!getMarkedForDeletion())
	{
		m_flags |= Flag::MARKED_FOR_DELETION;
		m_manager->increaseMarkedForDeletion();
	}
}

//==============================================================================
F32 Event::getDelta(F32 crntTime) const
{
	F32 d = crntTime - m_startTime; // delta
	F32 dp = d / m_duration; // delta as persentage
	return dp;
}

//==============================================================================
SceneGraph& Event::getSceneGraph()
{
	return m_manager->getSceneGraph();
}

//==============================================================================
const SceneGraph& Event::getSceneGraph() const
{
	return m_manager->getSceneGraph();
}

} // end namespace anki

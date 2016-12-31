// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/Event.h>
#include <anki/event/EventManager.h>
#include <anki/scene/SceneGraph.h>
#include <anki/util/Assert.h>

namespace anki
{

Event::Event(EventManager* manager)
	: m_manager(manager)
{
}

Event::~Event()
{
}

void Event::init(F32 startTime, F32 duration, SceneNode* snode, Flag flags)
{
	m_startTime = startTime;
	m_duration = duration;
	m_node = snode;
	m_flags = flags;

	if(duration < 0.0)
	{
		m_duration = 1000.0;
		m_flags |= Flag::REANIMATE;
	}
}

void Event::setMarkedForDeletion()
{
	if(!getMarkedForDeletion())
	{
		m_flags |= Flag::MARKED_FOR_DELETION;
		m_manager->increaseMarkedForDeletion();
	}
}

F32 Event::getDelta(F32 crntTime) const
{
	F32 d = crntTime - m_startTime; // delta
	F32 dp = d / m_duration; // delta as persentage
	return dp;
}

SceneGraph& Event::getSceneGraph()
{
	return m_manager->getSceneGraph();
}

const SceneGraph& Event::getSceneGraph() const
{
	return m_manager->getSceneGraph();
}

} // end namespace anki

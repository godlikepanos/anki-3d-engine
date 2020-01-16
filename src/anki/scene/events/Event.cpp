// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/events/Event.h>
#include <anki/scene/events/EventManager.h>
#include <anki/scene/SceneGraph.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

Event::Event(EventManager* manager)
	: m_manager(manager)
{
}

Event::~Event()
{
	m_associatedNodes.destroy(getAllocator());
}

void Event::init(Second startTime, Second duration)
{
	m_startTime = startTime;
	m_duration = (duration < 0.0) ? MAX_SECOND : duration;

	if(duration < 0.0)
	{
		m_reanimate = true;
	}
}

SceneAllocator<U8> Event::getAllocator() const
{
	return m_manager->getSceneGraph().getAllocator();
}

void Event::setMarkedForDeletion()
{
	m_manager->markEventForDeletion(this);
}

Second Event::getDelta(Second crntTime) const
{
	Second d = crntTime - m_startTime; // delta
	Second dp = d / m_duration; // delta as persentage
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

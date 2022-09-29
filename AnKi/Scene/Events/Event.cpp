// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Scene/Events/EventManager.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

Event::Event(EventManager* manager)
	: m_manager(manager)
{
}

Event::~Event()
{
	m_associatedNodes.destroy(getMemoryPool());
}

void Event::init(Second startTime, Second duration)
{
	m_startTime = startTime;
	m_duration = (duration < 0.0) ? kMaxSecond : duration;

	if(duration < 0.0)
	{
		m_reanimate = true;
	}
}

HeapMemoryPool& Event::getMemoryPool() const
{
	return m_manager->getSceneGraph().getMemoryPool();
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

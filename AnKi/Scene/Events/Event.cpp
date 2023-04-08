// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Scene/Events/EventManager.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/HighRezTimer.h>

namespace anki {

void Event::init(Second startTime, Second duration)
{
	m_startTime = startTime;
	m_duration = (duration < 0.0) ? kMaxSecond : duration;

	if(duration < 0.0)
	{
		m_reanimate = true;
	}
}

Second Event::getDelta(Second crntTime) const
{
	const Second d = crntTime - m_startTime; // delta
	const Second dp = d / m_duration; // delta as persentage
	return dp;
}

void Event::setMarkedForDeletion()
{
	SceneGraph::getSingleton().getEventManager().markEventForDeletion(this);
}

} // end namespace anki

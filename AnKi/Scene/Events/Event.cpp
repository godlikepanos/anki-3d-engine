// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Scene/Events/EventManager.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

Event::Event(Second startTime, Second duration, WeakArray<SceneNode*> nodes)
{
	m_startTime = startTime;
	m_duration = (duration < 0.0) ? kMaxSecond : duration;

	if(duration < 0.0)
	{
		m_reanimate = true;
	}

	m_associatedNodes = SceneDynamicArray<SceneNode*>(nodes);
}

} // end namespace anki

// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/events/EventManager.h>
#include <anki/scene/events/Event.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
	while(!m_events.isEmpty())
	{
		Event* event = &m_events.getFront();
		event->setMarkedForDeletion();
	}

	deleteEventsMarkedForDeletion();
}

Error EventManager::init(SceneGraph* scene)
{
	ANKI_ASSERT(scene);
	m_scene = scene;
	return Error::NONE;
}

SceneAllocator<U8> EventManager::getAllocator() const
{
	return m_scene->getAllocator();
}

SceneFrameAllocator<U8> EventManager::getFrameAllocator() const
{
	return m_scene->getFrameAllocator();
}

Error EventManager::updateAllEvents(Second prevUpdateTime, Second crntTime)
{
	Error err = Error::NONE;

	auto it = m_events.getBegin();
	auto end = m_events.getEnd();
	for(; it != end && !err; ++it)
	{
		Event& event = *it;

		// If event or the node's event is marked for deletion then dont do anything else for that event
		if(event.getMarkedForDeletion())
		{
			continue;
		}

		// Check if the associated scene nodes are marked for deletion
		for(SceneNode* node : event.m_associatedNodes)
		{
			if(node->getMarkedForDeletion())
			{
				event.setMarkedForDeletion();
			}
		}

		if(event.getMarkedForDeletion())
		{
			continue;
		}

		// Audjust starting time
		if(event.m_startTime < 0.0)
		{
			event.m_startTime = crntTime;
		}

		// Check if dead
		if(!event.isDead(crntTime))
		{
			// If not dead update it

			if(event.getStartTime() <= crntTime)
			{
				err = event.update(prevUpdateTime, crntTime);
			}
		}
		else
		{
			// Dead

			if(event.getReanimate())
			{
				event.m_startTime = prevUpdateTime;
				err = event.update(prevUpdateTime, crntTime);
			}
			else
			{
				err = event.onKilled(prevUpdateTime, crntTime);
				if(err || !event.getReanimate())
				{
					event.setMarkedForDeletion();
				}
			}
		}
	}

	return err;
}

void EventManager::markEventForDeletion(Event* event)
{
	ANKI_ASSERT(event);
	if(event->m_markedForDeletion)
	{
		return;
	}

	LockGuard<Mutex> lock(m_mtx);
	event->m_markedForDeletion = true;
	m_events.erase(event);
	m_eventsMarkedForDeletion.pushBack(event);
}

void EventManager::deleteEventsMarkedForDeletion()
{
	SceneAllocator<U8> alloc = getAllocator();

	// Gather events for deletion
	while(!m_eventsMarkedForDeletion.isEmpty())
	{
		Event* event = &m_eventsMarkedForDeletion.getFront();
		m_eventsMarkedForDeletion.popFront();

		alloc.deleteInstance(event);
	}
}

} // end namespace anki

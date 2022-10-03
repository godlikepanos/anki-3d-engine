// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/EventManager.h>
#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Scene/SceneGraph.h>

namespace anki {

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

	deleteEventsMarkedForDeletion(false);
}

Error EventManager::init(SceneGraph* scene)
{
	ANKI_ASSERT(scene);
	m_scene = scene;
	return Error::kNone;
}

HeapMemoryPool& EventManager::getMemoryPool() const
{
	return m_scene->getMemoryPool();
}

StackMemoryPool& EventManager::getFrameMemoryPool() const
{
	return m_scene->getFrameMemoryPool();
}

Error EventManager::updateAllEvents(Second prevUpdateTime, Second crntTime)
{
	Error err = Error::kNone;

	for(Event& event : m_events)
	{
		// If event or the node's event is marked for deletion then dont do anything else for that event
		if(event.getMarkedForDeletion())
		{
			continue;
		}

		// Check if the associated scene nodes are marked for deletion
		Bool skip = false;
		for(SceneNode* node : event.m_associatedNodes)
		{
			if(node->getMarkedForDeletion())
			{
				skip = true;
				break;
			}
		}

		if(skip)
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

void EventManager::deleteEventsMarkedForDeletion(Bool fullCleanup)
{
	// Mark events with to-be-deleted nodes as also to be deleted
	if(fullCleanup)
	{
		// Gather in an array because we can't call setMarkedForDeletion while iterating m_events
		DynamicArrayRaii<Event*> markedForDeletion(&getFrameMemoryPool());
		for(Event& event : m_events)
		{
			for(SceneNode* node : event.m_associatedNodes)
			{
				if(node->getMarkedForDeletion())
				{
					markedForDeletion.emplaceBack(&event);
					break;
				}
			}
		}

		for(Event* event : markedForDeletion)
		{
			event->setMarkedForDeletion();
		}
	}

	// Gather events for deletion
	while(!m_eventsMarkedForDeletion.isEmpty())
	{
		Event* event = &m_eventsMarkedForDeletion.getFront();
		m_eventsMarkedForDeletion.popFront();

		deleteInstance(getMemoryPool(), event);
	}
}

} // end namespace anki

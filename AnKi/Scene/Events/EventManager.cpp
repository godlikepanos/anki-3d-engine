// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	while(!m_eventsForRegistration.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_eventsForRegistration.popFront());
	}

	while(!m_events.isEmpty())
	{
		deleteInstance(SceneMemoryPool::getSingleton(), m_events.popFront());
	}
}

void EventManager::updateAllEvents(Second prevUpdateTime, Second crntTime)
{
	// Register new events
	while(!m_eventsForRegistration.isEmpty())
	{
		Event* e = m_eventsForRegistration.popFront();
		m_events.pushBack(e);
	}

	for(Event& event : m_events)
	{
		if(event.isMarkedForDeletion())
		{
			continue;
		}

		if(assosiatedNodesMarkedForDeletion(event))
		{
			event.markForDeletion();
			continue;
		}

		if(event.m_startTime < 0.0)
		{
			// Start the event now
			event.m_startTime = crntTime;
		}

		// Check if dead
		if(!event.isDead(crntTime))
		{
			// If not dead update it

			if(event.getStartTime() <= crntTime)
			{
				event.update(prevUpdateTime, crntTime);
			}

			if(assosiatedNodesMarkedForDeletion(event))
			{
				// Maybe the event marked the node for deletion, delete the event
				event.markForDeletion();
			}
		}
		else
		{
			// Dead, reanimate or kill

			if(event.getReanimate())
			{
				event.m_startTime = prevUpdateTime;
				event.update(prevUpdateTime, crntTime);
			}
			else
			{
				event.onKilled(prevUpdateTime, crntTime);
				if(!event.getReanimate())
				{
					event.markForDeletion();
				}
			}
		}
	}

	// Cleanup
	auto it = m_events.getBegin();
	while(it != m_events.getEnd())
	{
		if(it->isMarkedForDeletion() || assosiatedNodesMarkedForDeletion(*it))
		{
			Event* e = &(*it);

			++it; // Point to the next before deleting

			m_events.erase(e);
			deleteInstance(SceneMemoryPool::getSingleton(), e);
		}
		else
		{
			++it;
		}
	}
}

Bool EventManager::assosiatedNodesMarkedForDeletion(const Event& e)
{
	for(const SceneNode* node : e.getAssociatedSceneNodes())
	{
		if(node->isMarkedForDeletion())
		{
			return true;
		}
	}

	return false;
}

} // end namespace anki

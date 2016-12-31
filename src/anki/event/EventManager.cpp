// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/event/EventManager.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
	Error err = iterateEvents([&](Event& event) -> Error {
		event.setMarkedForDeletion();
		return ErrorCode::NONE;
	});
	(void)err;

	deleteEventsMarkedForDeletion();
}

Error EventManager::create(SceneGraph* scene)
{
	ANKI_ASSERT(scene);
	m_scene = scene;
	return ErrorCode::NONE;
}

SceneAllocator<U8> EventManager::getSceneAllocator() const
{
	return m_scene->getAllocator();
}

SceneFrameAllocator<U8> EventManager::getFrameAllocator() const
{
	return m_scene->getFrameAllocator();
}

void EventManager::registerEvent(Event* event)
{
	ANKI_ASSERT(event);
	m_events.pushBack(getSceneAllocator(), event);
}

void EventManager::unregisterEvent(List<Event*>::Iterator it)
{
	ANKI_ASSERT(it != m_events.getEnd());
	m_events.erase(getSceneAllocator(), it);
}

Error EventManager::updateAllEvents(F32 prevUpdateTime, F32 crntTime)
{
	Error err = ErrorCode::NONE;
	m_prevUpdateTime = prevUpdateTime;
	m_crntTime = crntTime;

	auto it = m_events.getBegin();
	auto end = m_events.getEnd();
	for(; it != end && !err; ++it)
	{
		Event* event = *it;

		// If event or the node's event is marked for deletion then dont do anything else for that event
		if(event->getMarkedForDeletion())
		{
			continue;
		}

		if(event->getSceneNode() != nullptr && event->getSceneNode()->getMarkedForDeletion())
		{
			event->setMarkedForDeletion();
			continue;
		}

		// Audjust starting time
		if(event->m_startTime < 0.0)
		{
			event->m_startTime = crntTime;
		}

		// Check if dead
		if(!event->isDead(crntTime))
		{
			// If not dead update it

			if(event->getStartTime() <= crntTime)
			{
				err = event->update(prevUpdateTime, crntTime);
			}
		}
		else
		{
			// Dead

			if(event->getReanimate())
			{
				event->m_startTime = prevUpdateTime;
				err = event->update(prevUpdateTime, crntTime);
			}
			else
			{
				Bool kill;
				err = event->onKilled(prevUpdateTime, crntTime, kill);
				if(!err && kill)
				{
					event->setMarkedForDeletion();
				}
			}
		}
	}

	return err;
}

void EventManager::deleteEventsMarkedForDeletion()
{
	SceneAllocator<U8> alloc = getSceneAllocator();

	// Check if nodes are marked for deletion
	if(true)
	{
		auto it = m_events.getBegin();
		auto end = m_events.getEnd();
		for(; it != end; ++it)
		{
		}
	}

	// Gather events for deletion
	while(m_markedForDeletionCount != 0)
	{
		auto it = m_events.getBegin();
		auto end = m_events.getEnd();
		for(; it != end; ++it)
		{
			Event* event = *it;
			if(event->getMarkedForDeletion())
			{
				unregisterEvent(it);
				alloc.deleteInstance<Event>(event);
				--m_markedForDeletionCount;
				break;
			}
		}
	}
}

} // end namespace anki

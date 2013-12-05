#include "anki/event/EventManager.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
EventManager::EventManager(SceneGraph* scene_)
	: scene(scene_), events(getSceneAllocator())
{}

//==============================================================================
EventManager::~EventManager()
{}

//==============================================================================
SceneAllocator<U8> EventManager::getSceneAllocator() const
{
	return scene->getAllocator();
}

//==============================================================================
SceneAllocator<U8> EventManager::getSceneFrameAllocator() const
{
	return scene->getFrameAllocator();
}

//==============================================================================
void EventManager::registerEvent(Event* event)
{
	ANKI_ASSERT(event);
	ANKI_ASSERT(std::find(events.begin(), events.end(), event) == events.end());
	events.push_back(event);
}

//==============================================================================
void EventManager::unregisterEvent(Event* event)
{
	ANKI_ASSERT(event);
	
	EventsContainer::iterator it = events.begin();
	for(; it != events.end(); it++)
	{
		if((*it) == event)
		{
			break;
		}
	}

	ANKI_ASSERT(it == events.end() && "Trying to unreg non-existing event");
	events.erase(it);
}

//==============================================================================
void EventManager::updateAllEvents(F32 prevUpdateTime_, F32 crntTime_)
{
	prevUpdateTime = prevUpdateTime_;
	crntTime = crntTime_;

	EventsContainer::iterator it = events.begin();
	for(; it != events.end(); it++)
	{
		Event* pevent = *it;

		// If event or the node's event is marked for deletion then dont 
		// do anything else for that event
		if(pevent->isMarkedForDeletion() 
			|| (pevent->getSceneNode() != nullptr 
				&& pevent->getSceneNode()->isMarkedForDeletion()))
		{
			pevent->markForDeletion();
			continue;
		}

		// Audjust starting time
		if(pevent->startTime < 0.0)
		{
			pevent->startTime = crntTime;
		}

		// Check if dead
		if(!pevent->isDead(crntTime))
		{
			// If not dead update it

			if(pevent->getStartTime() <= crntTime)
			{
				pevent->update(prevUpdateTime, crntTime);
			}
		}
		else
		{
			// Dead

			if(pevent->bitsEnabled(Event::EF_REANIMATE))
			{
				pevent->startTime = prevUpdateTime;
				pevent->update(prevUpdateTime, crntTime);
			}
			else
			{
				if(pevent->onKilled(prevUpdateTime, crntTime))
				{
					pevent->markForDeletion();
				}
			}
		}
	}
}

//==============================================================================
void EventManager::deleteEventsMarkedForDeletion()
{
	SceneAllocator<Event> al = getSceneAllocator();

	// Gather events for deletion
	SceneFrameVector<EventsContainer::iterator> 
		forDeletion(getSceneFrameAllocator());

	for(EventsContainer::iterator it = events.begin(); it != events.end(); ++it)
	{
		Event* event = *it;
		if(event->isMarkedForDeletion())
		{
			forDeletion.push_back(it);
		}
	}

	// Delete events
	for(auto it : forDeletion)
	{
		Event* event = *it;

		unregisterEvent(event);
		al.deleteInstance(event);
	}
}

} // end namespace anki

#include "anki/event/EventManager.h"
#include "anki/scene/SceneGraph.h"

#include "anki/event/SceneAmbientColorEvent.h"
#include "anki/event/LightEvent.h"
#include "anki/event/MovableEvent.h"
#include "anki/event/FollowPathEvent.h"

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
		if(it->get() == event)
		{
			break;
		}
	}

	ANKI_ASSERT(it == events.end());
	events.erase(it);
}

//==============================================================================
void EventManager::updateAllEvents(F32 prevUpdateTime_, F32 crntTime_)
{
	prevUpdateTime = prevUpdateTime_;
	crntTime = crntTime_;

	// Container to gather dead events
	SceneFrameVector<EventsContainer::iterator> 
		forDeletion(getSceneFrameAllocator());
	// XXX reserve on vector

	EventsContainer::iterator it = events.begin();
	for(; it != events.end(); it++)
	{
		Event* pevent = *it;

		// Audjust starting time
		if(pevent->startTime < 0.0)
		{
			pevent->startTime = crntTime_;
		}

		// If not dead update it
		if(!pevent->isDead(crntTime))
		{
			if(pevent->getStartTime() <= crntTime)
			{
				pevent->update(prevUpdateTime, crntTime);
			}
		}
		else
		{
			if(pevent->bitsEnabled(Event::EF_REANIMATE))
			{
				pevent->startTime = prevUpdateTime;
				pevent->update(prevUpdateTime, crntTime);
			}
			else
			{
				if((*it)->onKilled(prevUpdateTime, crntTime))
				{
					forDeletion.push_back(it);
				}
			}
		}
	}

	// Kick the dead events out
	for(EventsContainer::iterator& it : forDeletion)
	{
		events.erase(it);
	}
}

} // end namespace anki

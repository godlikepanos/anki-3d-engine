#include "anki/event/EventManager.h"
#include "anki/scene/SceneGraph.h"

#include "anki/event/SceneAmbientColorEvent.h"
#include "anki/event/LightEvent.h"
#include "anki/event/MovableEvent.h"

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
std::shared_ptr<Event> EventManager::registerEvent(Event* event)
{
	ANKI_ASSERT(event);
	SceneSharedPtrDeleter<Event> deleter;
	std::shared_ptr<Event> ptr(event, deleter, getSceneAllocator());
	events.push_back(ptr);
	return ptr;
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
	SceneFrameVector<EventsContainer::iterator> forDeletion(
		getSceneFrameAllocator());
	// XXX reserve on vector

	EventsContainer::iterator it = events.begin();
	for(; it != events.end(); it++)
	{
		std::shared_ptr<Event>& pevent = *it;

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
				forDeletion.push_back(it);
			}
		}
	}

	// Kick the dead events out
	for(EventsContainer::iterator& it : forDeletion)
	{
		events.erase(it);
	}
}

//==============================================================================
std::shared_ptr<Event> EventManager::newSceneAmbientColorEvent(
	F32 startTime, F32 duration, const Vec3& finalColor)
{
	return registerEvent(new SceneAmbientColorEvent(startTime, duration, this, 
		finalColor, scene));
}

//==============================================================================
std::shared_ptr<Event> EventManager::newLightEvent(
	F32 startTime, F32 duration, const LightEventData& data)
{
	return registerEvent(new LightEvent(startTime, duration, this,
		Event::EF_NONE, data));
}

//==============================================================================
std::shared_ptr<Event> EventManager::newMovableEvent(
	F32 startTime, F32 duration, const MovableEventData& data)
{
	return registerEvent(new MovableEvent(startTime, duration, this,
		Event::EF_NONE, data));
}

} // end namespace anki

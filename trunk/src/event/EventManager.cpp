#include "anki/event/EventManager.h"

namespace anki {

//==============================================================================
void EventManager::updateAllEvents(float prevUpdateTime_, float crntTime_)
{
	prevUpdateTime = prevUpdateTime_;
	crntTime = crntTime_;

	for(Event& event : events)
	{
		if(!event.isDead(crntTime))
		{
			event.update(prevUpdateTime, crntTime);
		}
	}
}

//==============================================================================
EventManager::EventsContainer::iterator EventManager::
	findADeadEvent(Event::EventType type)
{
	EventsContainer::iterator it = events.begin();

	while(it != events.end())
	{
		if(it->isDead(crntTime) && it->getEventType() == type)
		{
			break;
		}
		++it;
	}

	return it;
}

} // end namespace

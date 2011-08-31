#include "EventManager.h"
#include <boost/foreach.hpp>


//==============================================================================
// updateAllEvents                                                             =
//==============================================================================
void EventManager::updateAllEvents(float prevUpdateTime_, float crntTime_)
{
	prevUpdateTime = prevUpdateTime_;
	crntTime = crntTime_;

	BOOST_FOREACH(Event& event, events)
	{
		if(!event.isDead(crntTime))
		{
			event.update(prevUpdateTime, crntTime);
		}
	}
}


//==============================================================================
// findADeadEvent                                                              =
//==============================================================================
EventManager::EventsContainer::iterator EventManager::findADeadEvent(
	EventType type)
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

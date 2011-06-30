#include "EventManager.h"


namespace Event {


//==============================================================================
//  createEvent                                                                =
//==============================================================================
template<typename EventType>
EventType& Manager::createEvent(const EventType& event)
{
	EventsContainer::iterator it = findADeadEvent(event.getEventType());
	EventType* ev;

	if(it == events.end()) // No dead event found
	{
		ev = new EventType(event);
		events.push_back(ev);
	}
	else // Re-use a dead event
	{
		ev = &static_cast<EventType&>(*it);
		*ev = event;
	}

	return *ev;
}


} // end namespace

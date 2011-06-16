#include "EventManager.h"


namespace Event {


//======================================================================================================================
//  createEvent                                                                                                        =
//======================================================================================================================
template<typename EventType>
void Manager::createEvent(const EventType& event)
{
	EventsContainer::iterator it = findADeadEvent(event.getEventType());

	if(it == events.end()) // No dead event found
	{
		events.push_back(new EventType(event));
	}
	else // Re-use a dead event
	{
		Event& ev = *it;
		static_cast<EventType&>(ev) = event;
	}
}


} // end namespace

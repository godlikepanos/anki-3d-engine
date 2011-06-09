#include "EventManager.h"


namespace Event {


//======================================================================================================================
//  createEvent                                                                                                        =
//======================================================================================================================
template<typename EventType>
void Manager::createEvent(const EventType& event)
{
	events.push_back(new EventType(event));
}


} // end namespace

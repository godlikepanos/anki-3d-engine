#ifndef ANKI_EVENT_MANAGER_H
#define ANKI_EVENT_MANAGER_H

#include "anki/event/Event.h"
#include <boost/ptr_container/ptr_deque.hpp>


namespace anki {


/// This manager creates the events ands keeps tracks of them
class EventManager
{
public:
	typedef boost::ptr_deque<Event> EventsContainer;

	/// Create a new event
	template<typename EventType>
	EventType& createEvent(const EventType& event);

	/// Update
	void updateAllEvents(float prevUpdateTime, float crntTime);

private:
	enum
	{
		MAX_EVENTS_SIZE = 1000
	};

	EventsContainer events;
	float prevUpdateTime;
	float crntTime;

	/// Find a dead event of a certain type
	EventsContainer::iterator findADeadEvent(EventType type);
};


} // end namespace


#include "anki/event/EventManager.inl.h"


#endif

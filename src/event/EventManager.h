#ifndef MANAGER_H
#define MANAGER_H

#include "Event.h"
#include <boost/ptr_container/ptr_deque.hpp>


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


#include "EventManager.inl.h"


#endif

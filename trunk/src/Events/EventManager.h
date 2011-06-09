#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <boost/ptr_container/ptr_deque.hpp>
#include "Event.h"


namespace Event {


/// This manager creates the events ands keeps tracks of them
class Manager
{
	public:
		typedef boost::ptr_deque<Event> EventsContainer;

		/// Create a new event
		template<typename EventType>
		void createEvent(const EventType& event);

		/// Update
		void updateAllEvents(uint prevUpdateTime, uint crntTime);

	private:
		EventsContainer events;
};


} // end namespace


#include "EventManager.inl.h"


#endif

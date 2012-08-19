#ifndef ANKI_EVENT_MANAGER_H
#define ANKI_EVENT_MANAGER_H

#include "anki/event/Event.h"
#include "anki/util/Singleton.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// This manager creates the events ands keeps tracks of them
class EventManager
{
public:
	typedef PtrVector<Event> EventsContainer;

	EventManager();
	~EventManager();

	/// Create a new event
	template<typename EventClass>
	EventClass& createEvent(const EventClass& event);

	/// Update
	void updateAllEvents(float prevUpdateTime, float crntTime);

private:
	static const size_t MAX_EVENTS_SIZE = 1000;

	EventsContainer events;
	float prevUpdateTime;
	float crntTime;

	/// Find a dead event of a certain type
	EventsContainer::iterator findADeadEvent(Event::EventType type);
};

/// The singleton EventManager
typedef Singleton<EventManager> EventManagerSingleton;

//==============================================================================
template<typename EventClass>
EventClass& EventManager::createEvent(const EventClass& event)
{
	EventsContainer::iterator it = findADeadEvent(event.getEventType());
	EventClass* ev;

	if(it == events.end()) // No dead event found
	{
		ev = new EventClass(event);
		events.push_back(ev);
	}
	else // Re-use a dead event
	{
		ev = &static_cast<EventClass&>(*it);
		*ev = event;
	}

	return *ev;
}

} // end namespace

#endif

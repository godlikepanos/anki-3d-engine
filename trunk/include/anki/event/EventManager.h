#ifndef ANKI_EVENT_MANAGER_H
#define ANKI_EVENT_MANAGER_H

#include "anki/event/Event.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/scene/Common.h"
#include "anki/Math.h"

namespace anki {

// Forward
class SceneGraph;

/// @addtogroup Events
/// @{

/// This manager creates the events ands keeps tracks of them
class EventManager
{
public:
	typedef SceneVector<Event*> EventsContainer;

	EventManager(SceneGraph* scene);
	~EventManager();

	/// @name Accessors
	/// @{
	SceneGraph& getSceneGraph()
	{
		return *scene;
	}
	const SceneGraph& getSceneGraph() const
	{
		return *scene;
	}

	SceneAllocator<U8> getSceneAllocator() const;
	SceneAllocator<U8> getSceneFrameAllocator() const;
	/// @}

	/// Iterate events
	template<typename Func>
	void iterateEvents(Func func)
	{
		for(Event* e : events)
		{
			func(*e);
		}
	}

	/// Create a new event
	template<typename T, typename... Args>
	void newEvent(T*& event, Args&&... args)
	{
		SceneAllocator<T> al = getSceneAllocator();
		event = al.allocate(1);
		al.construct(event, this, std::forward<Args>(args)...);

		registerEvent(event);
	}

	/// Delete an event. It actualy marks it for deletion
	void deleteEvent(Event* event)
	{
		event->markForDeletion();
	}

	/// Update
	void updateAllEvents(F32 prevUpdateTime, F32 crntTime);

private:
	SceneGraph* scene = nullptr;
	EventsContainer events;
	F32 prevUpdateTime;
	F32 crntTime;

	/// Add an event to the local container
	void registerEvent(Event* event);

	/// Remove an event from the container
	void unregisterEvent(Event* event);
};
/// @}

} // end namespace anki

#endif

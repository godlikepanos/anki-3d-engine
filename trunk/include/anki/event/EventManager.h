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
class Path;
class SceneNode;

class SceneAmbientColorEvent;
class LightEvent;
class LightEventData;
class MovableEvent;
class MovableEventData;
class FollowPathEvent;

/// @addtogroup Events
/// @{

/// This manager creates the events ands keeps tracks of them
class EventManager
{
public:
	typedef SceneVector<Event> EventsContainer;

	EventManager(SceneGraph* scene);
	~EventManager();

	/// @name Accessors
	/// @{
	SceneAllocator<U8> getSceneAllocator() const;
	SceneAllocator<U8> getSceneFrameAllocator() const;

	SceneGraph& getScene()
	{
		return *scene;
	}
	const SceneGraph& getScene() const
	{
		return *scene;
	}
	/// @}

	/// Create a new event
	template<typename T, typename... Args>
	void newEvent(T*& event, Args&&.. args)
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
		++eventsMarkedForDeletionCount;
	}

	/// Update
	void updateAllEvents(F32 prevUpdateTime, F32 crntTime);

private:
	SceneGraph* scene = nullptr;
	EventsContainer events;
	F32 prevUpdateTime;
	F32 crntTime;
	U32 eventsMarkedForDeletionCount = 0;

	/// Add an event to the local container
	void registerEvent(Event* event);

	/// Remove an event from the container
	void unregisterEvent(Event* event);
};
/// @}

} // end namespace anki

#endif

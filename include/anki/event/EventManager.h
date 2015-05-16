// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_EVENT_MANAGER_H
#define ANKI_EVENT_MANAGER_H

#include "anki/event/Event.h"
#include "anki/util/List.h"
#include "anki/util/StdTypes.h"
#include "anki/scene/Common.h"
#include "anki/Math.h"

namespace anki {

// Forward
class SceneGraph;

/// @addtogroup event
/// @{

/// This manager creates the events ands keeps track of them
class EventManager
{
public:
	using EventsContainer = List<Event*>;

	EventManager();
	~EventManager();

	ANKI_USE_RESULT Error create(SceneGraph* scene);

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}

	SceneAllocator<U8> getSceneAllocator() const;
	SceneFrameAllocator<U8> getFrameAllocator() const;

	/// Iterate events
	template<typename Func>
	ANKI_USE_RESULT Error iterateEvents(Func func)
	{
		Error err = ErrorCode::NONE;
		auto it = m_events.getBegin();
		auto end = m_events.getEnd();
		for(; it != end && !err; ++it)
		{
			err = func(*(*it));
		}

		return err;
	}

	/// Create a new event
	template<typename T, typename... Args>
	ANKI_USE_RESULT Error newEvent(T*& event, Args... args)
	{
		event = getSceneAllocator().template newInstance<T>();
		ANKI_CHECK(event->create(this, args...));
		registerEvent(event);
		return ErrorCode::NONE;
	}

	/// Delete an event. It actualy marks it for deletion
	void deleteEvent(Event* event)
	{
		event->setMarkedForDeletion();
	}

	/// Update
	ANKI_USE_RESULT Error updateAllEvents(F32 prevUpdateTime, F32 crntTime);

	/// Delete events that pending deletion
	void deleteEventsMarkedForDeletion();

	void increaseMarkedForDeletion()
	{
		++m_markedForDeletionCount;
	}

private:
	SceneGraph* m_scene = nullptr;
	EventsContainer m_events;
	U32 m_markedForDeletionCount = 0;
	F32 m_prevUpdateTime;
	F32 m_crntTime;

	/// Add an event to the local container
	void registerEvent(Event* event);

	/// Remove an event from the container
	void unregisterEvent(EventsContainer::Iterator it);
};
/// @}

} // end namespace anki

#endif

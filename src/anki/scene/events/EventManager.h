// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/util/List.h>
#include <anki/Math.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// This manager creates the events ands keeps track of them
class EventManager
{
public:
	EventManager();
	~EventManager();

	ANKI_USE_RESULT Error init(SceneGraph* scene);

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}

	SceneAllocator<U8> getAllocator() const;
	SceneFrameAllocator<U8> getFrameAllocator() const;

	/// Create a new event
	/// @note It's thread-safe against itself.
	template<typename T, typename... Args>
	ANKI_USE_RESULT Error newEvent(T*& event, Args... args)
	{
		event = getAllocator().template newInstance<T>(this);
		Error err = event->init(std::forward<Args>(args)...);
		if(err)
		{
			getAllocator().deleteInstance(event);
		}
		else
		{
			LockGuard<Mutex> lock(m_mtx);
			m_events.pushBack(event);
		}
		return err;
	}

	/// Update
	ANKI_USE_RESULT Error updateAllEvents(Second prevUpdateTime, Second crntTime);

	/// Delete events that pending deletion
	void deleteEventsMarkedForDeletion();

	/// @note It's thread-safe against itself.
	void markEventForDeletion(Event* event);

private:
	SceneGraph* m_scene = nullptr;

	IntrusiveList<Event> m_events;
	IntrusiveList<Event> m_eventsMarkedForDeletion;
	Mutex m_mtx;
};
/// @}

} // end namespace anki

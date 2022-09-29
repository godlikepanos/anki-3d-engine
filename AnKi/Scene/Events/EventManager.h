// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Util/List.h>
#include <AnKi/Math.h>

namespace anki {

/// @addtogroup scene
/// @{

/// This manager creates the events ands keeps track of them
class EventManager
{
public:
	EventManager();
	~EventManager();

	Error init(SceneGraph* scene);

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}

	HeapMemoryPool& getMemoryPool() const;
	StackMemoryPool& getFrameMemoryPool() const;

	/// Create a new event
	/// @note It's thread-safe against itself.
	template<typename T, typename... Args>
	Error newEvent(T*& event, Args... args)
	{
		event = newInstance<T>(getMemoryPool(), this);
		Error err = event->init(std::forward<Args>(args)...);
		if(err)
		{
			deleteInstance(getMemoryPool(), event);
		}
		else
		{
			LockGuard<Mutex> lock(m_mtx);
			m_events.pushBack(event);
		}
		return err;
	}

	/// Update
	Error updateAllEvents(Second prevUpdateTime, Second crntTime);

	/// Delete events that pending deletion
	void deleteEventsMarkedForDeletion(Bool fullCleanup);

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

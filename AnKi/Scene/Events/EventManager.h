// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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

	/// Create a new event
	/// @note It's thread-safe against itself. Can be called in Event::update.
	template<typename T, typename... TArgs>
	T* newEvent(TArgs&&... args)
	{
		T* event = newInstance<T>(SceneMemoryPool::getSingleton(), std::forward<TArgs>(args)...);
		LockGuard lock(m_eventsForRegistrationMtx);
		m_eventsForRegistration.pushBack(event);
		return event;
	}

	/// Update
	ANKI_INTERNAL void updateAllEvents(Second prevUpdateTime, Second crntTime);

private:
	IntrusiveList<Event> m_events;

	IntrusiveList<Event> m_eventsForRegistration;
	SpinLock m_eventsForRegistrationMtx;

	static Bool assosiatedNodesMarkedForDeletion(const Event& e);
};
/// @}

} // end namespace anki

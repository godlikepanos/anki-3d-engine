// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Util/BlockArray.h>
#include <AnKi/Math.h>

namespace anki {

ANKI_CVAR(NumericCVar<U32>, Scene, EventUpdateMutexCount, 32, 1, 64, "Mutex count used to lock the nodes that are used in event update()")

// This manager creates the events ands keeps track of them
class EventManager : public MakeSingleton<EventManager>
{
public:
	EventManager();

	~EventManager();

	void init();

	// Create a new event
	// It's thread-safe against itself. Can be called in Event::update.
	template<typename T, typename... TArgs>
	T* newEvent(TArgs&&... args);

	// Update
	ANKI_INTERNAL void updateAllEvents(Second prevUpdateTime, Second crntTime);

private:
	static constexpr U64 kIsPtrMagic = 0xDEADC0FEE2531821;
	static constexpr U32 kEventStorageSize = getAlignedRoundUp(16, sizeof(Event) + sizeof(void*));
	static constexpr U32 kEventStorageAlignment = max(alignof(Event), alignof(Vec4));

	class UpdateCtx;

	// Most derived classes will have size almost equal to their base (Event). Use a storage that works for the common size as well as bigger sizes
	class EventStorage
	{
	public:
		class Ptr
		{
		public:
			U64 m_magic; // If it's kIsPtrMagic then the pointer below is valid
			Event* m_pEvent;
		};

		// m_magic aliases the first 8 bytes of an inline-constructed Event, which is its vtable pointer. A real vtable pointer will never
		// equal kIsPtrMagic, so reading m_magic distinguishes inline storage from pointer storage. See toEvent().
		union
		{
			alignas(kEventStorageAlignment) U8 m_eventStorage[kEventStorageSize];
			Ptr m_pEvent;
		};
	};

	static_assert(sizeof(EventStorage::Ptr) <= kEventStorageSize, "The pointer variant must fit in the inline storage");

	SceneBlockArray<EventStorage, BlockArrayConfig<16>> m_events;
	Mutex m_eventsMtx;

	SceneDynamicArray<Mutex> m_nodeMutexes;

	SceneDynamicArray<Event*> m_newEventsWhileUpdate; // Events created during updateAllEvents()
	Mutex m_newEventsWhileUpdateMtx;

	Bool m_inUpdate = false;

	static Bool associatedNodesMarkedForDeletion(const Event& e);

	void updateEvents(U32 threadId, UpdateCtx& ctx);

	void deleteEvents(ConstWeakArray<U32> eventIndices);

	// Reads m_magic (which aliases the inline Event's vtable pointer) to tell pointer storage from inline storage
	Event& toEvent(EventStorage& s)
	{
		ANKI_ASSERT(s.m_pEvent.m_magic != kIsPtrMagic || s.m_pEvent.m_pEvent);
		return (s.m_pEvent.m_magic == kIsPtrMagic) ? *s.m_pEvent.m_pEvent : *reinterpret_cast<Event*>(s.m_eventStorage);
	}

	void updateEvent(U32 threadId, UpdateCtx& ctx, Event& event);
};

template<typename T, typename... TArgs>
T* EventManager::newEvent(TArgs&&... args)
{
	T* event;

	if(m_inUpdate)
	{
		// Threads are iterating the m_events so we can't add new events in m_events, unfortunately we need to defer but also use pointer

		event = newInstance<T>(SceneMemoryPool::getSingleton(), std::forward<TArgs>(args)...);
		event->m_blockArrayIndex = kMaxU32;

		LockGuard lock(m_newEventsWhileUpdateMtx);
		m_newEventsWhileUpdate.emplaceBack(event);
	}
	else
	{
		U32 blockArrayIndex = kMaxU32;
		EventStorage* eventStorage;
		{
			LockGuard lock(m_eventsMtx);
			auto it = m_events.emplace();
			blockArrayIndex = it.getArrayIndex();
			eventStorage = &(*it);
		}

		// It's fine to be touching the eventStorage outside the lock, BlockArray will not move it around

		if(sizeof(T) <= kEventStorageSize && alignof(T) <= kEventStorageAlignment)
		{
			// Can use the storage already in EventStorage
			::new(eventStorage->m_eventStorage) T(std::forward<TArgs>(args)...);
			event = reinterpret_cast<T*>(eventStorage->m_eventStorage);
		}
		else
		{
			// Need a new allocation
			event = newInstance<T>(SceneMemoryPool::getSingleton(), std::forward<TArgs>(args)...);
			eventStorage->m_pEvent.m_pEvent = event;
			eventStorage->m_pEvent.m_magic = kIsPtrMagic;
		}

		event->m_blockArrayIndex = blockArrayIndex;
	}

	return event;
}

} // end namespace anki

// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Events/EventManager.h>
#include <AnKi/Scene/Events/Event.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

class EventManager::UpdateCtx
{
public:
	class PerThread
	{
	public:
		DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> m_eventsForDeletion;

		PerThread(StackMemoryPool* pool)
			: m_eventsForDeletion(pool)
		{
		}
	};

	Second m_prevUpdateTime = 0.0;
	Second m_crntTime = 0.0;

	Atomic<U32> m_crntBlockArrayIdx = {0};
	U32 m_lastBlockArrayIdx;

	DynamicArray<PerThread, MemoryPoolPtrWrapper<StackMemoryPool>> m_perThread;

	Atomic<Bool> m_nodesMarkedForDeletion = false;

	UpdateCtx(U32 threadCount)
		: m_perThread(&SceneGraph::getSingleton().getFrameMemoryPool())
	{
		m_perThread.resize(threadCount, &SceneGraph::getSingleton().getFrameMemoryPool());
	}
};

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
	for(Event* e : m_newEventsWhileUpdate)
	{
		deleteInstance(SceneMemoryPool::getSingleton(), e);
	}

	SceneDynamicArray<U32> indices;
	for(auto it = m_events.getBegin(); it != m_events.getEnd(); ++it)
	{
		indices.emplaceBack(it.getArrayIndex());
	}
	deleteEvents(indices);
}

void EventManager::init()
{
	m_nodeMutexes.resize(g_cvarSceneEventUpdateMutexCount);
}

void EventManager::updateAllEvents(Second prevUpdateTime, Second crntTime)
{
	ANKI_TRACE_SCOPED_EVENT(EventManagerUpdate);

	if(m_events.getSize() == 0 && m_newEventsWhileUpdate.getSize() == 0) [[unlikely]]
	{
		return;
	}

	ANKI_ASSERT(!m_inUpdate);
	m_inUpdate = true;

	// Move new events to m_events
	for(Event* e : m_newEventsWhileUpdate)
	{
		auto it = m_events.emplace();
		it->m_pEvent.m_magic = kIsPtrMagic;
		it->m_pEvent.m_pEvent = e;
		e->m_blockArrayIndex = it.getArrayIndex();
	}
	m_newEventsWhileUpdate.destroy();

	// Update all events
	UpdateCtx ctx(CoreThreadJobManager::getSingleton().getThreadCount());
	ctx.m_prevUpdateTime = prevUpdateTime;
	ctx.m_crntTime = crntTime;
	ctx.m_lastBlockArrayIdx = m_events.getBack().getArrayIndex();

	for(U32 i = 0; i < CoreThreadJobManager::getSingleton().getThreadCount(); i++)
	{
		CoreThreadJobManager::getSingleton().dispatchTask([this, &ctx](U32 tid) {
			updateEvents(tid, ctx);
		});
	}

	CoreThreadJobManager::getSingleton().waitForAllTasksToFinish();

	// Now that update has finished, take another look and kill some events
	if(ctx.m_nodesMarkedForDeletion.getNonAtomically())
	{
		for(EventStorage& s : m_events)
		{
			Event& event = toEvent(s);
			if(!event.isMarkedForDeletion() && associatedNodesMarkedForDeletion(event))
			{
				event.markForDeletion(); // Will be cleaned next frame but it's OK
			}
		}
	}

	// Cleanup
	for(UpdateCtx::PerThread& perThread : ctx.m_perThread)
	{
		if(perThread.m_eventsForDeletion.getSize())
		{
			deleteEvents(perThread.m_eventsForDeletion);
		}
	}

	m_inUpdate = false;
}

void EventManager::updateEvents(U32 threadId, UpdateCtx& ctx)
{
	ANKI_TRACE_SCOPED_EVENT(EventsUpdate);

	while(1)
	{
		// Fetch a batch
		constexpr U32 batchMaxSize = 8;
		const U32 entryIndex = ctx.m_crntBlockArrayIdx.fetchAdd(batchMaxSize);
		if(entryIndex > ctx.m_lastBlockArrayIdx)
		{
			break;
		}

		for(U32 i = entryIndex; i < min(entryIndex + batchMaxSize, ctx.m_lastBlockArrayIdx + 1); ++i)
		{
			const Bool exists = m_events.indexExists(i);
			if(!exists)
			{
				continue;
			}

			Event& event = toEvent(m_events[i]);
			ANKI_ASSERT(i == event.m_blockArrayIndex);
			updateEvent(threadId, ctx, event);
		}
	}
}

void EventManager::updateEvent(U32 threadId, UpdateCtx& ctx, Event& event)
{
	UpdateCtx::PerThread& perThread = ctx.m_perThread[threadId];

	if(event.isMarkedForDeletion())
	{
		perThread.m_eventsForDeletion.emplaceBack(event.m_blockArrayIndex);
		return;
	}

	// Lock the scene node mutexes to protect the scene nodes. Lock in order
	BitSet<128, U64> mutexesToLockMask(false);
	ANKI_ASSERT(m_nodeMutexes.getSize() <= 128);
	for(SceneNode* node : event.m_associatedNodes)
	{
		const U32 mtxIdx = node->getUuid() % m_nodeMutexes.getSize();
		mutexesToLockMask.set(mtxIdx);
	}

	U32 mtxIdx;
	BitSet<128, U64> tmpMask = mutexesToLockMask;
	while((mtxIdx = tmpMask.getLeastSignificantBit()) != kMaxU32)
	{
		m_nodeMutexes[mtxIdx].lock();
		tmpMask.unset(mtxIdx);
	}

	// Try update
	if(!associatedNodesMarkedForDeletion(event)) // Touch the nodes while holding the locks
	{
		// Do the event update

		const Second crntTime = ctx.m_crntTime;
		const Second prevUpdateTime = ctx.m_prevUpdateTime;

		if(event.m_startTime < 0.0)
		{
			// Start the event now
			event.m_startTime = crntTime;
		}

		// Check if dead
		if(!event.isDead(crntTime))
		{
			// If not dead update it

			if(event.getStartTime() <= crntTime)
			{
				event.update(prevUpdateTime, crntTime);
			}
		}
		else
		{
			// Dead, reanimate or kill

			if(event.getReanimate())
			{
				event.m_startTime = prevUpdateTime;
				event.update(prevUpdateTime, crntTime);
			}
			else
			{
				event.onKilled(prevUpdateTime, crntTime);
				if(!event.getReanimate())
				{
					event.markForDeletion();
				}
			}
		}

		// Do an additional check in case the event marked nodes for deletion
		if(associatedNodesMarkedForDeletion(event))
		{
			event.markForDeletion();
			ctx.m_nodesMarkedForDeletion.store(true);
		}
	}
	else
	{
		event.markForDeletion();
	}

	// Unlock the mutexes in reverse order
	while((mtxIdx = mutexesToLockMask.getMostSignificantBit()) != kMaxU32)
	{
		m_nodeMutexes[mtxIdx].unlock();
		mutexesToLockMask.unset(mtxIdx);
	}
}

Bool EventManager::associatedNodesMarkedForDeletion(const Event& e)
{
	for(const SceneNode* node : e.getAssociatedSceneNodes())
	{
		if(node->isMarkedForDeletion())
		{
			return true;
		}
	}

	return false;
}

void EventManager::deleteEvents(ConstWeakArray<U32> eventIndices)
{
	for(U32 idx : eventIndices)
	{
		auto it = m_events.indexToIterator(idx);
		const Bool isPtr = it->m_pEvent.m_magic == kIsPtrMagic;
		if(isPtr)
		{
			ANKI_ASSERT(it->m_pEvent.m_pEvent->m_blockArrayIndex == idx);
			deleteInstance(SceneMemoryPool::getSingleton(), it->m_pEvent.m_pEvent);
		}
		else
		{
			Event& e = *reinterpret_cast<Event*>(it->m_eventStorage);
			ANKI_ASSERT(e.m_blockArrayIndex == idx);
			e.~Event();
		}

		m_events.erase(it);
	}
}

} // end namespace anki

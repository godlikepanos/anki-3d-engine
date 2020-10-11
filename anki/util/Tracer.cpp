// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Tracer.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/HashMap.h>
#include <anki/util/List.h>

namespace anki
{

class Tracer::Chunk : public IntrusiveListEnabled<Chunk>
{
public:
	Array<TracerEvent, EVENTS_PER_CHUNK> m_events;
	U32 m_eventCount = 0;
	Array<TracerCounter, COUNTERS_PER_CHUNK> m_counters;
	U32 m_counterCount = 0;
};

/// Thread local storage.
class alignas(ANKI_CACHE_LINE_SIZE) Tracer::ThreadLocal
{
public:
	ThreadId m_tid = 0;

	Chunk* m_currentChunk = nullptr;
	IntrusiveList<Chunk> m_allChunks;
	SpinLock m_currentChunkLock;
};

thread_local Tracer::ThreadLocal* Tracer::m_threadLocal = nullptr;

Tracer::~Tracer()
{
	LockGuard<Mutex> lock(m_allThreadLocalMtx);
	for(ThreadLocal* tlocal : m_allThreadLocal)
	{
		m_alloc.deleteInstance(tlocal);
	}
	m_allThreadLocal.destroy(m_alloc);
}

Tracer::ThreadLocal& Tracer::getThreadLocal()
{
	ThreadLocal* out = m_threadLocal;
	if(ANKI_UNLIKELY(out == nullptr))
	{
		out = m_alloc.newInstance<ThreadLocal>();
		out->m_tid = Thread::getCurrentThreadId();
		m_threadLocal = out;

		// Store it
		LockGuard<Mutex> lock(m_allThreadLocalMtx);
		m_allThreadLocal.emplaceBack(m_alloc, out);
	}

	return *out;
}

Tracer::Chunk& Tracer::getOrCreateChunk(ThreadLocal& tlocal)
{
	Chunk* out;

	if(tlocal.m_currentChunk && tlocal.m_currentChunk->m_eventCount < EVENTS_PER_CHUNK
	   && tlocal.m_currentChunk->m_counterCount < COUNTERS_PER_CHUNK)
	{
		// There is a chunk and it has enough space
		out = tlocal.m_currentChunk;
	}
	else
	{
		// Create a new
		out = m_alloc.newInstance<Chunk>();
		tlocal.m_currentChunk = out;
		tlocal.m_allChunks.pushBack(out);
	}

	return *out;
}

TracerEventHandle Tracer::beginEvent()
{
	TracerEventHandle out;

	if(m_enabled)
	{
		out.m_start = HighRezTimer::getCurrentTime();
	}
	else
	{
		out.m_start = 0.0;
	}

	return out;
}

void Tracer::endEvent(const char* eventName, TracerEventHandle event)
{
	if(!m_enabled || event.m_start == 0.0)
	{
		return;
	}

	// Get the time before the lock and everything
	const Second duration = HighRezTimer::getCurrentTime() - event.m_start;
	if(duration == 0.0)
	{
		return;
	}

	ThreadLocal& tlocal = getThreadLocal();

	// Write the event
	LockGuard<SpinLock> lock(tlocal.m_currentChunkLock);
	Chunk& chunk = getOrCreateChunk(tlocal);

	TracerEvent& writeEvent = chunk.m_events[chunk.m_eventCount++];
	writeEvent.m_name = eventName;
	writeEvent.m_start = event.m_start;
	writeEvent.m_duration = duration;

	// Write counter as well. In ns
	TracerCounter& writeCounter = chunk.m_counters[chunk.m_counterCount++];
	writeCounter.m_name = eventName;
	writeCounter.m_value = U64(writeEvent.m_duration * 1000000000.0);
}

void Tracer::addCustomEvent(const char* eventName, Second start, Second duration)
{
	ANKI_ASSERT(eventName && start >= 0.0 && duration >= 0.0);
	if(!m_enabled || duration == 0.0)
	{
		return;
	}

	ThreadLocal& tlocal = getThreadLocal();

	// Write the event
	LockGuard<SpinLock> lock(tlocal.m_currentChunkLock);
	Chunk& chunk = getOrCreateChunk(tlocal);

	TracerEvent& writeEvent = chunk.m_events[chunk.m_eventCount++];
	writeEvent.m_name = eventName;
	writeEvent.m_start = start;
	writeEvent.m_duration = duration;

	// Write counter as well. In ns
	TracerCounter& writeCounter = chunk.m_counters[chunk.m_counterCount++];
	writeCounter.m_name = eventName;
	writeCounter.m_value = U64(duration * 1000000000.0);
}

void Tracer::incrementCounter(const char* counterName, U64 value)
{
	if(!m_enabled)
	{
		return;
	}

	ThreadLocal& tlocal = getThreadLocal();

	LockGuard<SpinLock> lock(tlocal.m_currentChunkLock);
	Chunk& chunk = getOrCreateChunk(tlocal);

	TracerCounter& writeTo = chunk.m_counters[chunk.m_counterCount++];
	writeTo.m_name = counterName;
	writeTo.m_value = value;
}

void Tracer::flush(TracerFlushCallback callback, void* callbackUserData)
{
	ANKI_ASSERT(callback);

	LockGuard<Mutex> lock(m_allThreadLocalMtx);
	for(ThreadLocal* tlocal : m_allThreadLocal)
	{
		LockGuard<SpinLock> lock2(tlocal->m_currentChunkLock);

		while(!tlocal->m_allChunks.isEmpty())
		{
			Chunk* chunk = tlocal->m_allChunks.popFront();

			callback(callbackUserData, tlocal->m_tid, WeakArray<TracerEvent>(&chunk->m_events[0], chunk->m_eventCount),
					 WeakArray<TracerCounter>(&chunk->m_counters[0], chunk->m_counterCount));

			m_alloc.deleteInstance(chunk);
		}

		tlocal->m_currentChunk = nullptr;
	}
}

} // end namespace anki

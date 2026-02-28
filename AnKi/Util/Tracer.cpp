// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/List.h>
#if ANKI_OS_ANDROID
#	include <ThirdParty/StreamlineAnnotate/streamline_annotate.h>
#endif
#if ANKI_GR_BACKEND_DIRECT3D && ANKI_TRACING_ENABLED
#	if !defined(WIN32_LEAN_AND_MEAN)
#		define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#	endif
#	include <windows.h>
#	define USE_PIX ANKI_TRACING_ENABLED
#	include <pix3.h>
#endif

namespace anki {

#if ANKI_TRACING_ENABLED

class Tracer::Chunk : public IntrusiveListEnabled<Chunk>
{
public:
	Array<TracerEvent, kEventsPerChunk> m_events;
	U32 m_eventCount = 0;
	Array<TracerCounter, kCountersPerChunk> m_counters;
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

Tracer::Tracer()
{
	setEnabled(false);
#	if ANKI_OS_ANDROID
	ANNOTATE_SETUP;
	setStreamlineEnabled(false);
#	endif
}

Tracer::~Tracer()
{
	LockGuard<Mutex> lock(m_allThreadLocalMtx);
	for(ThreadLocal* tlocal : m_allThreadLocal)
	{
		while(!tlocal->m_allChunks.isEmpty())
		{
			Chunk* chunk = tlocal->m_allChunks.popFront();
			deleteInstance(DefaultMemoryPool::getSingleton(), chunk);
		}

		deleteInstance(DefaultMemoryPool::getSingleton(), tlocal);
	}
}

Tracer::ThreadLocal& Tracer::getThreadLocal()
{
	ThreadLocal* out = m_threadLocal;
	if(out == nullptr) [[unlikely]]
	{
		out = newInstance<ThreadLocal>(DefaultMemoryPool::getSingleton());
		out->m_tid = Thread::getCurrentThreadId();
		m_threadLocal = out;

		// Store it
		LockGuard<Mutex> lock(m_allThreadLocalMtx);
		m_allThreadLocal.emplaceBack(out);
	}

	return *out;
}

Tracer::Chunk& Tracer::getOrCreateChunk(ThreadLocal& tlocal)
{
	Chunk* out;

	if(tlocal.m_currentChunk && tlocal.m_currentChunk->m_eventCount < kEventsPerChunk && tlocal.m_currentChunk->m_counterCount < kCountersPerChunk)
	{
		// There is a chunk and it has enough space
		out = tlocal.m_currentChunk;
	}
	else
	{
		// Create a new
		out = newInstance<Chunk>(DefaultMemoryPool::getSingleton());
		tlocal.m_currentChunk = out;
		tlocal.m_allChunks.pushBack(out);
	}

	return *out;
}

TracerEventHandle Tracer::beginEvent([[maybe_unused]] const char* eventName)
{
	TracerEventHandle out;

	if(m_enabled)
	{
		out.m_start = HighRezTimer::getCurrentTime();

#	if ANKI_OS_ANDROID
		if(m_streamlineEnabled)
		{
			ANNOTATE_COLOR(ANNOTATE_RED, eventName);
		}
#	endif

#	if ANKI_GR_BACKEND_DIRECT3D
		if(m_pixEnabled)
		{
			const U32 green = PIX_COLOR(0, 255, 0);
			PIXBeginEvent(green, "%s", eventName);
		}
#	endif
	}
	else
	{
		out.m_start = 0.0;
	}

	return out;
}

void Tracer::endEvent(const char* eventName, TracerEventHandle event)
{
	if(!m_enabled)
	{
		return;
	}

#	if ANKI_OS_ANDROID
	if(m_streamlineEnabled)
	{
		ANNOTATE_END();
	}
#	endif

#	if ANKI_GR_BACKEND_DIRECT3D
	if(m_pixEnabled)
	{
		PIXEndEvent();
	}
#	endif

	if(event.m_start == 0.0)
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

			deleteInstance(DefaultMemoryPool::getSingleton(), chunk);
		}

		tlocal->m_currentChunk = nullptr;
	}
}

#endif

} // end namespace anki

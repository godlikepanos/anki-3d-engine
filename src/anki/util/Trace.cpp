// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Trace.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

thread_local Tracer::ThreadLocal Tracer::m_threadLocal;

Tracer::ThreadLocal& Tracer::getThreadLocal()
{
	ThreadLocal& out = m_threadLocal;
	if(ANKI_UNLIKELY(!out.m_tracerKnowsAboutThis))
	{
		LockGuard<Mutex> lock(m_threadLocalMtx);
		m_allThreadLocal.emplaceBack(m_alloc, &out);
		out.m_tracerKnowsAboutThis = true;
	}

	return out;
}

void Tracer::beginEvent()
{
	if(m_enabled)
	{
		ThreadLocal& threadLocal = getThreadLocal();
		Event* event = threadLocal.m_eventAlloc.newInstance(m_alloc);
		event->m_timestamp = HighRezTimer::getCurrentTime();
		threadLocal.m_events.pushBack(event);
	}
}

void Tracer::endEvent(const char* eventName)
{
	if(m_enabled)
	{
		// Set the time in the event
		ThreadLocal& threadLocal = getThreadLocal();
		ANKI_ASSERT(!threadLocal.m_events.isEmpty());
		Event& event = threadLocal.m_events.getBack();
		event.m_name = eventName;
		event.m_timestamp = HighRezTimer::getCurrentTime() - event.m_timestamp;

		// Store a counter as well
		increaseCounter(eventName, U64(event.m_duration * 1000000000.0));
	}
}

void Tracer::increaseCounter(const char* counterName, U64 value)
{
	if(m_enabled)
	{
		ThreadLocal& threadLocal = getThreadLocal();
		Counter* counter = threadLocal.m_counterAlloc.newInstance(m_alloc);
		counter->m_name = counterName;
		counter->m_value = value;
		counter->m_frame = m_frame.load();
	}
}

} // end namespace anki
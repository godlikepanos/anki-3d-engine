// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Trace.h>
#include <anki/util/HighRezTimer.h>

namespace anki
{

thread_local Tracer::ThreadLocal Tracer::m_threadLocal;

void Tracer::beginFrame()
{
	m_startFrameTime = HighRezTimer::getCurrentTime();
}

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
	ANKI_ASSERT(eventName);

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
	ANKI_ASSERT(counterName);

	if(m_enabled)
	{
		ThreadLocal& threadLocal = getThreadLocal();
		Counter* counter = threadLocal.m_counterAlloc.newInstance(m_alloc);
		counter->m_name = counterName;
		counter->m_value = value;
		counter->m_frame = m_frame;
		counter->m_startFrameTime = m_startFrameTime;
	}
}

void Tracer::compactCounters(DynamicArrayAuto<Counter>& allCounters)
{
	// Gather all the counters
	DynamicArrayAuto<Counter> uncompactCounters(m_alloc);
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_counters.isEmpty())
		{
			// Pop counter
			Counter& inCounter = threadLocal->m_counters.getFront();
			threadLocal->m_counters.popFront();

			// Copy
			Counter newCounter = inCounter;
			uncompactCounters.emplaceBack(newCounter);

			// Delete poped counter
			threadLocal->m_counterAlloc.deleteInstance(m_alloc, &inCounter);
		}
	}

	if(uncompactCounters.getSize() == 0)
	{
		// Early exit
		return;
	}

	// Sort them
	std::sort(uncompactCounters.getBegin(), uncompactCounters.getEnd(), [](const Counter& a, const Counter& b) {
		if(a.m_frame != b.m_frame)
		{
			return a.m_frame < b.m_frame;
		}

		ANKI_ASSERT(a.m_name && a.m_name);
		return std::strcmp(a.m_name, b.m_name) < 0;
	});

	// Compact them
	allCounters.emplaceBack(uncompactCounters[0]);
	for(U i = 1; i < uncompactCounters.getSize(); ++i)
	{
		const Counter& inCounter = uncompactCounters[i];
		Counter& outCounter = allCounters.getBack();
		if(CString(inCounter.m_name) != CString(outCounter.m_name) || inCounter.m_frame != outCounter.m_frame)
		{
			// Create a new entry
			allCounters.emplaceBack(inCounter);
		}
		else
		{
			// Append value to the existing counter
			outCounter.m_value += inCounter.m_value;
		}
	}
}

Error Tracer::writeTraceJson(CString filename, const DynamicArrayAuto<Counter>& counters)
{
	class NEvent : public Event
	{
	public:
		ThreadId m_tid;
	};

	// Gather all events from all threads
	DynamicArrayAuto<NEvent> allEvents(m_alloc);
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_events.isEmpty())
		{
			// Pop event
			Event& inEvent = threadLocal->m_events.getFront();
			threadLocal->m_events.popFront();

			// Copy
			NEvent newEvent;
			static_cast<Event&>(newEvent) = inEvent;
			newEvent.m_tid = threadLocal->m_tid;
			allEvents.emplaceBack(newEvent);

			// Delete poped event
			threadLocal->m_eventAlloc.deleteInstance(m_alloc, &inEvent);
		}
	}

	if(allEvents.getSize() == 0)
	{
		// Early exit
		return Error::NONE;
	}

	// Sort them
	std::sort(allEvents.getBegin(), allEvents.getEnd(), [](const NEvent& a, const NEvent& b) {
		return a.m_timestamp < b.m_timestamp;
	});

	// Write the events to the file
	StringAuto newFname(m_alloc);
	newFname.sprintf("%s_trace.json", filename.cstr());
	File file;
	ANKI_CHECK(file.open(newFname.toCString(), FileOpenFlag::WRITE));

	for(const NEvent& event : allEvents)
	{
		U64 startMicroSec = U64(event.m_timestamp * 1000000.0);
		U64 durMicroSec = U64(event.m_duration * 1000000.0);

		if(durMicroSec == 0)
		{
			continue;
		}

		ANKI_CHECK(file.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
								  "\"pid\": 1, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
			event.m_name,
			event.m_tid,
			startMicroSec,
			durMicroSec));
	}

	// Write the counters
	for(const Counter& counter : counters)
	{
		ANKI_CHECK(m_traceFile.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"C\", "
										 "\"pid\": 1, \"ts\": %llu, \"args\": {\"val\": %llu}},\n",
			counter.m_name,
			U64(counter.m_startFrameTime * 1000000.0),
			counter.m_value));
	}

	return Error::NONE;
}

Error Tracer::flush(CString filename)
{
	// TODO
	return Error::NONE;
}

} // end namespace anki
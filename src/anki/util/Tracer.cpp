// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Tracer.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/HashMap.h>

namespace anki
{

thread_local Tracer::ThreadLocal Tracer::m_threadLocal;

class Tracer::PerFrameCounters
{
public:
	DynamicArrayAuto<Counter> m_counters;
	U32 m_frameIdx;

	PerFrameCounters(GenericMemoryPoolAllocator<U8> alloc)
		: m_counters(alloc)
	{
	}
};

class Tracer::FlushCtx
{
public:
	GenericMemoryPoolAllocator<U8> m_alloc;
	CString m_filename;
	DynamicArrayAuto<CString> m_counterNames;
	DynamicArrayAuto<PerFrameCounters> m_counters;
	DynamicArrayAuto<Event> m_events;

	FlushCtx(GenericMemoryPoolAllocator<U8> alloc, const CString& filename)
		: m_alloc(alloc)
		, m_filename(filename)
		, m_counterNames(alloc)
		, m_counters(alloc)
		, m_events(alloc)
	{
	}
};

Tracer::~Tracer()
{
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_counters.isEmpty())
		{
			Counter& counter = threadLocal->m_counters.getFront();
			threadLocal->m_counters.popFront();
			threadLocal->m_counterAlloc.deleteInstance(m_alloc, &counter);
		}

		while(!threadLocal->m_events.isEmpty())
		{
			Event& event = threadLocal->m_events.getFront();
			threadLocal->m_events.popFront();
			threadLocal->m_eventAlloc.deleteInstance(m_alloc, &event);
		}
	}

	m_allThreadLocal.destroy(m_alloc);
	m_frames.destroy(m_alloc);
}

void Tracer::beginFrame(U64 frame)
{
#if ANKI_ASSERTS_ENABLED
	if(m_frames.getSize() > 0)
	{
		ANKI_ASSERT(frame > m_frames.getBack().m_frame);
	}
#endif

	ANKI_ASSERT(!isInsideBeginEndFrame());

	Frame f;
	f.m_startFrameTime = HighRezTimer::getCurrentTime();
	f.m_endFrameTime = 0.0;
	f.m_frame = frame;
#if ANKI_ASSERTS_ENABLED
	f.m_canRecord = true;
#endif
	m_frames.emplaceBack(m_alloc, f);
}

void Tracer::endFrame()
{
	ANKI_ASSERT(isInsideBeginEndFrame());
	m_frames.getBack().m_canRecord = false;
	m_frames.getBack().m_endFrameTime = HighRezTimer::getCurrentTime();
}

Tracer::ThreadLocal& Tracer::getThreadLocal()
{
	ThreadLocal& out = m_threadLocal;
	if(ANKI_UNLIKELY(!out.m_tracerKnowsAboutThis))
	{
		LockGuard<Mutex> lock(m_threadLocalMtx);
		m_allThreadLocal.emplaceBack(m_alloc, &out);
		out.m_tid = Thread::getCurrentThreadId();
		out.m_tracerKnowsAboutThis = true;
	}

	return out;
}

void Tracer::beginEvent()
{
	ANKI_ASSERT(isInsideBeginEndFrame());

	ThreadLocal& threadLocal = getThreadLocal();
	Event* event = threadLocal.m_eventAlloc.newInstance(m_alloc);
	event->m_timestamp = HighRezTimer::getCurrentTime();
	threadLocal.m_events.pushBack(event);
}

void Tracer::endEvent(const char* eventName)
{
	ANKI_ASSERT(eventName);
	ANKI_ASSERT(isInsideBeginEndFrame());

	// Set the time in the event
	ThreadLocal& threadLocal = getThreadLocal();
	ANKI_ASSERT(!threadLocal.m_events.isEmpty());
	Event& event = threadLocal.m_events.getBack();
	event.m_name = eventName;
	event.m_duration = HighRezTimer::getCurrentTime() - event.m_timestamp;

	// Store a counter as well. In ns
	increaseCounter(eventName, U64(event.m_duration * 1000000000.0));
}

void Tracer::increaseCounter(const char* counterName, U64 value)
{
	ANKI_ASSERT(counterName);
	ANKI_ASSERT(isInsideBeginEndFrame());

	ThreadLocal& threadLocal = getThreadLocal();
	Counter* counter = threadLocal.m_counterAlloc.newInstance(m_alloc);
	counter->m_name = counterName;
	counter->m_value = value;
	counter->m_frameIdx = m_frames.getSize() - 1;

	threadLocal.m_counters.pushBack(counter);
}

void Tracer::gatherCounters(FlushCtx& ctx)
{
	// Gather all the counters
	DynamicArrayAuto<Counter> allCounters(m_alloc);
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_counters.isEmpty())
		{
			// Pop counter
			Counter& inCounter = threadLocal->m_counters.getFront();
			threadLocal->m_counters.popFront();

			// Copy
			Counter newCounter = inCounter;
			allCounters.emplaceBack(newCounter);

			// Delete poped counter
			threadLocal->m_counterAlloc.deleteInstance(m_alloc, &inCounter);
		}
	}

	if(allCounters.getSize() == 0)
	{
		// Early exit
		return;
	}

	// Sort them
	std::sort(allCounters.getBegin(), allCounters.getEnd(), [](const Counter& a, const Counter& b) {
		if(a.m_frameIdx != b.m_frameIdx)
		{
			return a.m_frameIdx < b.m_frameIdx;
		}

		ANKI_ASSERT(a.m_name && b.m_name);
		return a.m_name < b.m_name;
	});

	// Compact them
	for(U i = 0; i < allCounters.getSize(); ++i)
	{
		const Counter& inCounter = allCounters[i];

		// Create new frame
		if(ctx.m_counters.getSize() == 0 || ctx.m_counters.getBack().m_frameIdx != inCounter.m_frameIdx)
		{
			ctx.m_counters.emplaceBack(m_alloc);
			ctx.m_counters.getBack().m_frameIdx = inCounter.m_frameIdx;
		}

		PerFrameCounters& crntFrame = ctx.m_counters.getBack();

		// Check if we have a new counter
		if(crntFrame.m_counters.getSize() == 0 || CString(crntFrame.m_counters.getBack().m_name) != inCounter.m_name)
		{
			// Create new counter
			crntFrame.m_counters.emplaceBack(inCounter);

			// Update the counter names
			Bool found = false;
			for(const CString& counterName : ctx.m_counterNames)
			{
				if(counterName == inCounter.m_name)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				ctx.m_counterNames.emplaceBack(CString(inCounter.m_name));
			}
		}
		else
		{
			// Merge counters
			Counter& mergeTo = crntFrame.m_counters.getBack();
			ANKI_ASSERT(CString(mergeTo.m_name) == inCounter.m_name);
			ANKI_ASSERT(mergeTo.m_frameIdx == inCounter.m_frameIdx);
			mergeTo.m_value += inCounter.m_value;
		}
	}

	// Sort the counter names
	ANKI_ASSERT(ctx.m_counterNames.getSize() > 0);
	std::sort(ctx.m_counterNames.getBegin(), ctx.m_counterNames.getEnd(), [](CString a, CString b) { return a < b; });

	// Fill the gaps. Some counters might have not appeared in some frames
	for(PerFrameCounters& perFrame : ctx.m_counters)
	{
		ANKI_ASSERT(perFrame.m_counters.getSize() <= ctx.m_counterNames.getSize());

		for(U i = 0; i < ctx.m_counterNames.getSize(); ++i)
		{
			const CString& counterName = ctx.m_counterNames[i];

			// Try to find the counter
			Bool found = false;
			for(const Counter& c : perFrame.m_counters)
			{
				if(counterName == c.m_name)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{
				// Counter is missing
				Counter missingCounter;
				missingCounter.m_frameIdx = perFrame.m_frameIdx;
				missingCounter.m_name = counterName.cstr();
				missingCounter.m_value = 0;
				perFrame.m_counters.emplaceBack(missingCounter);
			}
		}

		std::sort(perFrame.m_counters.getBegin(), perFrame.m_counters.getEnd(), [](const Counter& a, const Counter& b) {
			ANKI_ASSERT(a.m_name && b.m_name);
			return CString(a.m_name) < CString(b.m_name);
		});

		ANKI_ASSERT(perFrame.m_counters.getSize() == ctx.m_counterNames.getSize());
	}
}

void Tracer::gatherEvents(FlushCtx& ctx)
{
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_events.isEmpty())
		{
			// Pop event
			Event& inEvent = threadLocal->m_events.getFront();
			threadLocal->m_events.popFront();

			// Copy
			Event newEvent = inEvent;
			newEvent.m_tid = threadLocal->m_tid;
			ctx.m_events.emplaceBack(newEvent);

			// Delete poped event
			threadLocal->m_eventAlloc.deleteInstance(m_alloc, &inEvent);
		}
	}

	// Sort them
	std::sort(ctx.m_events.getBegin(), ctx.m_events.getEnd(), [](const Event& a, const Event& b) {
		return a.m_timestamp < b.m_timestamp;
	});
}

Error Tracer::writeTraceJson(const FlushCtx& ctx)
{
	// Open the file
	StringAuto newFname(m_alloc);
	newFname.sprintf("%s_trace.json", ctx.m_filename.cstr());
	File file;
	ANKI_CHECK(file.open(newFname.toCString(), FileOpenFlag::WRITE));

	if(ctx.m_events.getSize() == 0)
	{
		// Early exit
		return Error::NONE;
	}

	ANKI_CHECK(file.writeText("[\n"));

	// Write the events to the file
	for(const Event& event : ctx.m_events)
	{
		const U64 startMicroSec = U64(event.m_timestamp * 1000000.0);
		const U64 durMicroSec = U64(event.m_duration * 1000000.0);

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
	for(U i = 0; i < ctx.m_counters.getSize(); ++i)
	{
		const PerFrameCounters& frame = ctx.m_counters[i];
		const Second startFrameTime = m_frames[frame.m_frameIdx].m_startFrameTime;
		const Second endFrameTime = m_frames[frame.m_frameIdx].m_endFrameTime;

		const Array<Second, 2> timestamps = {{startFrameTime, endFrameTime}};
		const U timestampCount = (i < ctx.m_counters.getSize() - 1) ? 1 : 2;

		for(const Counter& counter : frame.m_counters)
		{
			for(U j = 0; j < timestampCount; ++j)
			{
				ANKI_CHECK(file.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"C\", "
										  "\"pid\": 1, \"ts\": %llu, \"args\": {\"val\": %llu}},\n",
					counter.m_name,
					U64(timestamps[j] * 1000000.0),
					counter.m_value));
			}
		}
	}

	ANKI_CHECK(file.writeText("{}\n]\n"));

	return Error::NONE;
}

Error Tracer::writeCounterCsv(const FlushCtx& ctx)
{
	// Open the file
	StringAuto fname(m_alloc);
	fname.sprintf("%s_counters.csv", ctx.m_filename.cstr());
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));

	if(ctx.m_counters.getSize() == 0)
	{
		// If there are no counters leave the file empty and exit
		return Error::NONE;
	}

	// Write the counter names
	ANKI_CHECK(file.writeText("Frame"));
	for(CString counterName : ctx.m_counterNames)
	{
		ANKI_CHECK(file.writeText(",%s", counterName.cstr()));
	}
	ANKI_CHECK(file.writeText("\n"));

	// Dump the frames
	for(const PerFrameCounters& frame : ctx.m_counters)
	{
		ANKI_CHECK(file.writeText("%llu", m_frames[frame.m_frameIdx].m_frame));

		for(const Counter& c : frame.m_counters)
		{
			ANKI_CHECK(file.writeText(",%llu", c.m_value));
		}

		ANKI_CHECK(file.writeText("\n"));
	}

	return Error::NONE;
}

Error Tracer::flush(CString filename)
{
	ANKI_ASSERT(!isInsideBeginEndFrame());
	FlushCtx ctx(m_alloc, filename);

	gatherCounters(ctx);
	gatherEvents(ctx);

	ANKI_CHECK(writeTraceJson(ctx));
	ANKI_CHECK(writeCounterCsv(ctx));

	m_frames.destroy(m_alloc);

	return Error::NONE;
}

} // end namespace anki
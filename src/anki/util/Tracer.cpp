// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Tracer.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// Lightweight event storage.
class Tracer::Event
{
public:
	const char* m_name;
	Second m_timestamp;
	Second m_duration;
};

/// Event batch allocation.
class Tracer::EventsChunk : public IntrusiveListEnabled<EventsChunk>
{
public:
	Array<Event, EVENTS_PER_CHUNK> m_events;
	U32 m_eventCount = 0;
};

/// A heavyweight event with more info.
class Tracer::GatherEvent
{
public:
	CString m_name;
	Second m_timestamp;
	Second m_duration;
	ThreadId m_tid;
};

/// Lightweight counter storage.
class Tracer::Counter
{
public:
	const char* m_name;
	U64 m_value;
};

/// Counter batch allocation.
class Tracer::CountersChunk : public IntrusiveListEnabled<CountersChunk>
{
public:
	U64 m_frame;
	Second m_startFrameTime;
	Array<Counter, COUNTERS_PER_CHUNK> m_counters;
	U32 m_counterCount = 0;
};

/// Heavyweight counter storage.
class Tracer::GatherCounter
{
public:
	CString m_name;
	U64 m_value;
};

/// Thread local storage.
class Tracer::ThreadLocal
{
public:
	ThreadId m_tid ANKI_DBG_NULLIFY;

	IntrusiveList<CountersChunk> m_counterChunks;
	IntrusiveList<EventsChunk> m_eventChunks;
};

thread_local Tracer::ThreadLocal* Tracer::m_threadLocal = nullptr;

/// Storage of counters per frame.
class Tracer::PerFrameCounters
{
public:
	DynamicArrayAuto<GatherCounter> m_counters;
	DynamicArrayAuto<GatherCounter> m_tempCounters; ///< A temp storage.
	U64 m_frame;
	Second m_startFrameTime;

	PerFrameCounters(GenericMemoryPoolAllocator<U8> alloc)
		: m_counters(alloc)
		, m_tempCounters(alloc)
	{
	}
};

/// Context for Tracer::flush().
class Tracer::FlushCtx
{
public:
	GenericMemoryPoolAllocator<U8> m_alloc;
	CString m_filename;
	DynamicArrayAuto<CString> m_counterNames;
	DynamicArrayAuto<PerFrameCounters> m_counters;
	DynamicArrayAuto<GatherEvent> m_events;

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
		while(!threadLocal->m_counterChunks.isEmpty())
		{
			CountersChunk& chunk = threadLocal->m_counterChunks.getFront();
			threadLocal->m_counterChunks.popFront();
			m_alloc.deleteInstance(&chunk);
		}

		while(!threadLocal->m_eventChunks.isEmpty())
		{
			EventsChunk& chunk = threadLocal->m_eventChunks.getFront();
			threadLocal->m_eventChunks.popFront();
			m_alloc.deleteInstance(&chunk);
		}

		m_alloc.deleteInstance(threadLocal);
	}

	m_allThreadLocal.destroy(m_alloc);
}

void Tracer::newFrame(U64 frame)
{
	ANKI_ASSERT(frame == 0 || frame > m_frame);

	LockGuard<SpinLock> lock(m_frameMtx);

	m_startFrameTime = HighRezTimer::getCurrentTime();
	m_frame = frame;
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
		LockGuard<Mutex> lock(m_threadLocalMtx);
		m_allThreadLocal.emplaceBack(m_alloc, out);
	}

	return *out;
}

TracerEventHandle Tracer::beginEvent()
{
	ThreadLocal& threadLocal = getThreadLocal();

	// Allocate new chunk
	if(threadLocal.m_eventChunks.isEmpty() || threadLocal.m_eventChunks.getBack().m_eventCount >= EVENTS_PER_CHUNK)
	{
		EventsChunk* chunk = m_alloc.newInstance<EventsChunk>();
		threadLocal.m_eventChunks.pushBack(chunk);
	}

	EventsChunk& chunk = threadLocal.m_eventChunks.getBack();
	Event* event = &chunk.m_events[chunk.m_eventCount++];
	event->m_timestamp = HighRezTimer::getCurrentTime();

	return event;
}

void Tracer::endEvent(const char* eventName, TracerEventHandle eventHandle)
{
	ANKI_ASSERT(eventName);
	ANKI_ASSERT(eventHandle);

	Event* event = static_cast<Event*>(eventHandle);
	event->m_name = eventName;
	event->m_duration = HighRezTimer::getCurrentTime() - event->m_timestamp;

	// Store a counter as well. In ns
	increaseCounter(eventName, U64(event->m_duration * 1000000000.0));
}

void Tracer::increaseCounter(const char* counterName, U64 value)
{
	ANKI_ASSERT(counterName);

	ThreadLocal& threadLocal = getThreadLocal();

	// Create chunk
	if(threadLocal.m_counterChunks.isEmpty() || threadLocal.m_counterChunks.getBack().m_frame != m_frame
		|| threadLocal.m_counterChunks.getBack().m_counterCount >= COUNTERS_PER_CHUNK)
	{
		CountersChunk* newChunk = m_alloc.newInstance<CountersChunk>();
		threadLocal.m_counterChunks.pushBack(newChunk);

		{
			LockGuard<SpinLock> lock(m_frameMtx);
			newChunk->m_frame = m_frame;
			newChunk->m_startFrameTime = m_startFrameTime;
		}
	}

	CountersChunk& chunk = threadLocal.m_counterChunks.getBack();

	Counter& counter = chunk.m_counters[chunk.m_counterCount++];
	counter.m_name = counterName;
	counter.m_value = value;
}

void Tracer::gatherCounters(FlushCtx& ctx)
{
	// Iterate all the chunks and create the PerFrameCounters
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_counterChunks.isEmpty())
		{
			// Pop chunk
			CountersChunk& chunk = threadLocal->m_counterChunks.getFront();
			threadLocal->m_counterChunks.popFront();

			// Iterate the PerFrameCounters to find if the frame is present
			PerFrameCounters* perFrame = nullptr;
			for(PerFrameCounters& pf : ctx.m_counters)
			{
				if(pf.m_frame == chunk.m_frame)
				{
					perFrame = &pf;
					break;
				}
			}

			if(!perFrame)
			{
				ctx.m_counters.emplaceBack(m_alloc);

				perFrame = &ctx.m_counters.getBack();
				perFrame->m_frame = chunk.m_frame;
				perFrame->m_startFrameTime = chunk.m_startFrameTime;
			}

			ANKI_ASSERT(chunk.m_frame == perFrame->m_frame);

			// Copy the counters
			for(U i = 0; i < chunk.m_counterCount; ++i)
			{
				const Counter& inCounter = chunk.m_counters[i];

				GatherCounter outCounter;
				outCounter.m_name = inCounter.m_name;
				outCounter.m_value = inCounter.m_value;

				perFrame->m_tempCounters.emplaceBack(outCounter);
			}

			// Delete chunk
			m_alloc.deleteInstance(&chunk);
		}
	}

	if(ctx.m_counters.getSize() == 0)
	{
		// Early exit
		return;
	}

	// Compact the counters and get all counter names
	for(PerFrameCounters& perFrame : ctx.m_counters)
	{
		if(perFrame.m_tempCounters.getSize() == 0)
		{
			continue;
		}

		// Sort counters
		std::sort(perFrame.m_tempCounters.getBegin(),
			perFrame.m_tempCounters.getEnd(),
			[](const GatherCounter& a, const GatherCounter& b) { return a.m_name < b.m_name; });

		// Compact counters
		for(const GatherCounter& tmpCounter : perFrame.m_tempCounters)
		{
			if(perFrame.m_counters.getSize() == 0 || perFrame.m_counters.getBack().m_name != tmpCounter.m_name)
			{
				// Create new counter
				perFrame.m_counters.emplaceBack(tmpCounter);

				// Update the counter names
				Bool found = false;
				for(const CString& counterName : ctx.m_counterNames)
				{
					if(counterName == tmpCounter.m_name)
					{
						found = true;
						break;
					}
				}

				if(!found)
				{
					ctx.m_counterNames.emplaceBack(tmpCounter.m_name);
				}
			}
			else
			{
				// Merge counters
				GatherCounter& mergeTo = perFrame.m_counters.getBack();
				ANKI_ASSERT(mergeTo.m_name == tmpCounter.m_name);
				mergeTo.m_value += tmpCounter.m_value;
			}
		}

		// Free some memory
		perFrame.m_tempCounters.destroy();
	}

	// Sort the counter names
	ANKI_ASSERT(ctx.m_counterNames.getSize() > 0);
	std::sort(ctx.m_counterNames.getBegin(), ctx.m_counterNames.getEnd(), [](CString a, CString b) { return a < b; });

	// Fill the gaps. Some counters might have not appeared in some frames. Those counters need to have a zero value
	// because the CSV wants all counters present on all rows
	for(PerFrameCounters& perFrame : ctx.m_counters)
	{
		ANKI_ASSERT(perFrame.m_counters.getSize() <= ctx.m_counterNames.getSize());

		for(U i = 0; i < ctx.m_counterNames.getSize(); ++i)
		{
			const CString& counterName = ctx.m_counterNames[i];

			// Try to find the counter
			Bool found = false;
			for(const GatherCounter& c : perFrame.m_counters)
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
				GatherCounter missingCounter;
				missingCounter.m_name = counterName;
				missingCounter.m_value = 0;
				perFrame.m_counters.emplaceBack(missingCounter);
			}
		}

		// Sort again
		std::sort(perFrame.m_counters.getBegin(),
			perFrame.m_counters.getEnd(),
			[](const GatherCounter& a, const GatherCounter& b) { return a.m_name < b.m_name; });

		ANKI_ASSERT(perFrame.m_counters.getSize() == ctx.m_counterNames.getSize());
	}
}

void Tracer::gatherEvents(FlushCtx& ctx)
{
	for(ThreadLocal* threadLocal : m_allThreadLocal)
	{
		while(!threadLocal->m_eventChunks.isEmpty())
		{
			// Pop chunk
			EventsChunk& chunk = threadLocal->m_eventChunks.getFront();
			threadLocal->m_eventChunks.popFront();

			// Copy
			for(U i = 0; i < chunk.m_eventCount; ++i)
			{
				const Event& inEvent = chunk.m_events[i];

				GatherEvent outEvent;
				outEvent.m_duration = inEvent.m_duration;
				outEvent.m_name = inEvent.m_name;
				outEvent.m_timestamp = inEvent.m_timestamp;
				outEvent.m_tid = threadLocal->m_tid;

				ctx.m_events.emplaceBack(outEvent);
			}

			// Delete poped chunk
			m_alloc.deleteInstance(&chunk);
		}
	}

	// Sort them
	std::sort(ctx.m_events.getBegin(), ctx.m_events.getEnd(), [](const GatherEvent& a, const GatherEvent& b) {
		if(a.m_timestamp != b.m_timestamp)
		{
			return a.m_timestamp < b.m_timestamp;
		}

		if(a.m_duration != b.m_duration)
		{
			return a.m_duration < b.m_duration;
		}

		return a.m_name < b.m_name;
	});
}

Error Tracer::writeTraceJson(const FlushCtx& ctx)
{
	// Open the file
	StringAuto newFname(m_alloc);
	newFname.sprintf("%s.trace.json", ctx.m_filename.cstr());
	File file;
	ANKI_CHECK(file.open(newFname.toCString(), FileOpenFlag::WRITE));

	if(ctx.m_events.getSize() == 0)
	{
		// Early exit
		return Error::NONE;
	}

	ANKI_CHECK(file.writeText("[\n"));

	// Write the events to the file
	for(const GatherEvent& event : ctx.m_events)
	{
		const U64 startMicroSec = U64(event.m_timestamp * 1000000.0);
		const U64 durMicroSec = U64(event.m_duration * 1000000.0);

		if(durMicroSec == 0)
		{
			continue;
		}

		ANKI_CHECK(file.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
								  "\"pid\": 1, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
			event.m_name.cstr(),
			event.m_tid,
			startMicroSec,
			durMicroSec));
	}

	// Write the counters
	for(U i = 0; i < ctx.m_counters.getSize(); ++i)
	{
		const PerFrameCounters& frame = ctx.m_counters[i];
		const Second startFrameTime = frame.m_startFrameTime;

		// The counters need a range in order to appear. Add a dummy counter for the last frame
		const Array<Second, 2> timestamps = {{startFrameTime, startFrameTime + 1.0}};
		const U timestampCount = (i < ctx.m_counters.getSize() - 1) ? 1 : 2;

		for(const GatherCounter& counter : frame.m_counters)
		{
			for(U j = 0; j < timestampCount; ++j)
			{
				ANKI_CHECK(file.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"C\", "
										  "\"pid\": 1, \"ts\": %llu, \"args\": {\"val\": %llu}},\n",
					counter.m_name.cstr(),
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
	fname.sprintf("%s.counters.csv", ctx.m_filename.cstr());
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
	U rowCount = 0;
	for(const PerFrameCounters& frame : ctx.m_counters)
	{
		ANKI_CHECK(file.writeText("%llu", frame.m_frame));

		for(const GatherCounter& c : frame.m_counters)
		{
			ANKI_CHECK(file.writeText(",%llu", c.m_value));
		}

		ANKI_CHECK(file.writeText("\n"));
		++rowCount;
	}

	// Dump some spreadsheet functions
	ANKI_CHECK(file.writeText("SUM"));
	for(U i = 0; i < ctx.m_counterNames.getSize(); ++i)
	{
		Array<char, 3> columnName;
		getSpreadsheetColumnName(i + 1, columnName);
		ANKI_CHECK(file.writeText(",=SUM(%s2:%s%u)", &columnName[0], &columnName[0], rowCount + 1u));
	}
	ANKI_CHECK(file.writeText("\n"));

	ANKI_CHECK(file.writeText("AVG"));
	for(U i = 0; i < ctx.m_counterNames.getSize(); ++i)
	{
		Array<char, 3> columnName;
		getSpreadsheetColumnName(i + 1, columnName);
		ANKI_CHECK(file.writeText(",=AVERAGE(%s2:%s%u)", &columnName[0], &columnName[0], rowCount + 1u));
	}

	return Error::NONE;
}

Error Tracer::flush(CString filename)
{
	FlushCtx ctx(m_alloc, filename);

	gatherCounters(ctx);
	gatherEvents(ctx);

	ANKI_CHECK(writeTraceJson(ctx));
	ANKI_CHECK(writeCounterCsv(ctx));

	return Error::NONE;
}

void Tracer::getSpreadsheetColumnName(U column, Array<char, 3>& arr)
{
	U major = column / 26;
	U minor = column % 26;

	if(major)
	{
		arr[0] = 'A' + (major - 1);
		arr[1] = 'A' + minor;
	}
	else
	{
		arr[0] = 'A' + minor;
		arr[1] = '\0';
	}

	arr[2] = '\0';
}

} // end namespace anki
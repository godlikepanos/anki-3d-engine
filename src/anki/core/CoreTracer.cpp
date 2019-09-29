// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/CoreTracer.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/Tracer2.h>

namespace anki
{

class CoreTracer::Chunk : public IntrusiveListEnabled<Chunk>
{
public:
	DynamicArrayAuto<Tracer2Event> m_events;
	DynamicArrayAuto<Tracer2Counter> m_counters;
	ThreadId m_tid;

	Chunk(GenericMemoryPoolAllocator<U8>& alloc)
		: m_events(alloc)
		, m_counters(alloc)
	{
	}
};

class CoreTracer::Frame : public IntrusiveListEnabled<Frame>
{
public:
	GenericMemoryPoolAllocator<U8> m_alloc;
	IntrusiveList<Chunk> m_chunks;

	Frame(GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	~Frame()
	{
		while(!m_chunks.isEmpty())
		{
			Chunk* chunk = m_chunks.popFront();
			m_alloc.deleteInstance(chunk);
		}
	}
};

class CoreTracer::PerFrameCounters : public IntrusiveListEnabled<PerFrameCounters>
{
public:
	DynamicArrayAuto<Tracer2Counter> m_counters;

	PerFrameCounters(GenericMemoryPoolAllocator<U8>& alloc)
		: m_counters(alloc)
	{
	}
};

CoreTracer::CoreTracer()
	: m_thread("Tracer")
{
}

CoreTracer::~CoreTracer()
{
	if(m_traceJsonFile.isOpen())
	{
		Error err = m_traceJsonFile.writeText("{}\n]\n");
		(void)err;
	}
}

Error CoreTracer::init(GenericMemoryPoolAllocator<U8> alloc, CString directory)
{
	m_alloc = alloc;
	m_thread.start(this,
		[](ThreadCallbackInfo& info) -> Error { return static_cast<CoreTracer*>(info.m_userData)->threadWorker(); });

	ANKI_CHECK(m_traceJsonFile.open(StringAuto(alloc).sprintf("%s.trace.json", directory.cstr()), FileOpenFlag::WRITE));
	ANKI_CHECK(m_traceJsonFile.writeText("[\n"));

	ANKI_CHECK(
		m_countersCsvFile.open(StringAuto(alloc).sprintf("%s.trace.csv", directory.cstr()), FileOpenFlag::WRITE));

	return Error::NONE;
}

Error CoreTracer::threadWorker()
{
	Error err = Error::NONE;
	Bool quit = false;

	while(!err && !quit)
	{
		Frame* frame = nullptr;

		// Get some work
		{
			// Wait for something
			LockGuard<Mutex> lock(m_mtx);
			while(m_frames.isEmpty() && !m_quit)
			{
				m_cvar.wait(m_mtx);
			}

			// Get some work
			if(!m_frames.isEmpty())
			{
				frame = m_frames.popBack();
			}
			else if(m_quit)
			{
				quit = true;
			}
		}

		// Do some work
		if(frame)
		{
			// Delete frame
			m_alloc.deleteInstance(frame);
		}
	}

	return err;
}

Error CoreTracer::writeEvents(const Frame& frame)
{
	for(const Chunk& chunk : frame.m_chunks)
	{
		// Write events
		for(const Tracer2Event& event : chunk.m_events)
		{
			const U64 startMicroSec = U64(event.m_start * 1000000.0);
			const U64 durMicroSec = U64(event.m_duration * 1000000.0);

			ANKI_CHECK(m_traceJsonFile.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
												 "\"pid\": 1, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
				event.m_name.cstr(),
				chunk.m_tid,
				startMicroSec,
				durMicroSec));
		}

		// Write counters
		// TODO
	}

	return Error::NONE;
}

Error CoreTracer::writeCounters(const Frame& frame)
{
	// Gather everything
	DynamicArrayAuto<Tracer2Counter> allCounters(m_alloc);
	for(const Chunk& chunk : frame.m_chunks)
	{
		for(const Tracer2Counter& counter : chunk.m_counters)
		{
			allCounters.emplaceBack(counter);
		}
	}

	if(allCounters.getSize() == 0)
	{
		return Error::NONE;
	}

	// Sort
	std::sort(allCounters.getBegin(), allCounters.getEnd(), [](const Tracer2Counter& a, const Tracer2Counter& b) {
		return a.m_name < b.m_name;
	});

	// Merge same
	DynamicArrayAuto<Tracer2Counter> mergedCounters(m_alloc);
	for(U32 i = 0; i < allCounters.getSize(); ++i)
	{
		if(mergedCounters.getBack().m_name != allCounters[i].m_name)
		{
			// New
			mergedCounters.emplaceBack(allCounters[i]);
		}
		else
		{
			// Merge
			mergedCounters.getBack().m_value += allCounters[i].m_name;
		}
	}
	ANKI_ASSERT(mergedCounters.getSize() > 0);

	// Write values
	// TODO

	return Error::NONE;
}

} // end namespace anki

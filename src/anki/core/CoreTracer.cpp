// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/CoreTracer.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/Tracer2.h>

namespace anki
{

class CoreTracer::ThreadWorkSubItem : public IntrusiveListEnabled<ThreadWorkSubItem>
{
public:
	DynamicArrayAuto<Tracer2Event> m_events;
	DynamicArrayAuto<Tracer2Counter> m_counters;
	ThreadId m_tid;

	ThreadWorkSubItem(GenericMemoryPoolAllocator<U8>& alloc)
		: m_events(alloc)
		, m_counters(alloc)
	{
	}
};

class CoreTracer::ThreadWorkItem : public IntrusiveListEnabled<ThreadWorkItem>
{
public:
	GenericMemoryPoolAllocator<U8> m_alloc;
	IntrusiveList<ThreadWorkSubItem> m_chunks;

	ThreadWorkItem(GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
	{
	}

	~ThreadWorkItem()
	{
		while(!m_chunks.isEmpty())
		{
			ThreadWorkSubItem* sitem = m_chunks.popFront();
			m_alloc.deleteInstance(sitem);
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
		ThreadWorkItem* item = nullptr;

		// Get some work
		{
			// Wait for something
			LockGuard<Mutex> lock(m_mtx);
			while(m_workItems.isEmpty() && !m_quit)
			{
				m_cvar.wait(m_mtx);
			}

			// Get some work
			if(!m_workItems.isEmpty())
			{
				item = m_workItems.popBack();
			}
			else if(m_quit)
			{
				quit = true;
			}
		}

		// Do some work using the frame and delete it
		if(item)
		{
			err = writeEvents(*item);

			if(!err)
			{
				err = writeCounters(*item);
			}

			m_alloc.deleteInstance(item);
		}
	}

	return err;
}

Error CoreTracer::writeEvents(const ThreadWorkItem& item)
{
	for(const ThreadWorkSubItem& sitem : item.m_chunks)
	{
		// Write events
		for(const Tracer2Event& event : sitem.m_events)
		{
			const U64 startMicroSec = U64(event.m_start * 1000000.0);
			const U64 durMicroSec = U64(event.m_duration * 1000000.0);

			ANKI_CHECK(m_traceJsonFile.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
												 "\"pid\": 1, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
				event.m_name.cstr(),
				sitem.m_tid,
				startMicroSec,
				durMicroSec));
		}

		// Store counters
		// TODO
	}

	return Error::NONE;
}

Error CoreTracer::writeCounters(const ThreadWorkItem& item)
{
	// Gather everything
	DynamicArrayAuto<Tracer2Counter> allCounters(m_alloc);
	for(const ThreadWorkSubItem& sitem : item.m_chunks)
	{
		for(const Tracer2Counter& counter : sitem.m_counters)
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

	// Add missing counter names
	Bool addedCounterName = false;
	for(U32 i = 0; i < mergedCounters.getSize(); ++i)
	{
		const Tracer2Counter& counter = mergedCounters[i];

		Bool found = false;
		for(const String& name : m_counterNames)
		{
			if(name == counter.m_name)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			m_counterNames.emplaceBack(m_alloc, m_alloc, counter.m_name);
			addedCounterName = true;
		}
	}

	if(addedCounterName)
	{
		std::sort(m_counterNames.getBegin(), m_counterNames.getEnd());
	}

	// Store the counters
	PerFrameCounters* newPerFrame = m_alloc.newInstance<PerFrameCounters>(m_alloc);
	newPerFrame->m_counters = std::move(mergedCounters);
	m_frameCounters.pushBack(newPerFrame);

	return Error::NONE;
}

void CoreTracer::newFrame(U64 frame)
{
	// TODO
}

} // end namespace anki

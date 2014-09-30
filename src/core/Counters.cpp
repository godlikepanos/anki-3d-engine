// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/Config.h"

#if ANKI_ENABLE_COUNTERS

#include "anki/core/Counters.h"
#include "anki/core/Timestamp.h"
#include "anki/core/App.h"
#include "anki/util/Array.h"
#include <cstring>

namespace anki {

//==============================================================================

enum CounterFlag
{
	CF_PER_FRAME = 1 << 0,
	CF_PER_RUN = 1 << 1,
	CF_FPS = 1 << 2, ///< Only with CF_PER_RUN
	CF_F64 = 1 << 3,
	CF_U64 = 1 << 4
};

class CounterInfo
{
public:
	const char* m_name;
	U32 m_flags;
};

static const Array<CounterInfo, (U)Counter::COUNT> cinfo = {{
	{"FPS", CF_PER_RUN | CF_FPS | CF_F64},
	{"MAIN_RENDERER_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_MS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_IS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_PPS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_SHADOW_PASSES", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"RENDERER_LIGHTS_COUNT", CF_PER_RUN | CF_U64},
	{"SCENE_UPDATE_TIME", CF_PER_RUN | CF_F64},
	{"SWAP_BUFFERS_TIME", CF_PER_RUN | CF_F64},
	{"GL_CLIENT_WAIT_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"GL_SERVER_WAIT_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"GL_DRAWCALLS_COUNT", CF_PER_RUN | CF_U64},
	{"GL_VERTICES_COUNT", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GL_QUEUES_SIZE", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GL_CLIENT_BUFFERS_SIZE", CF_PER_FRAME | CF_PER_RUN | CF_U64}
}};

#define MAX_NAME "24"

//==============================================================================
CountersManager::CountersManager(
	HeapAllocator<U8>& alloc, const CString& cacheDir)
:	m_perframeValues(alloc),
	m_perrunValues(alloc),
	m_counterTimes(alloc)
{
	m_perframeValues.resize((U)Counter::COUNT, 0);
	m_perrunValues.resize((U)Counter::COUNT, 0);
	m_counterTimes.resize((U)Counter::COUNT, 0.0);

	// Open and write the headers to the files
	m_perframeFile.open(
		(String(cacheDir, alloc) + "/frame_counters.csv").toCString(),
		File::OpenFlag::WRITE);

	m_perframeFile.writeText("FRAME");

	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_FRAME)
		{
			m_perframeFile.writeText(", %s", inf.m_name);
		}
	}

	// Open and write the headers to the other file
	m_perrunFile.open(
		(String(cacheDir, alloc) + "/run_counters.csv").toCString(),
		File::OpenFlag::WRITE);

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_RUN)
		{
			if(i != 0)
			{
				m_perrunFile.writeText(", %" MAX_NAME "s", inf.m_name);
			}
			else
			{
				m_perrunFile.writeText("%" MAX_NAME "s", inf.m_name);
			}

			++i;
		}
	}
	m_perrunFile.writeText("\n");
}

//==============================================================================
CountersManager::~CountersManager()
{}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, U64 val)
{
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_U64);

	if(cinfo[(U)counter].m_flags & CF_PER_FRAME)
	{
		m_perframeValues[(U)counter] += val;
	}

	if(cinfo[(U)counter].m_flags & CF_PER_RUN)
	{
		m_perrunValues[(U)counter] += val;
	}
}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, F64 val)
{
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_F64);
	F64 f;
	memcpy(&f, &val, sizeof(F64));

	if(cinfo[(U)counter].m_flags & CF_PER_FRAME)
	{
		*(F64*)(&m_perframeValues[(U)counter]) += f;
	}

	if(cinfo[(U)counter].m_flags & CF_PER_RUN)
	{
		*(F64*)(&m_perrunValues[(U)counter]) += f;
	}
}

//==============================================================================
void CountersManager::startTimer(Counter counter)
{
	// The counter should be F64
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_F64);
	// The timer should have beeb reseted
	ANKI_ASSERT(m_counterTimes[(U)counter] == 0.0);

	m_counterTimes[(U)counter] = HighRezTimer::getCurrentTime();
}

//==============================================================================
void CountersManager::stopTimerIncreaseCounter(Counter counter)
{
	// The counter should be F64
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_F64);
	// The timer should have started
	ANKI_ASSERT(m_counterTimes[(U)counter] > 0.0);

	F32 prevTime = m_counterTimes[(U)counter];
	m_counterTimes[(U)counter] = 0.0;

	increaseCounter(counter, HighRezTimer::getCurrentTime() - prevTime);
}

//==============================================================================
void CountersManager::resolveFrame()
{
	// Write new line and frame no
	m_perframeFile.writeText("\n%llu", getGlobTimestamp());

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_FRAME)
		{
			if(inf.m_flags & CF_U64)
			{
				m_perframeFile.writeText(", %llu", m_perframeValues[i]);
			}
			else if(inf.m_flags & CF_F64)
			{
				m_perframeFile.writeText(", %f", *((F64*)&m_perframeValues[i]));
			}
			else
			{
				ANKI_ASSERT(0);
			}

			m_perframeValues[i] = 0;
		}

		i++;
	}
}

//==============================================================================
void CountersManager::flush()
{
	// Resolve per run counters
	U i = 0;
	U j = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_RUN)
		{
			if(j != 0)
			{
				m_perrunFile.writeText(", ");
			}

			if(inf.m_flags & CF_U64)
			{
				m_perrunFile.writeText("%" MAX_NAME "llu", m_perrunValues[i]);
			}
			else if(inf.m_flags & CF_F64)
			{
				if(inf.m_flags & CF_FPS)
				{
					m_perrunFile.writeText("%" MAX_NAME "f", 
						(F64)getGlobTimestamp() / *((F64*)&m_perrunValues[i]));
				}
				else
				{
					m_perrunFile.writeText("%" MAX_NAME "f", 
						*((F64*)&m_perrunValues[i]));
				}
			}
			else
			{
				ANKI_ASSERT(0);
			}

			m_perrunValues[i] = 0;
			++j;
		}

		++i;
	}
	m_perrunFile.writeText("\n");

	// Close and flush files
	m_perframeFile.close();
	m_perrunFile.close();
}

//==============================================================================
// TraceManager                                                                =
//==============================================================================

namespace detail {

//==============================================================================
TraceManager::TraceManager(HeapAllocator<U8>& alloc, const CString& storeDir)
{
	String filename(storeDir, alloc);
	filename += "/trace";
	m_traceFile.open(filename.toCString(), File::OpenFlag::WRITE);

	m_traceFile.writeText(
		"thread_id, depth, event_name, start_time, stop_time\n");
}

//==============================================================================
TraceManager::~TraceManager()
{
	flushAll();
}

//==============================================================================
void TraceManager::flush(ThreadTraceManager& thread)
{
	ANKI_ASSERT(thread.m_depth == 0);

	LockGuard<Mutex> lock(m_fileMtx);
	for(U i = 0; i < thread.m_bufferedEventsCount; i++)
	{
		const ThreadTraceManager::Event& event = thread.m_bufferedEvents[i];

		m_traceFile.writeText("%u, %u, %s, %f, %f\n", 
			thread.m_id,
			event.m_depth,
			event.m_name,
			event.m_startTime,
			event.m_stopTime);
	}

	thread.m_bufferedEventsCount = 0;
}

//==============================================================================
void TraceManager::flushAll()
{
	auto count = m_threadCount.load();
	while(count-- != 0)
	{
		flush(*m_threadData[count]);
	}

	m_traceFile.close();
}

//==============================================================================
ThreadTraceManager::ThreadTraceManager()
{
	TraceManager& master = TraceManagerSingleton::get();
	m_master = &master;

	auto index = master.m_threadCount.fetch_add(1);
	master.m_threadData[index] = this;
	m_id = index;
}

//==============================================================================
void ThreadTraceManager::pushEvent(const char* name)
{
	Event& event = m_inflightEvents[m_depth];
	event.m_depth = m_depth++;
	event.m_name = name;
	event.m_startTime = HighRezTimer::getCurrentTime();
}

//==============================================================================
void ThreadTraceManager::popEvent()
{
	ANKI_ASSERT(m_depth > 0);
	Event& event = m_inflightEvents[--m_depth];
	event.m_stopTime = HighRezTimer::getCurrentTime();

	ANKI_ASSERT(m_bufferedEventsCount < m_bufferedEvents.getSize());
	m_bufferedEvents[m_bufferedEventsCount++] = event;

	if(m_bufferedEventsCount == m_bufferedEvents.getSize())
	{
		m_master->flush(*this);
	}
}

} // end namespace detail

} // end namespace anki

#endif // ANKI_ENABLE_COUNTERS


// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Trace.h>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

static Array<const char*, U(TraceEventType::COUNT)> eventNames = {{
	"SCENE_UPDATE",
	"SCENE_DELETE_STUFF",
	"SCENE_PHYSICS_UPDATE",
	"SCENE_NODES_UPDATE",
	"SCENE_VISIBILITY_TESTS",
	"SCENE_VISIBILITY_TEST",
	"SCENE_VISIBILITY_COMBINE_RESULTS",
	"RENDER",
	"RENDER_MS",
	"RENDER_IS",
	"RENDER_SM",
	"RENDER_DRAWER",
	"GL_THREAD",
	"SWAP_BUFFERS",
	"IDLE"
}};

static Array<const char*, U(TraceCounterType::COUNT)> counterNames = {{
	"GR_DRAWCALLS",
	"GR_DYNAMIC_UNIFORMS_SIZE"
	"RENDERER_LIGHT_COUNT",
}};

#define ANKI_TRACE_FILE_ERROR() \
	if(err) { \
		ANKI_LOGE("Error writing the trace file"); \
	}

const U MAX_EVENTS_DEPTH = 10;
thread_local HighRezTimer::Scalar g_traceEventStartTime[MAX_EVENTS_DEPTH];
thread_local I g_traceEventsInFlight = 0;

//==============================================================================
// TraceManager                                                                =
//==============================================================================

//==============================================================================
TraceManager::~TraceManager()
{
	if(m_file.isOpen())
	{
		Error err = m_file.writeText("{\"name\": \"dummy\", \"cat\": \"PERF\", "
			"\"ph\": \"X\", \"pid\": 666, \"tid\": %llu, \"ts\": 0, "
			"\"dur\": 1}]}", Thread::getCurrentThreadId());

		ANKI_TRACE_FILE_ERROR();
	}
}

//==============================================================================
Error TraceManager::create(HeapAllocator<U8> alloc, const CString& cacheDir)
{
	// Open and write some stuff to the file
	StringAuto fname(alloc);
	fname.sprintf("%s/trace.json", &cacheDir[0]);

	ANKI_CHECK(m_file.open(fname.toCString(), File::OpenFlag::WRITE));
	ANKI_CHECK(m_file.writeText(
		"{\n"
		"\"displayTimeUnit\": \"ms\",\n"
		"\"traceEvents\": [\n"));

	return ErrorCode::NONE;
}

//==============================================================================
void TraceManager::startEvent()
{
	I i = ++g_traceEventsInFlight;
	--i;
	ANKI_ASSERT(i > 0 && <= MAX_EVENTS_DEPTH);

	g_traceEventStartTime[i] = HighRezTimer::getCurrentTime();
}

//==============================================================================
void TraceManager::stopEvent(TraceEventType type)
{
	I i = --g_traceEventsInFlight;
	ANKI_ASSERT(i >= 0 && < MAX_EVENTS_DEPTH);
	auto startedTime = g_traceEventStartTime[i];

	U id = m_count.fetchAdd(1);
	if(id < BUFFERED_ENTRIES)
	{
		auto now = HighRezTimer::getCurrentTime();
		m_entries[id] = Entry{type, startedTime, now - startedTime,
			Thread::getCurrentThreadId()};
	}
	else
	{
		ANKI_LOGW("Increase the buffered trace entries");
	}
}

//==============================================================================
void TraceManager::flushCounters()
{
	for(U i = 0; i < U(TraceCounterType::COUNT); ++i)
	{
		auto count =
			m_perFrameCounters[U(TraceEventType::COUNT) + i].exchange(0);


		Error err = m_file.writeText(
			"{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"C\", "
			"\"pid\": 666, \"ts\": %llu, \"args\": {\"val\": %llu}},\n",
			counterNames[i], U64(m_startFrameTime * 1000000.0), count);

		ANKI_TRACE_FILE_ERROR();
	}
}

//==============================================================================
void TraceManager::stopFrame()
{
	flushCounters();

	// Write the FPS counter
	HighRezTimer::Scalar now = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar time = now - m_startFrameTime;
	F32 fps = 1.0 / time;
	Error err = m_file.writeText(
		"{\"name\": \"FPS\", \"cat\": \"PERF\", \"ph\": \"C\", "
		"\"pid\": 666, \"ts\": %llu, \"args\": {\"val\": %f}},\n",
		U64(m_startFrameTime * 1000000.0), fps);

	ANKI_TRACE_FILE_ERROR();

	// Write the events
	U count = m_count.exchange(0);
	count = min<U>(count, BUFFERED_ENTRIES);

	for(U i = 0; i < count; ++i)
	{
		const Entry& e = m_entries[i];

		Error err = m_file.writeText(
			"{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
			"\"pid\": 666, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
			eventNames[e.m_event], e.m_tid, U64(e.m_timestamp * 1000000.0),
			U64(e.m_duration * 1000000.0));

		ANKI_TRACE_FILE_ERROR();
	}
}

} // end namespace anki

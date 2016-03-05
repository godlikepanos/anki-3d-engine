// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Trace.h>

#if ANKI_ENABLE_TRACE

namespace anki
{

//==============================================================================
// Misc                                                                        =
//==============================================================================

static Array<const char*, U(TraceEventType::COUNT)> eventNames = {
	{"SCENE_UPDATE",
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
		"RENDER_IR",
		"RENDER_DRAWER",
		"RENDERER_COMMAND_BUFFER_BUILDING",
		"GL_THREAD",
		"SWAP_BUFFERS",
		"BARRIER_WAIT"}};

static Array<const char*, U(TraceCounterType::COUNT)> counterNames = {
	{"GR_DRAWCALLS",
		"GR_DYNAMIC_UNIFORMS_SIZE",
		"GR_DYNAMIC_STORAGE_SIZE",
		"GR_VERTICES",
		"RENDERER_LIGHTS",
		"RENDERER_SHADOW_PASSES",
		"RENDERER_MERGED_DRAWCALLS",
		"RENDERER_REFLECTIONS",
		"SCENE_NODES_UPDATED"}};

#define ANKI_TRACE_FILE_ERROR()                                                \
	if(err)                                                                    \
	{                                                                          \
		ANKI_LOGE("Error writing the trace file");                             \
	}

const U MAX_EVENTS_DEPTH = 20;
thread_local HighRezTimer::Scalar g_traceEventStartTime[MAX_EVENTS_DEPTH];
thread_local I g_traceEventsInFlight = 0;

//==============================================================================
// TraceManager                                                                =
//==============================================================================

//==============================================================================
TraceManager::~TraceManager()
{
	if(m_traceFile.isOpen())
	{
		Error err = m_traceFile.writeText(
			"{\"name\": \"dummy\", "
			"\"cat\": \"PERF\", \"ph\": \"X\", \"pid\": 666, \"tid\": %llu, "
			"\"ts\": 0, \"dur\": 1}]}",
			Thread::getCurrentThreadId());

		ANKI_TRACE_FILE_ERROR();
	}
}

//==============================================================================
Error TraceManager::create(HeapAllocator<U8> alloc, const CString& cacheDir)
{
	// Create trace file
	StringAuto fname(alloc);
	fname.sprintf("%s/trace.json", &cacheDir[0]);

	ANKI_CHECK(m_traceFile.open(fname.toCString(), File::OpenFlag::WRITE));
	ANKI_CHECK(m_traceFile.writeText("{\n"
									 "\"displayTimeUnit\": \"ms\",\n"
									 "\"traceEvents\": [\n"));

	// Create per frame file
	StringAuto perFrameFname(alloc);
	perFrameFname.sprintf("%s/per_frame.csv", &cacheDir[0]);
	ANKI_CHECK(
		m_perFrameFile.open(perFrameFname.toCString(), File::OpenFlag::WRITE));

	ANKI_CHECK(m_perFrameFile.writeText("FPS, "));
	for(U i = 0; i < U(TraceCounterType::COUNT); ++i)
	{
		ANKI_CHECK(m_perFrameFile.writeText("%s, ", counterNames[i]));
	}

	for(U i = 0; i < U(TraceEventType::COUNT); ++i)
	{
		const char* fmt = (i < U(TraceEventType::COUNT) - 1) ? "%s, " : "%s\n";
		ANKI_CHECK(m_perFrameFile.writeText(fmt, eventNames[i]));
	}

	return ErrorCode::NONE;
}

//==============================================================================
void TraceManager::startEvent()
{
	I i = ++g_traceEventsInFlight;
	--i;
	ANKI_ASSERT(i >= 0 && i <= I(MAX_EVENTS_DEPTH));

	g_traceEventStartTime[i] = HighRezTimer::getCurrentTime();
}

//==============================================================================
void TraceManager::stopEvent(TraceEventType type)
{
	ANKI_ASSERT(g_traceEventsInFlight > 0
		&& g_traceEventsInFlight < I(MAX_EVENTS_DEPTH));
	I i = --g_traceEventsInFlight;
	ANKI_ASSERT(i >= 0 && i < I(MAX_EVENTS_DEPTH));
	auto startedTime = g_traceEventStartTime[i];

	U id = m_count.fetchAdd(1);
	if(id < BUFFERED_ENTRIES)
	{
		auto now = HighRezTimer::getCurrentTime();
		auto dur = now - startedTime;

		m_entries[id] =
			Entry{type, startedTime, dur, Thread::getCurrentThreadId()};

		m_perFrameCounters[U(TraceCounterType::COUNT) + U(type)].fetchAdd(
			U64(dur * 1000000000.0));
	}
	else
	{
		ANKI_LOGW("Increase the buffered trace entries");
		m_perFrameCounters[U(TraceCounterType::COUNT) + U(type)].fetchAdd(0);
	}
}

//==============================================================================
Error TraceManager::flushCounters()
{
	// Write the FPS counter
	HighRezTimer::Scalar now = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar time = now - m_startFrameTime;
	F32 fps = 1.0 / time;
	ANKI_CHECK(m_traceFile.writeText(
		"{\"name\": \"FPS\", \"cat\": \"PERF\", \"ph\": \"C\", "
		"\"pid\": 666, \"ts\": %llu, \"args\": {\"val\": %f}},\n",
		U64(m_startFrameTime * 1000000.0),
		fps));

	ANKI_CHECK(m_perFrameFile.writeText("%f, ", fps));

	for(U i = 0; i < U(TraceCounterType::COUNT); ++i)
	{
		auto count = m_perFrameCounters[i].exchange(0);

		ANKI_CHECK(m_traceFile.writeText(
			"{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"C\", "
			"\"pid\": 666, \"ts\": %llu, \"args\": {\"val\": %llu}},\n",
			counterNames[i],
			U64(m_startFrameTime * 1000000.0),
			count));

		ANKI_CHECK(m_perFrameFile.writeText("%llu, ", count));
	}

	return ErrorCode::NONE;
}

//==============================================================================
Error TraceManager::flushEvents()
{
	// Write the events
	U count = m_count.exchange(0);
	count = min<U>(count, BUFFERED_ENTRIES);

	for(U i = 0; i < count; ++i)
	{
		const Entry& e = m_entries[i];

		ANKI_CHECK(m_traceFile.writeText(
			"{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
			"\"pid\": 666, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
			eventNames[e.m_event],
			e.m_tid,
			U64(e.m_timestamp * 1000000.0),
			U64(e.m_duration * 1000000.0)));
	}

	for(U i = 0; i < U(TraceEventType::COUNT); ++i)
	{
		const char* fmt = (i < U(TraceEventType::COUNT) - 1) ? "%f, " : "%f\n";
		U64 ns = m_perFrameCounters[i + U(TraceCounterType::COUNT)].exchange(0);
		ANKI_CHECK(m_perFrameFile.writeText(fmt, F64(ns) / 1000000.0));
	}

	return ErrorCode::NONE;
}

//==============================================================================
void TraceManager::stopFrame()
{
	Error err = flushCounters();

	if(!err)
	{
		err = flushEvents();
	}

	if(err)
	{
		ANKI_LOGE("Error writing the trace file");
	}
}

} // end namespace anki

#endif

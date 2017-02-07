// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/Trace.h>
#include <cstdlib>

#if ANKI_ENABLE_TRACE

namespace anki
{

static Array<const char*, U(TraceEventType::COUNT)> eventNames = {{"SCENE_UPDATE",
	"SCENE_DELETE_STUFF",
	"SCENE_PHYSICS_UPDATE",
	"SCENE_NODES_UPDATE",
	"SCENE_VISIBILITY_TESTS",
	"VIS_TEST",
	"VIS_COMBINE_RESULTS",
	"VIS_ITERATE_SECTORS",
	"VIS_GATHER_TRIANGLES",
	"VIS_RASTERIZE",
	"VIS_RASTERIZER_TEST",
	"RENDER",
	"RENDER_MS",
	"RENDER_IS",
	"RENDER_SM",
	"RENDER_IR",
	"RENDER_DRAWER",
	"RENDERER_COMMAND_BUFFER_BUILDING",
	"RENDERER_LIGHT_BINNING",
	"GL_THREAD",
	"GL_2ND_LEVEL_CMD_BUFFER",
	"GL_BIND_RESOURCES",
	"GL_BIND_PPLINE",
	"GL_CMD_BUFFER_DESTROY",
	"VK_ACQUIRE_IMAGE",
	"VK_QUEUE_SUBMIT",
	"SWAP_BUFFERS",
	"BARRIER_WAIT"}};

static Array<const char*, U(TraceCounterType::COUNT)> counterNames = {{"GR_DRAWCALLS",
	"GR_VERTICES",
	"GL_PROGS_SKIPPED",
	"VK_PIPELINES_CREATED",
	"VK_PIPELINE_BARRIERS",
	"VK_CMD_BUFFER_CREATE",
	"VK_FENCE_CREATE",
	"VK_SEMAPHORE_CREATE",
	"RENDERER_LIGHTS",
	"RENDERER_SHADOW_PASSES",
	"RENDERER_MERGED_DRAWCALLS",
	"RENDERER_REFLECTIONS",
	"RESOURCE_ASYNC_TASKS",
	"SCENE_NODES_UPDATED",
	"STAGING_UNIFORMS_SIZE",
	"STAGING_STORAGE_SIZE"}};

#define ANKI_TRACE_FILE_ERROR()                                                                                        \
	if(err)                                                                                                            \
	{                                                                                                                  \
		ANKI_CORE_LOGE("Error writing the trace file");                                                                \
	}

const U MAX_EVENTS_DEPTH = 20;
thread_local HighRezTimer::Scalar g_traceEventStartTime[MAX_EVENTS_DEPTH];
thread_local I g_traceEventsInFlight = 0;

TraceManager::~TraceManager()
{
	// No need to close the json (no need to add ']'). Chrome will take care of that
}

Error TraceManager::create(HeapAllocator<U8> alloc, const CString& cacheDir)
{
	if(getenv("ANKI_DISABLE_TRACE") && CString(getenv("ANKI_DISABLE_TRACE")) == "1")
	{
		m_disabled = true;
		return ErrorCode::NONE;
	}

	memset(&m_perFrameCounters[0], 0, sizeof(m_perFrameCounters));
	memset(&m_perRunCounters[0], 0, sizeof(m_perRunCounters));

	// Create trace file
	StringAuto fname(alloc);
	fname.sprintf("%s/trace.json", &cacheDir[0]);

	ANKI_CHECK(m_traceFile.open(fname.toCString(), FileOpenFlag::WRITE));
	ANKI_CHECK(m_traceFile.writeText("["));

	// Create per frame file
	StringAuto perFrameFname(alloc);
	perFrameFname.sprintf("%s/per_frame.csv", &cacheDir[0]);
	ANKI_CHECK(m_perFrameFile.open(perFrameFname.toCString(), FileOpenFlag::WRITE));

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

void TraceManager::startEvent()
{
	if(ANKI_UNLIKELY(m_disabled))
	{
		return;
	}

	I i = ++g_traceEventsInFlight;
	--i;
	ANKI_ASSERT(i >= 0 && i <= I(MAX_EVENTS_DEPTH));

	g_traceEventStartTime[i] = HighRezTimer::getCurrentTime();
}

void TraceManager::stopEvent(TraceEventType type)
{
	if(ANKI_UNLIKELY(m_disabled))
	{
		return;
	}

	ANKI_ASSERT(g_traceEventsInFlight > 0 && g_traceEventsInFlight < I(MAX_EVENTS_DEPTH));
	I i = --g_traceEventsInFlight;
	ANKI_ASSERT(i >= 0 && i < I(MAX_EVENTS_DEPTH));
	auto startedTime = g_traceEventStartTime[i];

	U id = m_count.fetchAdd(1);
	if(id < BUFFERED_ENTRIES)
	{
		auto now = HighRezTimer::getCurrentTime();
		auto dur = now - startedTime;

		m_entries[id] = Entry{type, startedTime, dur, Thread::getCurrentThreadId()};

		m_perFrameCounters[U(TraceCounterType::COUNT) + U(type)].fetchAdd(U64(dur * 1000000000.0));
	}
	else
	{
		ANKI_CORE_LOGW("Increase the buffered trace entries");
		m_perFrameCounters[U(TraceCounterType::COUNT) + U(type)].fetchAdd(0);
	}
}

Error TraceManager::flushCounters()
{
	if(ANKI_UNLIKELY(m_disabled))
	{
		return ErrorCode::NONE;
	}

	// Write the FPS counter
	HighRezTimer::Scalar now = HighRezTimer::getCurrentTime();
	HighRezTimer::Scalar time = now - m_startFrameTime;
	F32 fps = 1.0 / time;
	ANKI_CHECK(m_traceFile.writeText("{\"name\": \"FPS\", \"cat\": \"PERF\", \"ph\": \"C\", "
									 "\"pid\": 1, \"ts\": %llu, \"args\": {\"val\": %f}},\n",
		U64(m_startFrameTime * 1000000.0),
		fps));

	ANKI_CHECK(m_perFrameFile.writeText("%f, ", fps));

	for(U i = 0; i < U(TraceCounterType::COUNT); ++i)
	{
		auto count = m_perFrameCounters[i].exchange(0);

		ANKI_CHECK(m_traceFile.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"C\", "
										 "\"pid\": 1, \"ts\": %llu, \"args\": {\"val\": %llu}},\n",
			counterNames[i],
			U64(m_startFrameTime * 1000000.0),
			count));

		ANKI_CHECK(m_perFrameFile.writeText("%llu, ", count));
	}

	return ErrorCode::NONE;
}

Error TraceManager::flushEvents()
{
	if(ANKI_UNLIKELY(m_disabled))
	{
		return ErrorCode::NONE;
	}

	// Write the events
	U count = m_count.exchange(0);
	count = min<U>(count, BUFFERED_ENTRIES);

	for(U i = 0; i < count; ++i)
	{
		const Entry& e = m_entries[i];

		U64 startMicroSec = U64(e.m_timestamp * 1000000.0);
		U64 durMicroSec = U64(e.m_duration * 1000000.0);

		if(durMicroSec == 0)
		{
			continue;
		}

		ANKI_CHECK(m_traceFile.writeText("{\"name\": \"%s\", \"cat\": \"PERF\", \"ph\": \"X\", "
										 "\"pid\": 1, \"tid\": %llu, \"ts\": %llu, \"dur\": %llu},\n",
			eventNames[e.m_event],
			e.m_tid,
			startMicroSec,
			durMicroSec));
	}

	for(U i = 0; i < U(TraceEventType::COUNT); ++i)
	{
		const char* fmt = (i < U(TraceEventType::COUNT) - 1) ? "%f, " : "%f\n";
		U64 ns = m_perFrameCounters[i + U(TraceCounterType::COUNT)].exchange(0);
		ANKI_CHECK(m_perFrameFile.writeText(fmt, F64(ns) / 1000000.0)); // Time in ms
	}

	return ErrorCode::NONE;
}

void TraceManager::startFrame()
{
	if(ANKI_UNLIKELY(m_disabled))
	{
		return;
	}

	m_startFrameTime = HighRezTimer::getCurrentTime();
}

void TraceManager::stopFrame()
{
	if(ANKI_UNLIKELY(m_disabled))
	{
		return;
	}

	Error err = flushCounters();

	if(!err)
	{
		err = flushEvents();
	}

	if(err)
	{
		ANKI_CORE_LOGE("Error writing the trace file");
	}
}

} // end namespace anki

#endif

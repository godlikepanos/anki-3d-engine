// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Singleton.h>
#include <anki/util/DArray.h>
#include <anki/util/File.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/Atomic.h>
#include <anki/util/Thread.h>
#include <anki/core/Timestamp.h>

namespace anki {

/// Enumeration of counters
enum class Counter
{
	FPS,
	MAIN_RENDERER_TIME,
	RENDERER_MS_TIME,
	RENDERER_IS_TIME,
	RENDERER_PPS_TIME,
	RENDERER_SHADOW_PASSES,
	RENDERER_LIGHTS_COUNT,
	RENDERER_MERGED_DRAWCALLS,
	SCENE_UPDATE_TIME,
	SWAP_BUFFERS_TIME,
	GL_CLIENT_WAIT_TIME,
	GL_SERVER_WAIT_TIME,
	GL_DRAWCALLS_COUNT,
	GL_VERTICES_COUNT,
	GL_QUEUES_SIZE,
	GL_CLIENT_BUFFERS_SIZE,
	GR_DYNAMIC_UNIFORMS_SIZE,
	GR_DYNAMIC_STORAGE_SIZE,

	COUNT
};

/// The counters manager. It's been used with a singleton
class CountersManager
{
public:
	CountersManager()
	{}

	~CountersManager();

	ANKI_USE_RESULT Error create(
		HeapAllocator<U8> alloc, const CString& cacheDir,
		const Timestamp* globalTimestamp);

	void increaseCounter(Counter counter, F64 val);
	void increaseCounter(Counter counter, U64 val);

	/// Write the counters of the frame. Should be called after swapbuffers
	void resolveFrame();

	/// Resolve all counters and closes the files. Should be called when app
	/// terminates
	void flush();

	/// Start a timer of a specific counter
	void startTimer(Counter counter);

	/// Stop the timer and increase the counter
	void stopTimerIncreaseCounter(Counter counter);

private:
	union Value
	{
		F64 m_float;
		U64 m_int;
	};

	const Timestamp* m_globalTimestamp = nullptr;
	HeapAllocator<U8> m_alloc;
	File m_perframeFile;
	File m_perrunFile;
	DArray<Value> m_perframeValues;
	DArray<Value> m_perrunValues;
	DArray<HighRezTimer::Scalar> m_counterTimes;
};

/// The singleton of the counters manager
typedef Singleton<CountersManager> CountersManagerSingleton;

// Macros that encapsulate the functionaly

#if ANKI_ENABLE_COUNTERS
#	define ANKI_COUNTER_INC(counter_, val_) \
		CountersManagerSingleton::get().increaseCounter(Counter::counter_, val_)

#	define ANKI_COUNTER_START_TIMER(counter_) \
		CountersManagerSingleton::get().startTimer(Counter::counter_)

#	define ANKI_COUNTER_STOP_TIMER_INC(counter_) \
		CountersManagerSingleton::get(). \
		stopTimerIncreaseCounter(Counter::counter_)

#	define ANKI_COUNTERS_RESOLVE_FRAME() \
		CountersManagerSingleton::get().resolveFrame()

#	define ANKI_COUNTERS_FLUSH() \
		CountersManagerSingleton::get().flush()
#else // !ANKI_ENABLE_COUNTERS
#	define ANKI_COUNTER_INC(counter_, val_) ((void)0)
#	define ANKI_COUNTER_START_TIMER(counter_) ((void)0)
#	define ANKI_COUNTER_STOP_TIMER_INC(counter_) ((void)0)
#	define ANKI_COUNTERS_RESOLVE_FRAME() ((void)0)
#	define ANKI_COUNTERS_FLUSH() ((void)0)
#endif // ANKI_ENABLE_COUNTERS

// Forward
class ThreadTraceManager;

/// Trace manager per process.
class TraceManager
{
public:
	Array<ThreadTraceManager*, 32> m_threadData;
	Atomic<U32> m_threadCount = {0};
	File m_traceFile;
	Mutex m_fileMtx;

	TraceManager()
	{}

	~TraceManager();

	ANKI_USE_RESULT Error create(
		HeapAllocator<U8>& alloc, const CString& storeDir);

	void flushAll();

	void flush(ThreadTraceManager& thread);
};

using TraceManagerSingleton = Singleton<TraceManager>;

/// Trace manager per thread.
class ThreadTraceManager
{
public:
	static const U MAX_DEPTH = 4;
	static const U BUFFERED_EVENTS_COUNT = 16;

	class Event
	{
	public:
		F64 m_startTime = 0.0;
		F64 m_stopTime = 0.0;
		const char* m_name;
		U32 m_depth = 0;
	};

	TraceManager* m_master = nullptr; ///< Cache it
	U32 m_id = 0;

	Array<Event, MAX_DEPTH> m_inflightEvents; ///< Stack
	U32 m_depth = 0;

	Array<Event, BUFFERED_EVENTS_COUNT> m_bufferedEvents;
	U32 m_bufferedEventsCount = 0;

	ThreadTraceManager();

	/// Begin a new event
	void pushEvent(const char* name);

	/// Stop an already started event
	void popEvent();
};

using ThreadTraceManagerSingleton = SingletonThreadSafe<ThreadTraceManager>;

/// @name Trace macros.
/// @{

#if ANKI_ENABLE_COUNTERS

#	define ANKI_TRACE_PUSH_EVENT(name_) \
	ThreadTraceManagerSingleton::get().pushEvent(name_)

#	define ANKI_TRACE_POP_EVENT() \
	ThreadTraceManagerSingleton::get().popEvent()

#	define ANKI_TRACE_FLUSH() \
	TraceManagerSingleton::get().flushAll()

#else

#	define ANKI_TRACE_PUSH_EVENT(name_) ((void)0)
#	define ANKI_TRACE_POP_EVENT() ((void)0)
#	define ANKI_TRACE_FLUSH() ((void)0)

#endif
/// @}

} // end namespace anki

// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/StdTypes.h"
#include "anki/util/Singleton.h"
#include "anki/util/File.h"
#include "anki/util/HighRezTimer.h"
#include "anki/util/Atomic.h"
#include "anki/util/Thread.h"

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
	SCENE_UPDATE_TIME,
	SWAP_BUFFERS_TIME,
	GL_CLIENT_WAIT_TIME,
	GL_SERVER_WAIT_TIME,
	GL_DRAWCALLS_COUNT,
	GL_VERTICES_COUNT,
	GL_QUEUES_SIZE,
	GL_CLIENT_BUFFERS_SIZE,

	COUNT
};

/// The counters manager. It's been used with a singleton
class CountersManager
{
public:
	CountersManager(HeapAllocator<U8>& alloc, const CString& cacheDir);

	~CountersManager();

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
	File m_perframeFile;
	File m_perrunFile;
	Vector<U64> m_perframeValues;
	Vector<U64> m_perrunValues;
	Vector<HighRezTimer::Scalar> m_counterTimes;
};

/// The singleton of the counters manager
typedef SingletonInit<CountersManager> CountersManagerSingleton;

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

namespace detail {

// Forward
class ThreadTraceManager;

/// Trace manager per process.
class TraceManager
{
public:
	Array<ThreadTraceManager*, 32> m_threadData;
	AtomicU32 m_threadCount = {0};
	File m_traceFile;
	Mutex m_fileMtx;

	TraceManager(HeapAllocator<U8>& alloc, const CString& storeDir);

	~TraceManager();

	void flushAll();

	void flush(ThreadTraceManager& thread);
};

using TraceManagerSingleton = SingletonInit<TraceManager>;

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

} // end namespace detail

/// @name Trace macros.
/// @{

#if ANKI_ENABLE_COUNTERS

#	define ANKI_TRACE_PUSH_EVENT(name_) \
	detail::ThreadTraceManagerSingleton::get().pushEvent(name_)

#	define ANKI_TRACE_POP_EVENT() \
	detail::ThreadTraceManagerSingleton::get().popEvent()

#	define ANKI_TRACE_FLUSH() \
	detail::TraceManagerSingleton::get().flushAll()

#else

#	define ANKI_TRACE_PUSH_EVENT(name_) ((void)0)
#	define ANKI_TRACE_POP_EVENT() ((void)0)
#	define ANKI_TRACE_FLUSH() ((void)0)

#endif

/// @}

} // end namespace anki

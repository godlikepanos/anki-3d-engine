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
using CountersManagerSingleton = Singleton<CountersManager>;

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
/// @}

} // end namespace anki

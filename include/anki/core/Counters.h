// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/util/StdTypes.h"
#include "anki/util/Singleton.h"
#include "anki/util/File.h"
#include "anki/util/HighRezTimer.h"

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
	CountersManager();
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

} // end namespace anki

#include "anki/util/StdTypes.h"
#include "anki/util/Singleton.h"
#include "anki/util/Filesystem.h"
#include "anki/util/HighRezTimer.h"

namespace anki {

/// Enumeration of counters
enum Counter
{
	C_FPS,
	C_MAIN_RENDERER_TIME,
	C_RENDERER_MS_TIME,
	C_RENDERER_IS_TIME,
	C_RENDERER_PPS_TIME,
	C_RENDERER_DRAWCALLS_COUNT,
	C_RENDERER_VERTICES_COUNT,
	C_SCENE_UPDATE_TIME,
	C_SWAP_BUFFERS_TIME,

	C_COUNT
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
	File perframeFile;
	File perrunFile;
	Vector<U64> perframeValues;
	Vector<U64> perrunValues;
	Vector<HighRezTimer::Scalar> counterTimes;
};

/// The singleton of the counters manager
typedef Singleton<CountersManager> CountersManagerSingleton;

// Macros that encapsulate the functionaly

#if ANKI_ENABLE_COUNTERS
#	define ANKI_COUNTER_INC(counter_, val_) \
		CountersManagerSingleton::get().increaseCounter(counter_, val_)

#	define ANKI_COUNTER_START_TIMER(counter_) \
		CountersManagerSingleton::get().startTimer(counter_)

#	define ANKI_COUNTER_STOP_TIMER_INC(counter_) \
		CountersManagerSingleton::get().stopTimerIncreaseCounter(counter_)

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

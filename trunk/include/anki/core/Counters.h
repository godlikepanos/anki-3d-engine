#include "anki/util/StdTypes.h"
#include "anki/util/Singleton.h"
#include "anki/util/Filesystem.h"

namespace anki {

/// Enumeration of counters
enum Counter
{
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

private:
	File perframeFile;
	File globalFile;
	Vector<U64> counters;
};

/// The singleton of the counters manager
typedef Singleton<CountersManager> CountersManagerSingleton;

#define ANKI_COUNTER_INC(counter_, val_) \
	CountersManagerSingleton::get().increaseCounter(counter_, val_)

#define ANKI_COUNTERS_RESOLVE_FRAME() \
	CountersManagerSingleton::get().resolveFrame()

#define ANKI_COUNTERS_FLUSH() \
	CountersManagerSingleton::get().flush()

} // end namespace anki

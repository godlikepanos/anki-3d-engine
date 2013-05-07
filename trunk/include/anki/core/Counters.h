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
	C_SCENE_UPDATE_TIME,
	C_SWAP_BUFFERS_TIME,

	C_COUNT
};

/// XXX
class CountersManager
{
public:
	void increaseCounter(Counter counter, F64 val);
	void increaseCounter(Counter counter, U64 val);

	/// Write the counters of the frame
	void resolveFrame();

private:
	File perframeFile;
};

/// The singleton of the counters manager
typedef Singleton<CountersManager> CountersManagerSingleton;

#define ANKI_COUNTER_INC(counter, val)

} // end namespace anki

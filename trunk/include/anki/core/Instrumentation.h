#include "anki/util/StdTypes.h"

namespace anki {

enum InstrumentationCounter
{
	IC_RENDERER_MS_TIME,
	IC_RENDERER_IS_TIME,
	IC_RENDERER_PPS_TIME,
	IC_RENDERER_DRAWCALLS_COUNT,
	IC_SCENE_UPDATE_TIME,
	IC_SWAP_BUFFERS_TIME,

	IC_COUNT
};

#define ANKI_INSTR_UPDATE_COUNTER(counter, val)

} // end namespace anki

#include "anki/core/Counters.h"

namespace anki {

//==============================================================================

enum CounterFlag
{
	CF_PER_FRAME = 1 << 0,
	CF_GLOBAL = 1 << 1,
	CF_F64 = 1 << 2,
	CF_U64 = 1 << 3
};

struct CounterInfo
{
	const char* name;
	U32 flags;
};

static const CounterInfo counters[C_COUNT - 1] = {
	{"C_RENDERER_MS_TIME", CF_PER_FRAME | CF_F64},
	{"C_RENDERER_IS_TIME", CF_PER_FRAME | CF_F64},
	{"C_RENDERER_PPS_TIME", CF_PER_FRAME | CF_F64},
	{"C_RENDERER_DRAWCALLS_COUNT", CF_GLOBAL | CF_U64},
	{"C_SCENE_UPDATE_TIME", CF_PER_FRAME | CF_F64},
	{"C_SWAP_BUFFERS_TIME", CF_PER_FRAME | CF_F64}
};

} // end namespace anki

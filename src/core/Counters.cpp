#include "anki/core/Counters.h"
#include "anki/core/Timestamp.h"
#include "anki/util/Filesystem.h"
#include "anki/util/Array.h"
#include <cstring>

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

static const Array<CounterInfo, C_COUNT> cinfo = {{
	{"C_RENDERER_MS_TIME", CF_PER_FRAME | CF_F64},
	{"C_RENDERER_IS_TIME", CF_PER_FRAME | CF_F64},
	{"C_RENDERER_PPS_TIME", CF_PER_FRAME | CF_F64},
	{"C_RENDERER_DRAWCALLS_COUNT", CF_GLOBAL | CF_U64},
	{"C_RENDERER_VERTICES_COUNT", CF_GLOBAL | CF_U64},
	{"C_SCENE_UPDATE_TIME", CF_PER_FRAME | CF_F64},
	{"C_SWAP_BUFFERS_TIME", CF_PER_FRAME | CF_F64}
}};

//==============================================================================
CountersManager::CountersManager()
{
	counters.resize(C_COUNT, 0);

	// Open and write the headers to the files
	perframeFile.open("./perframe_counters.csv", File::OF_WRITE);

	perframeFile.writeString("FRAME");

	for(const CounterInfo& inf : cinfo)
	{
		if(inf.flags & CF_PER_FRAME)
		{
			perframeFile.writeString(", %s", inf.name);
		}
	}
}

//==============================================================================
CountersManager::~CountersManager()
{}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, F64 val)
{
	counters[counter] += val;
}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, U64 val)
{
	F64 f;
	memcpy(&f, &val, sizeof(F64));
	*(F64*)(&counters[counter]) += f;
}

//==============================================================================
void CountersManager::resolveFrame()
{
	// Write new line and frame no
	perframeFile.writeString("\n%llu", getGlobTimestamp());

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.flags & CF_PER_FRAME)
		{
			if(inf.flags & CF_U64)
			{
				perframeFile.writeString(", %llu", counters[i]);
			}
			else if(inf.flags & CF_F64)
			{
				perframeFile.writeString(", %f", *((F64*)&counters[i]));
			}
			else
			{
				ANKI_ASSERT(0);
			}

			counters[i] = 0;
		}

		i++;
	}
}

//==============================================================================
void CountersManager::flush()
{
	perframeFile.close();
}

} // end namespace anki

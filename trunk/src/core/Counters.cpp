#include "anki/core/Counters.h"
#include "anki/core/Timestamp.h"
#include "anki/core/App.h"
#include "anki/util/Array.h"
#include <cstring>

namespace anki {

#if ANKI_ENABLE_COUNTERS

//==============================================================================

enum CounterFlag
{
	CF_PER_FRAME = 1 << 0,
	CF_PER_RUN = 1 << 1,
	CF_FPS = 1 << 2, ///< Only with CF_PER_RUN
	CF_F64 = 1 << 3,
	CF_U64 = 1 << 4
};

class CounterInfo
{
public:
	const char* m_name;
	U32 m_flags;
};

static const Array<CounterInfo, (U)Counter::COUNT> cinfo = {{
	{"FPS", CF_PER_RUN | CF_FPS | CF_F64},
	{"MAIN_RENDERER_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_MS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_IS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_PPS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_SHADOW_PASSES", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"RENDERER_LIGHTS_COUNT", CF_PER_RUN | CF_U64},
	{"SCENE_UPDATE_TIME", CF_PER_RUN | CF_F64},
	{"SWAP_BUFFERS_TIME", CF_PER_RUN | CF_F64},
	{"GL_CLIENT_WAIT_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"GL_SERVER_WAIT_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"GL_DRAWCALLS_COUNT", CF_PER_RUN | CF_U64},
	{"GL_VERTICES_COUNT", CF_PER_FRAME | CF_PER_RUN | CF_U64},
}};

#define MAX_NAME "24"

//==============================================================================
CountersManager::CountersManager()
{
	HeapAllocator<U8> alloc(HeapMemoryPool(0));
	
	m_perframeValues.resize((U)Counter::COUNT, 0);
	m_perrunValues.resize((U)Counter::COUNT, 0);
	m_counterTimes.resize((U)Counter::COUNT, 0.0);

	// Open and write the headers to the files
	m_perframeFile.open(
		(AppSingleton::get().getSettingsPath() + "/frame_counters.csv").c_str(), 
		File::OpenFlag::WRITE);

	m_perframeFile.writeText("FRAME");

	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_FRAME)
		{
			m_perframeFile.writeText(", %s", inf.m_name);
		}
	}

	// Open and write the headers to the other file
	m_perrunFile.open(
		(AppSingleton::get().getSettingsPath() + "/run_counters.csv").c_str(), 
		File::OpenFlag::WRITE);

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_RUN)
		{
			if(i != 0)
			{
				m_perrunFile.writeText(", %" MAX_NAME "s", inf.m_name);
			}
			else
			{
				m_perrunFile.writeText("%" MAX_NAME "s", inf.m_name);
			}

			++i;
		}
	}
	m_perrunFile.writeText("\n");
}

//==============================================================================
CountersManager::~CountersManager()
{}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, U64 val)
{
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_U64);

	if(cinfo[(U)counter].m_flags & CF_PER_FRAME)
	{
		m_perframeValues[(U)counter] += val;
	}

	if(cinfo[(U)counter].m_flags & CF_PER_RUN)
	{
		m_perrunValues[(U)counter] += val;
	}
}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, F64 val)
{
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_F64);
	F64 f;
	memcpy(&f, &val, sizeof(F64));

	if(cinfo[(U)counter].m_flags & CF_PER_FRAME)
	{
		*(F64*)(&m_perframeValues[(U)counter]) += f;
	}

	if(cinfo[(U)counter].m_flags & CF_PER_RUN)
	{
		*(F64*)(&m_perrunValues[(U)counter]) += f;
	}
}

//==============================================================================
void CountersManager::startTimer(Counter counter)
{
	// The counter should be F64
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_F64);
	// The timer should have beeb reseted
	ANKI_ASSERT(m_counterTimes[(U)counter] == 0.0);

	m_counterTimes[(U)counter] = HighRezTimer::getCurrentTime();
}

//==============================================================================
void CountersManager::stopTimerIncreaseCounter(Counter counter)
{
	// The counter should be F64
	ANKI_ASSERT(cinfo[(U)counter].m_flags & CF_F64);
	// The timer should have started
	ANKI_ASSERT(m_counterTimes[(U)counter] > 0.0);

	F32 prevTime = m_counterTimes[(U)counter];
	m_counterTimes[(U)counter] = 0.0;

	increaseCounter(counter, HighRezTimer::getCurrentTime() - prevTime);
}

//==============================================================================
void CountersManager::resolveFrame()
{
	// Write new line and frame no
	m_perframeFile.writeText("\n%llu", getGlobTimestamp());

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_FRAME)
		{
			if(inf.m_flags & CF_U64)
			{
				m_perframeFile.writeText(", %llu", m_perframeValues[i]);
			}
			else if(inf.m_flags & CF_F64)
			{
				m_perframeFile.writeText(", %f", *((F64*)&m_perframeValues[i]));
			}
			else
			{
				ANKI_ASSERT(0);
			}

			m_perframeValues[i] = 0;
		}

		i++;
	}
}

//==============================================================================
void CountersManager::flush()
{
	// Resolve per run counters
	U i = 0;
	U j = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_RUN)
		{
			if(j != 0)
			{
				m_perrunFile.writeText(", ");
			}

			if(inf.m_flags & CF_U64)
			{
				m_perrunFile.writeText("%" MAX_NAME "llu", m_perrunValues[i]);
			}
			else if(inf.m_flags & CF_F64)
			{
				if(inf.m_flags & CF_FPS)
				{
					m_perrunFile.writeText("%" MAX_NAME "f", 
						(F64)getGlobTimestamp() / *((F64*)&m_perrunValues[i]));
				}
				else
				{
					m_perrunFile.writeText("%" MAX_NAME "f", 
						*((F64*)&m_perrunValues[i]));
				}
			}
			else
			{
				ANKI_ASSERT(0);
			}

			m_perrunValues[i] = 0;
			++j;
		}

		++i;
	}
	m_perrunFile.writeText("\n");

	// Close and flush files
	m_perframeFile.close();
	m_perrunFile.close();
}

#endif // ANKI_ENABLE_COUNTERS

} // end namespace anki

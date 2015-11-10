// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/Config.h>

#if ANKI_ENABLE_COUNTERS

#include <anki/core/Counters.h>
#include <anki/core/App.h>
#include <anki/util/Array.h>
#include <cstring>

namespace anki {

//==============================================================================

enum CounterFlag
{
	CF_PER_FRAME = 1 << 0,
	CF_PER_RUN = 1 << 1,
	CF_FPS = 1 << 2, ///< Only with CF_PER_RUN
	CF_F64 = 1 << 3,
	CF_U64 = 1 << 4
};

struct CounterInfo
{
	const char* m_name;
	U32 m_flags;
};

static const Array<CounterInfo, U(Counter::COUNT)> cinfo = {{
	{"FPS", CF_PER_RUN | CF_FPS | CF_F64},
	{"MAIN_RENDERER_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_MS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_IS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_PPS_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"RENDERER_SHADOW_PASSES", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"RENDERER_LIGHTS_COUNT", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"RENDERER_MERGED_DRAWCALLS", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"SCENE_UPDATE_TIME", CF_PER_RUN | CF_F64},
	{"SWAP_BUFFERS_TIME", CF_PER_RUN | CF_F64},
	{"GL_CLIENT_WAIT_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"GL_SERVER_WAIT_TIME", CF_PER_FRAME | CF_PER_RUN | CF_F64},
	{"GL_DRAWCALLS_COUNT", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GL_VERTICES_COUNT", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GL_QUEUES_SIZE", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GL_CLIENT_BUFFERS_SIZE", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GR_DYNAMIC_UNIFORMS_SIZE", CF_PER_FRAME | CF_PER_RUN | CF_U64},
	{"GR_DYNAMIC_STORAGE_SIZE", CF_PER_FRAME | CF_PER_RUN | CF_U64}
}};

#define MAX_NAME "25"

//==============================================================================
Error CountersManager::create(
	HeapAllocator<U8> alloc, const CString& cacheDir,
	const Timestamp* globalTimestamp)
{
	m_globalTimestamp = globalTimestamp;

	U count = static_cast<U>(Counter::COUNT);
	m_alloc = alloc;

	m_perframeValues.create(m_alloc, count);

	m_perrunValues.create(m_alloc, count);

	m_counterTimes.create(m_alloc, count);

	memset(&m_perframeValues[0], 0, m_perframeValues.getByteSize());
	memset(&m_perrunValues[0], 0, m_perrunValues.getByteSize());
	memset(&m_counterTimes[0], 0, m_counterTimes.getByteSize());

	// Open and write the headers to the files
	StringAuto tmp(alloc);
	tmp.sprintf("%s/frame_counters.csv", &cacheDir[0]);

	ANKI_CHECK(m_perframeFile.open(tmp.toCString(), File::OpenFlag::WRITE));

	ANKI_CHECK(m_perframeFile.writeText("FRAME"));

	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_FRAME)
		{
			ANKI_CHECK(m_perframeFile.writeText(", %s", inf.m_name));
		}
	}

	// Open and write the headers to the other file
	tmp.destroy(alloc);
	tmp.sprintf("%s/run_counters.csv", &cacheDir[0]);

	ANKI_CHECK(m_perrunFile.open(tmp.toCString(), File::OpenFlag::WRITE));

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_RUN)
		{
			if(i != 0)
			{
				ANKI_CHECK(
					m_perrunFile.writeText(", %" MAX_NAME "s", inf.m_name));
			}
			else
			{
				ANKI_CHECK(
					m_perrunFile.writeText("%" MAX_NAME "s", inf.m_name));
			}

			++i;
		}
	}
	ANKI_CHECK(m_perrunFile.writeText("\n"));

	return ErrorCode::NONE;
}

//==============================================================================
CountersManager::~CountersManager()
{
	m_perframeValues.destroy(m_alloc);
	m_perrunValues.destroy(m_alloc);
	m_counterTimes.destroy(m_alloc);
}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, U64 val)
{
	ANKI_ASSERT(cinfo[U(counter)].m_flags & CF_U64);

	if(cinfo[U(counter)].m_flags & CF_PER_FRAME)
	{
		m_perframeValues[U(counter)].m_int += val;
	}

	if(cinfo[U(counter)].m_flags & CF_PER_RUN)
	{
		m_perrunValues[U(counter)].m_int += val;
	}
}

//==============================================================================
void CountersManager::increaseCounter(Counter counter, F64 val)
{
	ANKI_ASSERT(cinfo[U(counter)].m_flags & CF_F64);

	if(cinfo[U(counter)].m_flags & CF_PER_FRAME)
	{
		m_perframeValues[U(counter)].m_float += val;
	}

	if(cinfo[U(counter)].m_flags & CF_PER_RUN)
	{
		m_perrunValues[U(counter)].m_float += val;
	}
}

//==============================================================================
void CountersManager::startTimer(Counter counter)
{
	// The counter should be F64
	ANKI_ASSERT(cinfo[U(counter)].m_flags & CF_F64);
	// The timer should have been reseted
	ANKI_ASSERT(m_counterTimes[U(counter)] == 0.0);

	m_counterTimes[U(counter)] = HighRezTimer::getCurrentTime();
}

//==============================================================================
void CountersManager::stopTimerIncreaseCounter(Counter counter)
{
	// The counter should be F64
	ANKI_ASSERT(cinfo[U(counter)].m_flags & CF_F64);
	// The timer should have started
	ANKI_ASSERT(m_counterTimes[U(counter)] > 0.0);

	auto prevTime = m_counterTimes[U(counter)];
	m_counterTimes[U(counter)] = 0.0;

	increaseCounter(
		counter, (HighRezTimer::getCurrentTime() - prevTime) * 1000.0);
}

//==============================================================================
void CountersManager::resolveFrame()
{
	Error err = ErrorCode::NONE;

	// Write new line and frame no
	err = m_perframeFile.writeText("\n%llu", *m_globalTimestamp);

	U i = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_FRAME)
		{
			if(inf.m_flags & CF_U64)
			{
				err = m_perframeFile.writeText(
					", %llu", m_perframeValues[i].m_int);
			}
			else if(inf.m_flags & CF_F64)
			{
				err = m_perframeFile.writeText(
					", %f", m_perframeValues[i].m_float);
			}
			else
			{
				ANKI_ASSERT(0);
			}

			m_perframeValues[i].m_int = 0;
		}

		i++;
	}

	(void)err;
}

//==============================================================================
void CountersManager::flush()
{
	Error err = ErrorCode::NONE;

	// Resolve per run counters
	U i = 0;
	U j = 0;
	for(const CounterInfo& inf : cinfo)
	{
		if(inf.m_flags & CF_PER_RUN)
		{
			if(j != 0)
			{
				err = m_perrunFile.writeText(", ");
			}

			if(inf.m_flags & CF_U64)
			{
				err = m_perrunFile.writeText(
					"%" MAX_NAME "llu", m_perrunValues[i].m_int);
			}
			else if(inf.m_flags & CF_F64)
			{
				if(inf.m_flags & CF_FPS)
				{
					F32 fps =
						(F64)(*m_globalTimestamp)
						/ (m_perrunValues[i].m_float / 1000.0);

					err = m_perrunFile.writeText("%" MAX_NAME "f", fps);
				}
				else
				{
					err = m_perrunFile.writeText("%" MAX_NAME "f",
						m_perrunValues[i].m_float);
				}
			}
			else
			{
				ANKI_ASSERT(0);
			}

			m_perrunValues[i].m_int = 0;
			++j;
		}

		++i;
	}
	err = m_perrunFile.writeText("\n");

	// Close and flush files
	m_perframeFile.close();
	m_perrunFile.close();

	if(err)
	{
		ANKI_LOGE("Error in counters file");
	}
}

} // end namespace anki

#undef MAX_NAME

#endif // ANKI_ENABLE_COUNTERS


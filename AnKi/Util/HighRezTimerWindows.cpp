// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/Win32Minimal.h>
#include <cstdio>

namespace anki {

namespace {

/// A dummy struct that inits the timer
class DummyInitTimer
{
public:
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_ticksPerSec;

	DummyInitTimer()
	{
		if(!QueryPerformanceFrequency(&m_ticksPerSec))
		{
			fprintf(stderr, "QueryPerformanceFrequency() failed\n");
			abort();
		}

		QueryPerformanceCounter(&m_start);
	}
};

static DummyInitTimer g_init;

} // namespace

void HighRezTimer::sleep(Second sec)
{
	Sleep(U32(sec * 1000.0));
}

U64 HighRezTimer::getCurrentTimeMs()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	now.QuadPart -= g_init.m_start.QuadPart;
	now.QuadPart *= 1000;
	now.QuadPart /= g_init.m_ticksPerSec.QuadPart;

	return now.QuadPart;
}

U64 HighRezTimer::getCurrentTimeUs()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	now.QuadPart -= g_init.m_start.QuadPart;
	now.QuadPart *= 1000000;
	now.QuadPart /= g_init.m_ticksPerSec.QuadPart;

	return now.QuadPart;
}

Second HighRezTimer::getCurrentTime()
{
	return Second(getCurrentTimeUs()) / 1000000.0;
}

} // end namespace anki

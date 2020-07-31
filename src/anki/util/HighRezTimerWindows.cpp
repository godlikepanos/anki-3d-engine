// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/HighRezTimer.h>
#include <anki/util/Assert.h>
#include <anki/util/Win32Minimal.h>
#include <cstdio>

namespace anki
{

namespace
{

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

DummyInitTimer init;

} // namespace

static U64 getMs()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	now.QuadPart -= init.m_start.QuadPart;
	now.QuadPart *= 1000;
	now.QuadPart /= init.m_ticksPerSec.QuadPart;

	return now.QuadPart;
}

void HighRezTimer::sleep(Second sec)
{
	Sleep(U32(sec * 1000.0));
}

Second HighRezTimer::getCurrentTime()
{
	// Second(ticks) / 1000.0
	return Second(getMs()) * 0.001;
}

} // end namespace anki

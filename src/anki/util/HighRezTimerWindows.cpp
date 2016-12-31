// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/HighRezTimer.h>
#include <anki/util/Assert.h>
#include <Windows.h>

namespace anki
{

namespace
{

/// A dummy struct that inits the timer
class DummyInitTimer
{
public:
	DWORD m_start;

	DummyInitTimer()
	{
		m_start = GetTickCount();
	}
};

DummyInitTimer init;

} // end namespace anonymous

static U32 getMs()
{
	DWORD now = GetTickCount();
	return now - init.m_start;
}

void HighRezTimer::sleep(Scalar sec)
{
	U32 ms = static_cast<U32>(sec * 1000.0);
	Sleep(ms);
}

HighRezTimer::Scalar HighRezTimer::getCurrentTime()
{
	// Scalar(ticks) / 1000.0
	return static_cast<Scalar>(getMs()) * 0.001;
}

} // end namespace anki

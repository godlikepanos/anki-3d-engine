// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/Tracer.h>
#include <anki/core/CoreTracer.h>
#include <anki/util/HighRezTimer.h>

#if ANKI_ENABLE_TRACE
ANKI_TEST(Util, Tracer)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	CoreTracer tracer;
	ANKI_TEST_EXPECT_NO_ERR(tracer.init(alloc, "./"));
	TracerSingleton::get().setEnabled(true);

	// 1st frame
	tracer.flushFrame(0);

	// 2nd frame
	// 2 events
	{
		ANKI_TRACE_SCOPED_EVENT(EVENT);
		HighRezTimer::sleep(0.5);
	}

	{
		ANKI_TRACE_SCOPED_EVENT(EVENT);
		HighRezTimer::sleep(0.25);
	}

	tracer.flushFrame(1);

	// 4rd frame
	// 2 different events & non zero counter
	{
		ANKI_TRACE_SCOPED_EVENT(EVENT);
		HighRezTimer::sleep(0.5);
	}

	{
		ANKI_TRACE_SCOPED_EVENT(EVENT2);
		HighRezTimer::sleep(0.25);
	}

	ANKI_TRACE_INC_COUNTER(COUNTER, 100);

	tracer.flushFrame(3);

	// 5th frame
	ANKI_TRACE_INC_COUNTER(COUNTER, 150);
	tracer.flushFrame(4);
}
#endif

// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/Tracer.h>
#include <anki/util/HighRezTimer.h>

ANKI_TEST(Util, Tracer)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	Tracer tracer;
	tracer.init(alloc);

	// 1st frame
	tracer.beginFrame(0);
	tracer.endFrame();
	ANKI_TEST_EXPECT_NO_ERR(tracer.flush("./0"));

	// 2nd frame
	// 2 same events
	tracer.beginFrame(1);

	tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event");

	tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event");

	tracer.endFrame();

	// 4rd frame
	// 2 different events & non zero counter
	tracer.beginFrame(3);

	tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event");

	tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event2");

	tracer.increaseCounter("counter", 100);

	tracer.endFrame();

	// 5th frame
	tracer.beginFrame(4);
	tracer.increaseCounter("counter", 150);
	HighRezTimer::sleep(0.1);
	tracer.endFrame();

	ANKI_TEST_EXPECT_NO_ERR(tracer.flush("./1"));
}

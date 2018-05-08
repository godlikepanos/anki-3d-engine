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
	tracer.newFrame(0);
	ANKI_TEST_EXPECT_NO_ERR(tracer.flush("./0"));

	// 2nd frame
	// 2 same events
	tracer.newFrame(1);

	auto handle0 = tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event", handle0);

	auto handle1 = tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event", handle1);

	// 4rd frame
	// 2 different events & non zero counter
	tracer.newFrame(3);

	auto handle2 = tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event", handle2);

	auto handle3 = tracer.beginEvent();
	HighRezTimer::sleep(0.5);
	tracer.endEvent("event2", handle3);

	tracer.increaseCounter("counter", 100);

	// 5th frame
	tracer.newFrame(4);
	tracer.increaseCounter("counter", 150);
	HighRezTimer::sleep(0.1);

	ANKI_TEST_EXPECT_NO_ERR(tracer.flush("./1"));
}

// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <tests/framework/Framework.h>
#include <anki/util/HighRezTimer.h>
#include <chrono>
#include <thread>

ANKI_TEST(Util, Test)
{
	HighRezTimer t;
	t.start();

	std::this_thread::sleep_for(std::chrono::seconds(2));

	ANKI_TEST_EXPECT_NEAR(t.getElapsedTime(), 2.0, 0.2);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	ANKI_TEST_EXPECT_NEAR(t.getElapsedTime(), 3.0, 0.2);

	std::this_thread::sleep_for(std::chrono::seconds(1));
	t.stop();
	std::this_thread::sleep_for(std::chrono::seconds(1));

	ANKI_TEST_EXPECT_NEAR(t.getElapsedTime(), 4.0, 0.2);
}

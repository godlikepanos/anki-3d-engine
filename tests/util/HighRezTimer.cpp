// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/util/HighRezTimer.h"
#include <unistd.h>

ANKI_TEST(Util, Test)
{
	HighRezTimer t;
	t.start();

	sleep(2);

	ANKI_TEST_EXPECT_NEAR(t.getElapsedTime(), 2.0, 0.2);

	sleep(1);

	ANKI_TEST_EXPECT_NEAR(t.getElapsedTime(), 3.0, 0.2);

	sleep(1);
	t.stop();
	sleep(1);

	ANKI_TEST_EXPECT_NEAR(t.getElapsedTime(), 4.0, 0.2);
}

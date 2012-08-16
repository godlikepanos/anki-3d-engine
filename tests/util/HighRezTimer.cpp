#include "tests/framework/Framework.h"
#include "anki/util/HighRezTimer.h"
#include <unistd.h>

ANKI_TEST(HighRezTimer, Test)
{
	HighRezTimer t;
	t.start();
	
	sleep(2);
	
	ANKI_TEST_EXPECT_EQ(t.getElapsedTime(), 2.0);
	
	sleep(1);
	
	ANKI_TEST_EXPECT_EQ(t.getElapsedTime(), 3.0);
	
	sleep(1);
	t.stop();
	sleep(1);
	
	ANKI_TEST_EXPECT_EQ(t.getElapsedTime(), 4.0);
}

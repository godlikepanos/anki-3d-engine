#include <gtest/gtest.h>
#include "HighRezTimer.h"


TEST(HighRezTimer, Test)
{
	HighRezTimer t;
	t.start();
	
	sleep(2);
	
	EXPECT_EQ(t.getElapsedTime(), 2.0);
	
	sleep(1);
	
	EXPECT_EQ(t.getElapsedTime(), 3.0);
	
	sleep(1);
	t.stop();
	sleep(1);
	
	EXPECT_EQ(t.getElapsedTime(), 4.0);
}

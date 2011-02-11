#include <gtest/gtest.h>
#include "HighRezTimer.h"


TEST(HighRezTimer, Test)
{
	HighRezTimer t;
	t.start();
	
	sleep(2);
	
	EXPECT_EQ(t.getElapsedTime(), 2000);	
	
	sleep(1);
	
	EXPECT_EQ(t.getElapsedTime(), 3000);	
	
	sleep(1);
	t.stop();
	sleep(1);
	
	EXPECT_EQ(t.getElapsedTime(), 4000);	
}

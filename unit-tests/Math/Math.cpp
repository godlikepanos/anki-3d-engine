#include <gtest/gtest.h>
#include <iostream>
#include "Math.h"


TEST(MathTests, Init)
{
	Vec4 a, b;
	EXPECT_EQ(a, Vec4(0.0));
	EXPECT_EQ(b, Vec4(0.0));
}


TEST(MathTests, Addition)
{
	float arr0[] = {1.0001, -2.0, 0.00000012300123, 400.0};
	float arr1[] = {1.0, 2.0, -3.0, 4.0};

	Vec4 a(arr0), b(arr1);
	EXPECT_EQ(a + b, Vec4(arr0[0] + arr1[0], arr0[1] + arr1[1], arr0[2] + arr1[2], arr0[3] + arr1[3]));

	//a += b;
	//EXPECT_EQ(a, Vec4(arr0[0] + arr0[0], arr0[1] + arr0[1], arr0[2] + arr0[2], arr0[3] + arr0[3]));
}

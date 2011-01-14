#include <gtest/gtest.h>
#include <iostream>
#include "Math.h"
#include "Util.h"


const float RANGE_MIN = -1.00123;
const float RANGE_MAX = 9.9990001;


static float randFloat()
{
	return Util::randRange(RANGE_MIN, RANGE_MAX);
}


TEST(MathTests, Alignment)
{
	const int FS = sizeof(float); // float size

	EXPECT_EQ(sizeof(Vec2), FS * 2);
	EXPECT_EQ(sizeof(Vec3), FS * 3);
	EXPECT_EQ(sizeof(Vec4), FS * 4);
	EXPECT_EQ(sizeof(Quat), FS * 4);
	EXPECT_EQ(sizeof(Euler), FS * 3);
	EXPECT_EQ(sizeof(Mat3), FS * 9);
	EXPECT_EQ(sizeof(Mat4), FS * 16);
}


TEST(MathTests, Constructors)
{
	{
		EXPECT_EQ(Vec4().x(), 0.0);
		EXPECT_EQ(Vec4().y(), 0.0);
		EXPECT_EQ(Vec4().z(), 0.0);
		EXPECT_EQ(Vec4().w(), 0.0);

		float f = randFloat();
		Vec4 a(f);
		EXPECT_EQ(a.x(), f);
		EXPECT_EQ(a.y(), f);
		EXPECT_EQ(a.z(), f);
		EXPECT_EQ(a.w(), f);

		float arr[4] = {randFloat(), randFloat(), randFloat(), randFloat()};
		a = Vec4(arr);
		EXPECT_EQ(a[0], arr[0]);
		EXPECT_EQ(a.y(), arr[1]);
		EXPECT_EQ(a.z(), arr[2]);
		EXPECT_EQ(a[3], arr[3]);
	}


}


template<typename Type, int size, Type (Type::* op)(const Type&) const>
void testOperator()
{
	Type a, b, c;

	for(int i = 0; i < size; i++)
	{
		a[i] = randFloat();
		b[i] = randFloat();
		//c[i] = a[i] + b[i];
	}

	EXPECT_EQ((a.*op)(b), c);

	/*a += b;

	EXPECT_EQ(a, c);*/
}


TEST(MathTests, Addition)
{
	testOperator<Vec4, 4, &Vec4::operator+ >();
}


TEST(MathTests, Substraction)
{
	Vec4 a(randFloat(), randFloat(), randFloat(), randFloat());
	Vec4 b(randFloat(), randFloat(), randFloat(), randFloat());
	EXPECT_EQ(a - b, Vec4(a.x() - b.x(), a.y() - b.y(), a.z() - b.z(), a.w() - b.w()));

	Vec4 c = a;
	a -= b;
	EXPECT_EQ(a, Vec4(c.x() - b.x(), c.y() - b.y(), c.z() - b.z(), c.w() - b.w()));
}

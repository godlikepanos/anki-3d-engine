#include "tests/framework/Framework.h"
#include "anki/Math.h"

using namespace anki;

/// Test the common perators between a vector and another vector
template<typename Vec, typename T>
void operatorsSame()
{
	const U size = sizeof(Vec) / sizeof(T);

	Vec a, b;
	Array<T, size> add, sub, mul, div;
	for(U i = 0; i < size; i++)
	{
		T v0 = T(i * 10) / 2;
		T v1 = T(i * 1000) / 5.123;

		a[i] = v0;
		b[i] = v1;
		add[i] = v0 + v1;
		sub[i] = v0 - v1;
		mul[i] = v0 * v1;
		div[i] = v0 / v1;
	}

	Vec c = a + b;
	Vec d = a - b;
	Vec e = a * b;
	Vec f = a / b;
	for(U i = 0; i < size; i++)
	{
		ANKI_TEST_EXPECT_EQ(c[i], add[i]);
		ANKI_TEST_EXPECT_EQ(d[i], sub[i]);
		ANKI_TEST_EXPECT_EQ(e[i], mul[i]);
		ANKI_TEST_EXPECT_EQ(f[i], div[i]);
	}
}

/// Test the dot prods
template<typename Vec, typename T>
void dot()
{
	const U size = sizeof(Vec) / sizeof(T);
	T res = 0;
	Vec vec;

	for(U i = 0; i < size; i++)
	{
		T x = i * 666;

		vec[i] = x;
		res += x * x;
	}

	ANKI_TEST_EXPECT_EQ(vec.dot(vec), res);
}

/// Common vector tests
template<typename Vec, typename T>
void commonVecTests()
{
	operatorsSame<Vec, T>();
	dot<Vec, T>();
}

ANKI_TEST(Math, Vec2)
{
	commonVecTests<Vec2, F32>();
	commonVecTests<IVec2, I32>();
	commonVecTests<UVec2, U32>();
}

ANKI_TEST(Math, Vec3)
{
	commonVecTests<Vec3, F32>();
	commonVecTests<IVec3, I32>();
	commonVecTests<UVec3, U32>();
}

ANKI_TEST(Math, Vec4)
{
	commonVecTests<Vec4, F32>();
	commonVecTests<IVec4, I32>();
	commonVecTests<UVec4, U32>();
}

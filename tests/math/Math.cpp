// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "tests/framework/Framework.h"
#include "anki/Math.h"

using namespace anki;

/// Test the common perators between a vector and another vector
template<typename Vec>
void operatorsSame()
{
	const U size = Vec::COMPONENT_COUNT;
	using T = typename Vec::Scalar;

	Vec a, b;
	Array<T, size> add, sub, mul, div;
	for(U i = 0; i < size; i++)
	{
		T v0 = T(F64(i * 10) / 2.0 + 1.0);
		T v1 = T(F64(i * 1000) / 5.123 + 1.0);

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

	c = a;
	c += b;
	d = a;
	d -= b;
	e = a;
	e *= b;
	f = a;
	f /= b;
	for(U i = 0; i < size; i++)
	{
		ANKI_TEST_EXPECT_EQ(c[i], add[i]);
		ANKI_TEST_EXPECT_EQ(d[i], sub[i]);
		ANKI_TEST_EXPECT_EQ(e[i], mul[i]);
		ANKI_TEST_EXPECT_EQ(f[i], div[i]);
	}
}

/// Test the dot prods
template<typename Vec>
void dot()
{
	const U size = Vec::COMPONENT_COUNT;
	using T = typename Vec::Scalar;
	T res = 0;
	Vec vec;

	for(U i = 0; i < size; i++)
	{
		T x = T(i * 666 + 1);

		vec[i] = x;
		res += x * x;
	}

	ANKI_TEST_EXPECT_EQ(vec.dot(vec), res);
}

/// Test length and normalization
template<typename Vec>
void length()
{
	using T = typename Vec::Scalar;
	U size = Vec::COMPONENT_COUNT;
	Vec vec;
	T res = 0;

	for(U i = 0; i < size; i++)
	{
		T x = T(i * 666);

		vec[i] = x;
		res += x * x;
	}

	res = T(sqrt(F32(res)));
	ANKI_TEST_EXPECT_EQ(vec.getLength(), res);

	if(Vec::IS_INTEGER)
	{
		ANKI_TEST_EXPECT_EQ(vec.getNormalized(), vec / res);
	}
	else
	{
		auto a = vec / res;
		auto b = vec.getNormalized();
		for(U i = 0; i < size; i++)
		{
			ANKI_TEST_EXPECT_NEAR(a[i], b[i], 0.0001);
		}
	}
}

template<typename Vec>
void comparision()
{
	U size = Vec::COMPONENT_COUNT;
	using Scalar = typename Vec::Scalar;
	Vec a, a1, b;

	for(U i = 0; i < size; i++)
	{
		a[i] = Scalar(i * 666);
		a1[i] = a[i];
		b[i] = a[i] + 1;
	}

	ANKI_TEST_EXPECT_EQ(a == a1, true);
	ANKI_TEST_EXPECT_EQ(a != a1, false);
	ANKI_TEST_EXPECT_EQ(a < a1, false);
	ANKI_TEST_EXPECT_EQ(a <= a1, true);
	ANKI_TEST_EXPECT_EQ(a > a1, false);
	ANKI_TEST_EXPECT_EQ(a >= a1, true);

	ANKI_TEST_EXPECT_EQ(a == b, false);
	ANKI_TEST_EXPECT_EQ(a != b, true);
	ANKI_TEST_EXPECT_EQ(a < b, true);
	ANKI_TEST_EXPECT_EQ(a <= b, true);
	ANKI_TEST_EXPECT_EQ(a > b, false);
	ANKI_TEST_EXPECT_EQ(a >= b, false);
}

/// Common vector tests
template<typename Vec>
void commonVecTests()
{
	operatorsSame<Vec>();
	dot<Vec>();
	length<Vec>();
	comparision<Vec>();
}

ANKI_TEST(Math, Vec2)
{
	commonVecTests<Vec2>();
	commonVecTests<IVec2>();
	commonVecTests<UVec2>();
}

ANKI_TEST(Math, Vec3)
{
	commonVecTests<Vec3>();
	commonVecTests<IVec3>();
	commonVecTests<UVec3>();
}

ANKI_TEST(Math, Vec4)
{
	commonVecTests<Vec4>();
	commonVecTests<IVec4>();
	commonVecTests<UVec4>();
}

/// Test the common operators between a matrices
template<typename Mat>
void matOperatorsSame()
{
	const U size = Mat::SIZE;
	using T = typename Mat::Scalar;

	Mat a, b;
	Array<T, size> add, sub;
	for(U i = 0; i < size; i++)
	{
		T v0 = T(i * 10 / 2);
		T v1 = T(F64(i * 1000) / 5.123);

		a[i] = v0;
		b[i] = v1;
		add[i] = v0 + v1;
		sub[i] = v0 - v1;
	}

	Mat c = a + b;
	Mat d = a - b;
	for(U i = 0; i < size; i++)
	{
		ANKI_TEST_EXPECT_EQ(c[i], add[i]);
		ANKI_TEST_EXPECT_EQ(d[i], sub[i]);
	}

	c = a;
	c += b;
	d = a;
	d -= b;
	for(U i = 0; i < size; i++)
	{
		ANKI_TEST_EXPECT_EQ(c[i], add[i]);
		ANKI_TEST_EXPECT_EQ(d[i], sub[i]);
	}
}

/// Get a filled matrix
template<typename Mat>
Mat getNonEmptyMat(typename Mat::Scalar offset = 0)
{
	Mat out;

	for(U i = 0; i < Mat::SIZE; i++)
	{
		out[i] = typename Mat::Scalar(i) + offset;
	}

	return out;
}

/// Some getters setters
template<typename Mat, typename RowVec, typename ColVec>
void matSettersGetters()
{
	using T = typename Mat::Scalar;

	Mat m(T(0));

	m.setRow(1, RowVec(1.0));
	ANKI_TEST_EXPECT_EQ(m.getRow(1), RowVec(1.0));

	m.setColumn(1, ColVec(2.0));
	m.setColumn(2, ColVec(3.0));
	ANKI_TEST_EXPECT_EQ(m.getColumn(1), ColVec(2.0));
	ANKI_TEST_EXPECT_EQ(m.getColumn(2), ColVec(3.0));

	ANKI_TEST_EXPECT_EQ(m.getYAxis(), m.getColumn(1));
}

/// Transpose
template<typename Mat>
void transpose()
{
	Mat a = getNonEmptyMat<Mat>();
	Mat b = a.getTransposed();

	for(U j = 0; j < Mat::ROW_SIZE; j++)
	{
		for(U i = 0; i < Mat::COLUMN_SIZE; i++)
		{
			ANKI_TEST_EXPECT_EQ(a(j, i), b(i, j));
		}
	}

	b.transposeRotationPart();
	for(U j = 0; j < 3; j++)
	{
		for(U i = 0; i < 3; i++)
		{
			ANKI_TEST_EXPECT_EQ(a(j, i), b(j, i));
		}
	}
}

/// Mat vector multiplication
template<typename Mat, typename VecIn, typename VecOut>
void matVecMul()
{
	using T = typename Mat::Scalar;

	Mat m = getNonEmptyMat<Mat>();
	VecIn v;
	for(U i = 0; i < VecIn::SIZE; i++)
	{
		v[i] = i;
	}

	VecOut out = m * v;
	VecOut out1;
	for(U j = 0; j < Mat::ROW_SIZE; j++)
	{
		T sum = 0;
		for(U i = 0; i < Mat::COLUMN_SIZE; i++)
		{
			sum += m(j, i) * v[j];
		}

		out1[j] = sum;
	}

	ANKI_TEST_EXPECT_EQ(out1, out);
}

template<typename Mat, typename RowVec, typename ColVec>
void commonMatTests()
{
	matOperatorsSame<Mat>();
	matSettersGetters<Mat, RowVec, ColVec>();
}

ANKI_TEST(Math, Mat3)
{
	commonMatTests<Mat3, Vec3, Vec3>();
	transpose<Mat3>();

	// mat*mat
	{
		Mat3 a = getNonEmptyMat<Mat3>(0);
		Mat3 b = getNonEmptyMat<Mat3>(1);
		Mat3 c = a * b;
		Mat3 d = Mat3(18.000, 21.000, 24.000, 54.000, 66.000, 78.000, 90.000, 111.000, 132.000);
		ANKI_TEST_EXPECT_EQ(c, d);
	}

	// mat*vec
	{
		Mat3 m = getNonEmptyMat<Mat3>(1.0);
		Vec3 v(0.0, 1.0, 2.0);

		ANKI_TEST_EXPECT_EQ(m * v, Vec3(8, 17, 26));
	}
}

ANKI_TEST(Math, Mat4)
{
	commonMatTests<Mat4, Vec4, Vec4>();
	transpose<Mat4>();

	// mat*mat
	{
		Mat4 a = getNonEmptyMat<Mat4>(0);
		Mat4 b = getNonEmptyMat<Mat4>(1);
		Mat4 c = a * b;
		Mat4 d = Mat4(62.0, 68.0, 74.0, 80.0, 174.000, 196.000, 218.000, 240.000, 286.000, 324.000, 362.000, 400.000,
					  398.000, 452.000, 506.000, 560.000);
		ANKI_TEST_EXPECT_EQ(c, d);
	}

	// mat*vec
	{
		Mat4 m = getNonEmptyMat<Mat4>(1.0);
		Vec4 v(0.0, 1.0, 2.0, 3.0);

		ANKI_TEST_EXPECT_EQ(m * v, Vec4(20, 44, 68, 92));
	}
}

ANKI_TEST(Math, Mat3x4)
{
	commonMatTests<Mat3x4, Vec4, Vec3>();

	// combine transforms
	{
		Mat3x4 a = getNonEmptyMat<Mat3x4>(0);
		Mat3x4 b = getNonEmptyMat<Mat3x4>(1);
		Mat3x4 c = a.combineTransformations(b);
		Mat3x4 d = Mat3x4(23.000, 26.000, 29.000, 35.000, 83.000, 98.000, 113.000, 135.000, 143.000, 170.000, 197.000,
						  235.000);
		ANKI_TEST_EXPECT_EQ(c, d);
	}

	// mat*vec
	{
		Mat3x4 m = getNonEmptyMat<Mat3x4>(1.0);
		Vec4 v(0.0, 1.0, 2.0, 3.0);

		ANKI_TEST_EXPECT_EQ(m * v, Vec3(20, 44, 68));
	}
}

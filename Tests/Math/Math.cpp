// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <AnKi/Math.h>

using namespace anki;

/// Test the common perators between a vector and another vector
template<typename Vec>
void operatorsSame()
{
	const U size = Vec::kComponentCount;
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
	const U size = Vec::kComponentCount;
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
	U size = Vec::kComponentCount;
	Vec vec;
	T res = 0;

	for(U i = 0; i < size; i++)
	{
		T x = T(i * 666);

		vec[i] = x;
		res += x * x;
	}

	res = T(sqrt(F32(res)));
	ANKI_TEST_EXPECT_EQ(vec.length(), res);

	if(Vec::kIsInteger)
	{
		ANKI_TEST_EXPECT_EQ(vec.normalize(), vec / res);
	}
	else
	{
		auto a = vec / res;
		auto b = vec.normalize();
		for(U i = 0; i < size; i++)
		{
			ANKI_TEST_EXPECT_NEAR(a[i], b[i], 0.0001);
		}
	}
}

template<typename Vec>
void comparision()
{
	U size = Vec::kComponentCount;
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

ANKI_TEST(Math, VecOperators)
{
	// Float vec vs scalar
	const Vec3 a(1.0f, -2.0f, 3.5f);
	const Vec3 add = a + 2.0f;
	const Vec3 sub = a - 1.0f;
	const Vec3 mul = a * 2.0f;
	const Vec3 div = a / 2.0f;
	ANKI_TEST_EXPECT_EQ(add, Vec3(3.0f, 0.0f, 5.5f));
	ANKI_TEST_EXPECT_EQ(sub, Vec3(0.0f, -3.0f, 2.5f));
	ANKI_TEST_EXPECT_EQ(mul, Vec3(2.0f, -4.0f, 7.0f));
	ANKI_TEST_EXPECT_NEAR(div.x, 0.5f, 1e-6f);
	ANKI_TEST_EXPECT_NEAR(div.y, -1.0f, 1e-6f);
	ANKI_TEST_EXPECT_NEAR(div.z, 1.75f, 1e-6f);

	// Integer-specific operators
	const IVec2 ai(4, 8);
	const IVec2 bi(1, 3);
	ANKI_TEST_EXPECT_EQ(ai << bi, IVec2(8, 64));
	ANKI_TEST_EXPECT_EQ(ai >> bi, IVec2(2, 1));
	ANKI_TEST_EXPECT_EQ(ai & bi, IVec2(0, 0));
	ANKI_TEST_EXPECT_EQ(ai | bi, IVec2(5, 11));
	ANKI_TEST_EXPECT_EQ(ai ^ bi, IVec2(5, 11));
	ANKI_TEST_EXPECT_EQ(ai % bi, IVec2(0, 2));

	// Mixed vector/scalar integer operators
	IVec2 ci(2, 3);
	ci <<= 1;
	ANKI_TEST_EXPECT_EQ(ci, IVec2(4, 6));
	ci &= IVec2(6);
	ANKI_TEST_EXPECT_EQ(ci, IVec2(4, 6));
	ci |= 1;
	ANKI_TEST_EXPECT_EQ(ci, IVec2(5, 7));
}

ANKI_TEST(Math, VecExtras)
{
	// Cross product (Vec3)
	const Vec3 a(1.0f, 2.0f, 3.0f);
	const Vec3 b(4.0f, 5.0f, 6.0f);
	const Vec3 cross = a.cross(b);
	ANKI_TEST_EXPECT_NEAR(cross.x, -3.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(cross.y, 6.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(cross.z, -3.0f, 1e-5f);

	// Projection on vector
	const Vec3 proj = a.projectTo(Vec3::xAxis());
	ANKI_TEST_EXPECT_NEAR(proj.x, 1.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(proj.y, 0.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(proj.z, 0.0f, 1e-5f);

	// Projection on ray
	const Vec3 projRay = a.projectTo(Vec3(0.0f), Vec3::yAxis());
	ANKI_TEST_EXPECT_NEAR(projRay.x, 0.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(projRay.y, 2.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(projRay.z, 0.0f, 1e-5f);

	// Lerp (scalar t)
	const Vec3 lerpRes = a.lerp(b, 0.5f);
	ANKI_TEST_EXPECT_NEAR(lerpRes.x, 2.5f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(lerpRes.y, 3.5f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(lerpRes.z, 4.5f, 1e-5f);

	// Clamp and reciprocal
	const Vec3 clamped = Vec3(-1.0f, 2.0f, 10.0f).clamp(0.0f, 5.0f);
	ANKI_TEST_EXPECT_NEAR(clamped.x, 0.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(clamped.y, 2.0f, 1e-5f);
	ANKI_TEST_EXPECT_NEAR(clamped.z, 5.0f, 1e-5f);

	const Vec2 recip = Vec2(2.0f, -4.0f).reciprocal();
	ANKI_TEST_EXPECT_NEAR(recip.x, 0.5f, 1e-6f);
	ANKI_TEST_EXPECT_NEAR(recip.y, -0.25f, 1e-6f);

	// Perspective divide (Vec4)
	const Vec4 v4(2.0f, 4.0f, 6.0f, 2.0f);
	const Vec4 pdiv = v4.perspectiveDivide();
	ANKI_TEST_EXPECT_NEAR(pdiv.x, 1.0f, 1e-6f);
	ANKI_TEST_EXPECT_NEAR(pdiv.y, 2.0f, 1e-6f);
	ANKI_TEST_EXPECT_NEAR(pdiv.z, 3.0f, 1e-6f);
	ANKI_TEST_EXPECT_NEAR(pdiv.w, 1.0f, 1e-6f);
}

/// Test the common operators between a matrices
template<typename Mat>
void matOperatorsSame()
{
	const U size = Mat::kSize;
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

	for(U i = 0; i < Mat::kSize; i++)
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
	Mat b = a.transpose();

	for(U j = 0; j < Mat::kRowCount; j++)
	{
		for(U i = 0; i < Mat::kColumnCount; i++)
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
	for(U i = 0; i < VecIn::kSize; i++)
	{
		v[i] = i;
	}

	VecOut out = m * v;
	VecOut out1;
	for(U j = 0; j < Mat::kRowCount; j++)
	{
		T sum = 0;
		for(U i = 0; i < Mat::kColumnCount; i++)
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
		Mat4 d =
			Mat4(62.0, 68.0, 74.0, 80.0, 174.000, 196.000, 218.000, 240.000, 286.000, 324.000, 362.000, 400.000, 398.000, 452.000, 506.000, 560.000);
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
		Mat3x4 d = Mat3x4(23.000, 26.000, 29.000, 35.000, 83.000, 98.000, 113.000, 135.000, 143.000, 170.000, 197.000, 235.000);
		ANKI_TEST_EXPECT_EQ(c, d);
	}

	// mat*vec
	{
		Mat3x4 m = getNonEmptyMat<Mat3x4>(1.0);
		Vec4 v(0.0, 1.0, 2.0, 3.0);

		ANKI_TEST_EXPECT_EQ(m * v, Vec3(20, 44, 68));
	}
}

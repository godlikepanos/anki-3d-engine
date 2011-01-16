#include <gtest/gtest.h>
#include <iostream>
#include "Math.h"
#include "Util.h"


const float RANGE_MIN = -1.00123;
const float RANGE_MAX = 900.9990001;
const float REALY_SMALL_FLOAT = 1.0e-3;

/// Get random float and not zero cause of the divs
static float randFloat()
{
	float f;
	while(true)
	{
		f = Util::randRange(RANGE_MIN, RANGE_MAX);
		if(!isZero(f))
		{
			break;
		}
	}

	return f;
}


//======================================================================================================================
// Alignment                                                                                                           =
//======================================================================================================================
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


//======================================================================================================================
// Constructors                                                                                                        =
//======================================================================================================================

template<typename Type>
void testCommonContructors()
{
	Type a, b;
	float f;
	float arr[sizeof(Type) / sizeof(float)];

	f = randFloat();
	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = f;
		b[i] =  randFloat();
		arr[i] = b[i];
	}

	// float
	EXPECT_EQ(Type(f), a);

	// arr
	EXPECT_EQ(Type(arr), b);

	// Type
	EXPECT_EQ(Type(b), b);
}


TEST(MathTests, Constructors)
{
	testCommonContructors<Vec2>();

	testCommonContructors<Vec3>();

	// Vec4
	testCommonContructors<Vec4>();
}


//======================================================================================================================
// Arithmetic                                                                                                          =
//======================================================================================================================

float addf(float a, float b)
{
	return a + b;
}


float subf(float a, float b)
{
	return a - b;
}


float mulf(float a, float b)
{
	return a * b;
}


float divf(float a, float b)
{
	return a / b;
}


template<
	typename Type,
	Type (Type::* op)(const Type&) const,
	Type& (Type::* compoundAssignment)(const Type&),
	float (* normalOp)(float, float)>
void testOperators()
{
	Type a, b, c;

	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = randFloat();
		b[i] = randFloat();
		c[i] = normalOp(a[i], b[i]);
	}

	EXPECT_EQ((a.*op)(b), c);

	(a.*compoundAssignment)(b);

	EXPECT_EQ(a, c);
}


template<
	typename Type,
	Type (Type::* op)(float) const,
	Type& (Type::* compoundAssignment)(float),
	Type (* opExtern)(float, const Type&),
	float (* normalOp)(float, float)>
void testOperatorsWithFloat()
{
	Type a, b, c;
	float f = randFloat();

	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = randFloat();
		b[i] = normalOp(a[i], f);
		c[i] = normalOp(f, a[i]);
	}

	EXPECT_EQ((a.*op)(f), b);

	EXPECT_EQ(opExtern(f, a), c);

	(a.*compoundAssignment)(f);
	EXPECT_EQ(a, b);
}


template<typename Type>
void testCmpOperators()
{
	Type a, b;

	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = b[i] = randFloat();
	}

	EXPECT_EQ(a == b, true);
	EXPECT_EQ(a != b, false);
}


template<typename Type>
void testNegOperator()
{
	Type a, b;
	for(int i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = randFloat();
		b[i] = -a[i];
	}

	EXPECT_EQ(-a == b, true);
}


template<typename Type>
void arithmeticOperations()
{
	testOperators<Type, &Type::operator+, &Type::operator+=, &addf>();
	testOperators<Type, &Type::operator-, &Type::operator-=, &subf>();
	testOperators<Type, &Type::operator*, &Type::operator*=, &mulf>();
	testOperators<Type, &Type::operator/, &Type::operator/=, &divf>();
	testOperatorsWithFloat<Type, &Type::operator+, &Type::operator+=, &operator+, &addf>();
	testOperatorsWithFloat<Type, &Type::operator-, &Type::operator-=, &operator-, &subf>();
	testOperatorsWithFloat<Type, &Type::operator*, &Type::operator*=, &operator*, &mulf>();
	testOperatorsWithFloat<Type, &Type::operator/, &Type::operator/=, &operator/, &divf>();
	testCmpOperators<Type>();
}


TEST(MathTests, VectorArithmetic)
{
	arithmeticOperations<Vec2>();
	arithmeticOperations<Vec3>();
	arithmeticOperations<Vec4>();
}


template<typename Type>
void testDotProd()
{
	Type a, b;
	float o = 0.0;

	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = randFloat();
		b[i] = randFloat();
		o += a[i] * b[i];
	}

	EXPECT_NEAR(a.dot(b), o, M::EPSILON);
}


TEST(MathTests, DotProducts)
{
	testDotProd<Vec2>();
	testDotProd<Vec3>();
	testDotProd<Vec4>();
}


template<typename Type>
void testLengthAndNormalize()
{
	Type a;
	float o = 0.0;

	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = randFloat();
		o += a[i] * a[i];
	}

	o = sqrt(o);

	EXPECT_NEAR(a.getLength(), o, REALY_SMALL_FLOAT);
	EXPECT_NEAR(a.getNormalized().getLength(), 1.0, REALY_SMALL_FLOAT);
	//EXPECT_EQ(a.getNormalized() * a.getLength(), a);
}


TEST(MathTests, LengthsAndNormals)
{
	testLengthAndNormalize<Vec2>();
	testLengthAndNormalize<Vec3>();
	testLengthAndNormalize<Vec4>();
}

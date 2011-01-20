#include <gtest/gtest.h>
#include <iostream>
#include "Math.h"
#include "MathCommon.ut.h"


const float REALY_SMALL_FLOAT = 1.0e-3;




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


TEST(MathTests, VectorConstructors)
{
	testCommonContructors<Vec2>();
	testCommonContructors<Vec3>();
	testCommonContructors<Vec4>();
}


//======================================================================================================================
// Operators                                                                                                           =
//======================================================================================================================

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


//======================================================================================================================
// Misc                                                                                                                =
//======================================================================================================================

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

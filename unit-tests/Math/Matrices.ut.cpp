#include <gtest/gtest.h>
#include <iostream>
#include "Math.h"
#include "MathCommon.ut.h"

/// Test nxn matrix multiplication
template<typename Type>
void testMatrixMul()
{
	int dimention = log2(sizeof(Type) / sizeof(float));
	Type r, a, b;

	// Fill
	for(int i = 0; i < dimention; i++)
	{
		for(int j = 0; j < dimention; j++)
		{
			a(i, j) = randFloat();
			b(i, j) = randFloat();
		}
	}

	// Calc r = a * b as usual
	for(int i = 0; i < dimention; i++)
	{
		for(int j = 0; j < dimention; j++)
		{
			r(i, j) = 0.0;
			for(int k = 0; k < dimention; k++)
			{
				r(i, j) += a(i, k) * b(k, j);
			}
		}
	}

	EXPECT_EQ(r, a * b);
}


template<typename Type>
void arithmeticOperations()
{
	testOperators<Type, &Type::operator+, &Type::operator+=, &addf>();
	testMatrixMul<Type>();
	testOperators<Type, &Type::operator-, &Type::operator-=, &subf>();
	testOperatorsWithFloat<Type, &Type::operator+, &Type::operator+=, &operator+, &addf>();
	testOperatorsWithFloat<Type, &Type::operator-, &Type::operator-=, &operator-, &subf>();
	testOperatorsWithFloat<Type, &Type::operator*, &Type::operator*=, &operator*, &mulf>();
	testOperatorsWithFloat<Type, &Type::operator/, &Type::operator/=, &operator/, &divf>();
	testCmpOperators<Type>();
}


TEST(MathTests, MatrixOperators)
{
	arithmeticOperations<Mat3>();
	arithmeticOperations<Mat4>();
}

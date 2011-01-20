#include <gtest/gtest.h>
#include <iostream>
#include "Math.h"
#include "MathCommon.ut.h"


template<typename Type>
void arithmeticOperations()
{
	/*testOperators<Type, &Type::operator+, &Type::operator+=, &addf>();
	testOperators<Type, &Type::operator-, &Type::operator-=, &subf>();
	testOperatorsWithFloat<Type, &Type::operator+, &Type::operator+=, &operator+, &addf>();
	testOperatorsWithFloat<Type, &Type::operator-, &Type::operator-=, &operator-, &subf>();
	testOperatorsWithFloat<Type, &Type::operator*, &Type::operator*=, &operator*, &mulf>();*/
	testOperatorsWithFloat<Type, &Type::operator/, &Type::operator/=, &operator/, &divf>();
	/*testCmpOperators<Type>();*/
}


TEST(MathTests, MatrixOperators)
{
	arithmeticOperations<Mat3>();
	arithmeticOperations<Mat4>();
}

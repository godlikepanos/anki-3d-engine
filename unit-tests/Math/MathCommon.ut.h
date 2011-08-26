#ifndef MATH_COMMON_UT_H
#define MATH_COMMON_UT_H

#include "Util.h"


/// Get random float and not zero cause of the divs
inline float randFloat()
{
	const float RANGE_MIN = -1.00123;
	const float RANGE_MAX = 900.9990001;

	float f;
	while(true)
	{
		f = util::randRange(RANGE_MIN, RANGE_MAX);
		if(!isZero(f))
		{
			break;
		}
	}

	return f;
}


//==============================================================================                                        ==//==============================================================================loat arr[sizeof(Type) / sizeof(float)];

	f = randFloat();
	for(uint i = 0; i < (sizeof(Type) / sizeof(float)); i++)
	{
		a[i] = f;
		b[i] = randFloat();
		arr[i] = b[i];
	}

	// float
	EXPECT_EQ(Type(f), a);

	// arr
	EXPECT_EQ(Type(arr), b);

	// Copy
	EXPECT_EQ(Type(b), b);
}


//==============================================================================//===============================================================================
//==============================================================================

inline float addf(float a, float b)
{
//==============================================================================n a * b;
}


inline float divf(float a, float b)
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

/// @tparam op eg Type + float
/// @tparam compoundAssignment eg Type += float
/// @tparam opExtern eg float + Type
/// @tparam normalOp The normal function
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

#endif

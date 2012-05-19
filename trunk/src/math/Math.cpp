#include "anki/math/MathCommonSrc.h"


namespace anki {


//==============================================================================
const float Math::PI = 3.14159265358979323846;
const float Math::EPSILON = 1.0e-6;


//==============================================================================
float Math::polynomialSinQuadrant(const float a)
{
	return a * (1.0 + a * a * (-0.16666 + a * a *
		(0.0083143 - a * a * 0.00018542)));
}


//==============================================================================
void Math::sinCos(const float a_, float& sina, float& cosa)
{
	bool negative = false;
	float a = a_;
	if(a < 0.0)
	{
		a = -a;
		negative = true;
	}
	const float TWO_OVER_PI = 1.0 / (PI / 2.0);
	float floatA = TWO_OVER_PI * a;
	int intA = (int)floatA;

	const float RATIONAL_HALF_PI = 201 / 128.0;
	const float REMAINDER_HALF_PI = 4.8382679e-4;

	floatA = (a - RATIONAL_HALF_PI * intA) - REMAINDER_HALF_PI * intA;

	float floatAMinusHalfPi = (floatA - RATIONAL_HALF_PI) - REMAINDER_HALF_PI;

	switch(intA & 3)
	{
		case 0: // 0 - Pi/2
			sina = polynomialSinQuadrant(floatA);
			cosa = polynomialSinQuadrant(-floatAMinusHalfPi);
			break;
		case 1: // Pi/2 - Pi
			sina = polynomialSinQuadrant(-floatAMinusHalfPi);
			cosa = polynomialSinQuadrant(-floatA);
			break;
		case 2: // Pi - 3Pi/2
			sina = polynomialSinQuadrant(-floatA);
			cosa = polynomialSinQuadrant(floatAMinusHalfPi);
			break;
		case 3: // 3Pi/2 - 2Pi
			sina = polynomialSinQuadrant(floatAMinusHalfPi);
			cosa = polynomialSinQuadrant(floatA);
			break;
	};

	if(negative)
	{
		sina = -sina;
	}
}


} // end namespace

#include "MathCommonSrc.h"


//==============================================================================
// Statics                                                                     =
//==============================================================================

const float Math::PI = 3.14159265358979323846;
const float Math::EPSILON = 1.0e-6;


//==============================================================================
// polynomialSinQuadrant                                                       =
//==============================================================================
float Math::polynomialSinQuadrant(float a)
{
	return a * (1.0 + a * a * (-0.16666 + a * a *
		(0.0083143 - a * a * 0.00018542)));
}


//==============================================================================
// sinCos                                                                      =
//==============================================================================
void Math::sinCos(float a, float& sina, float& cosa)
{
	bool negative = false;
	if(a < 0.0)
	{
		a = -a;
		negative = true;
	}
	const float kTwoOverPi = 1.0 / (Math::PI / 2.0);
	float floatA = kTwoOverPi * a;
	int intA = (int)floatA;

	const float k_rational_half_pi = 201 / 128.0;
	const float kRemainderHalfPi = 4.8382679e-4;

	floatA = (a - k_rational_half_pi * intA) - kRemainderHalfPi * intA;

	float floatAMinusHalfPi = (floatA - k_rational_half_pi) - kRemainderHalfPi;

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

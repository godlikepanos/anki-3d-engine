#include "anki/math/MathCommonSrc.h"

namespace anki {

//==============================================================================
F32 Math::polynomialSinQuadrant(const F32 a)
{
	return a * (1.0 + a * a * (-0.16666 + a * a *
		(0.0083143 - a * a * 0.00018542)));
}

//==============================================================================
void Math::sinCos(const F32 a_, F32& sina, F32& cosa)
{
#if NDEBUG
	Bool negative = false;
	F32 a = a_;
	if(a < 0.0)
	{
		a = -a;
		negative = true;
	}
	const F32 TWO_OVER_PI = 1.0 / (PI / 2.0);
	F32 floatA = TWO_OVER_PI * a;
	I intA = (int)floatA;

	const F32 RATIONAL_HALF_PI = 201 / 128.0;
	const F32 REMAINDER_HALF_PI = 4.8382679e-4;

	floatA = (a - RATIONAL_HALF_PI * intA) - REMAINDER_HALF_PI * intA;

	F32 floatAMinusHalfPi = (floatA - RATIONAL_HALF_PI) - REMAINDER_HALF_PI;

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
#else
	sina = ::sin(a_);
	cosa = ::cos(a_);
#endif
}

} // end namespace anki

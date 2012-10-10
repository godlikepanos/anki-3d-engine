#include "anki/math/Functions.h"

namespace anki {

//==============================================================================
template<typename Scalar>
static Scalar polynomialSinQuadrant(const Scalar a)
{
	return a * (1.0 + a * a * (-0.16666 + a * a *
		(0.0083143 - a * a * 0.00018542)));
}

//==============================================================================
template<typename Scalar>
static void sinCosInternal(const Scalar a_, Scalar& sina, Scalar& cosa)
{
#if NDEBUG
	Bool negative = false;
	Scalar a = a_;
	if(a < 0.0)
	{
		a = -a;
		negative = true;
	}
	const Scalar TWO_OVER_PI = 1.0 / (getPi<Scalar>() / 2.0);
	Scalar floatA = TWO_OVER_PI * a;
	I intA = (int)floatA;

	const Scalar RATIONAL_HALF_PI = 201 / 128.0;
	const Scalar REMAINDER_HALF_PI = 4.8382679e-4;

	floatA = (a - RATIONAL_HALF_PI * intA) - REMAINDER_HALF_PI * intA;

	Scalar floatAMinusHalfPi = (floatA - RATIONAL_HALF_PI) - REMAINDER_HALF_PI;

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
	sina = sin(a_);
	cosa = cos(a_);
#endif
}

//==============================================================================
void sinCos(const F32 a, F32& sina, F32& cosa)
{
	sinCosInternal(a, sina, cosa);
}

//==============================================================================
void sinCos(const F64 a, F64& sina, F64& cosa)
{
	sinCosInternal(a, sina, cosa);
}

} // end namespace anki

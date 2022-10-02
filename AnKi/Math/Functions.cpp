// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Math/Functions.h>
#include <AnKi/Config.h>

namespace anki {

template<typename Scalar>
static Scalar polynomialSinQuadrant(const Scalar a)
{
	return Scalar(a * (1.0 + a * a * (-0.16666 + a * a * (0.0083143 - a * a * 0.00018542))));
}

template<typename Scalar>
static void sinCosInternal(const Scalar a_, Scalar& sina, Scalar& cosa)
{
#if ANKI_EXTRA_CHECKS
	sina = sin(a_);
	cosa = cos(a_);
#else
	Bool negative = false;
	Scalar a = a_;
	if(a < 0.0)
	{
		a = -a;
		negative = true;
	}
	constexpr Scalar k2OverPi = Scalar(1.0 / (kPi / 2.0));
	Scalar floatA = k2OverPi * a;
	I intA = (int)floatA;

	constexpr Scalar kRationalHalfPi = 201 / 128.0;
	constexpr Scalar kRemainderHalfPi = Scalar(4.8382679e-4);

	floatA = (a - kRationalHalfPi * Scalar(intA)) - kRemainderHalfPi * Scalar(intA);

	Scalar floatAMinusHalfPi = (floatA - kRationalHalfPi) - kRemainderHalfPi;

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
#endif
}

void sinCos(const F32 a, F32& sina, F32& cosa)
{
	sinCosInternal(a, sina, cosa);
}

void sinCos(const F64 a, F64& sina, F64& cosa)
{
	sinCosInternal(a, sina, cosa);
}

} // end namespace anki

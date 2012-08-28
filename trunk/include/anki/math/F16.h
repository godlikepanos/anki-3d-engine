#ifndef ANKI_MATH_F16_H
#define ANKI_MATH_F16_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Half float
class F16
{
public:
	explicit F16();
	explicit F16(const F32 f);
	F16(const F16& a);

	/// @name Operators with same type
	/// @{
	F16& operator=(const F16& b);
	F16 operator+(const F16& b) const;
	F16& operator+=(const F16& b);
	F16 operator-(const F16& b) const;
	F16& operator-=(const F16& b);
	F16 operator*(const F16& b) const;
	F16& operator*=(const F16& b);
	F16 operator/(const F16& b) const;
	F16& operator/=(const F16& b);
	Bool operator==(const F16& b) const;
	Bool operator!=(const F16& b) const;
	/// @}

	static F32 toF32(F16 h);
	static F16 toF16(F32 f);
private:
	U16 data;
};
/// @}

} // end namespace anki

#endif

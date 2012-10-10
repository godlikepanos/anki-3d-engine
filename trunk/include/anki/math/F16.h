#ifndef ANKI_MATH_F16_H
#define ANKI_MATH_F16_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Half float
class F16
{
public:
	/// @name Constructors
	/// @{
	explicit F16();
	F16(const F16& a);
	explicit F16(const F32 f);
	/// @}

	/// @name Operators with same type
	/// @{
	F16& operator=(const F16 b);
	F16 operator+(const F16 b) const;
	F16& operator+=(const F16 b);
	F16 operator-(const F16 b) const;
	F16& operator-=(const F16 b);
	F16 operator*(const F16 b) const;
	F16& operator*=(const F16 b);
	F16 operator/(const F16 b) const;
	F16& operator/=(const F16 b);
	Bool operator==(const F16 b) const;
	Bool operator!=(const F16 b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	F16& operator=(const F32 b);
	F32 operator+(const F32 b) const;
	F16& operator+=(const F32 b);
	F32 operator-(const F32 b) const;
	F16& operator-=(const F32 b);
	F32 operator*(const F32 b) const;
	F16& operator*=(const F32 b);
	F32 operator/(const F32 b) const;
	F16& operator/=(const F32 b);
	Bool operator==(const F32 b) const;
	Bool operator!=(const F32 b) const;
	/// @}

	/// @name Other
	/// @{
	F32 toF32() const;
	/// @}

	/// @name Friends
	/// @{
	friend F32 operator+(const F32 f, const F16 h);
	friend F32 operator-(const F32 f, const F16 h);
	friend F32 operator*(const F32 f, const F16 h);
	friend F32 operator/(const F32 f, const F16 h);
	friend std::ostream& operator<<(std::ostream& s, const F16& m);
	/// @}
private:
	U16 data;

	static F32 toF32(F16 h);
	static F16 toF16(F32 f);
};
/// @}

static_assert(sizeof(F16) == 2, "Incorrect size");

} // end namespace anki

#include "anki/math/F16.inl.h"

#endif

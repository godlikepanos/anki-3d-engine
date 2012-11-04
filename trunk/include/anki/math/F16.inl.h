#include "anki/math/CommonSrc.h"

namespace anki {

//==============================================================================
// Constructors                                                                =
//==============================================================================

// Default
inline F16::F16()
{}

// Copy
inline F16::F16(const F16& b)
{
	data = b.data;
}

// F32
inline F16::F16(const F32 f)
{
	*this = toF16(f);
}

// U16
inline F16::F16(const U16 ui)
{
	data = ui;
}

//==============================================================================
// Operators with same                                                         =
//==============================================================================

// Copy
inline F16& F16::operator=(const F16 b)
{
	data = b.data;
	return *this;
}

// +
inline F16 F16::operator+(const F16 b) const
{
	return toF16(toF32() + b.toF32());
}

// +=
inline F16& F16::operator+=(const F16 b)
{
	*this = toF16(toF32() + b.toF32());
	return *this;
}

// -
inline F16 F16::operator-(const F16 b) const
{
	return toF16(toF32() - b.toF32());
}

// -=
inline F16& F16::operator-=(const F16 b)
{
	*this = toF16(toF32() - b.toF32());
	return *this;
}

// *
inline F16 F16::operator*(const F16 b) const
{
	return toF16(toF32() * b.toF32());
}

// *=
inline F16& F16::operator*=(const F16 b)
{
	*this = toF16(toF32() * b.toF32());
	return *this;
}

// /
inline F16 F16::operator/(const F16 b) const
{
	return toF16(toF32() / b.toF32());
}

// /=
inline F16& F16::operator/=(const F16 b)
{
	*this = toF16(toF32() / b.toF32());
	return *this;
}

// ==
inline Bool F16::operator==(const F16 b) const
{
	return toF32() == b.toF32();
}

// !=
inline Bool F16::operator!=(const F16 b) const
{
	return toF32() != b.toF32();
}

//==============================================================================
// Operators with F32                                                          =
//==============================================================================

// Copy
inline F16& F16::operator=(const F32 b)
{
	*this = toF16(b);
	return *this;
}

// +
inline F32 F16::operator+(const F32 b) const
{
	return toF32() + b;
}

// +=
inline F16& F16::operator+=(const F32 b)
{
	*this = toF16(toF32() + b);
	return *this;
}

// -
inline F32 F16::operator-(const F32 b) const
{
	return toF32() - b;
}

// -=
inline F16& F16::operator-=(const F32 b)
{
	*this = toF16(toF32() - b);
	return *this;
}

// *
inline F32 F16::operator*(const F32 b) const
{
	return toF32() * b;
}

// *=
inline F16& F16::operator*=(const F32 b)
{
	*this = toF16(toF32() * b);
	return *this;
}

// /
inline F32 F16::operator/(const F32 b) const
{
	return toF32() / b;
}

// /=
inline F16& F16::operator/=(const F32 b)
{
	*this = toF16(toF32() / b);
	return *this;
}

// ==
inline Bool F16::operator==(const F32 b) const
{
	return isZero(toF32() - b);
}

// !=
inline Bool F16::operator!=(const F32 b) const
{
	return !isZero(toF32() - b);
}

//==============================================================================
// Other                                                                       =
//==============================================================================

// toF32
inline F32 F16::toF32() const
{
	return toF32(*this);
}

// toU16
inline U16 F16::toU16() const
{
	return data;
}

//==============================================================================
// Other                                                                       =
//==============================================================================

// F32 + F16
inline F32 operator+(const F32 f, const F16 h)
{
	return f + h.toF32();
}

// F32 - F16
inline F32 operator-(const F32 f, const F16 h)
{
	return f - h.toF32();
}

// F32 * F16
inline F32 operator*(const F32 f, const F16 h)
{
	return f * h.toF32();
}

// F32 / F16
inline F32 operator/(const F32 f, const F16 h)
{
	return f / h.toF32();
}

// Print
inline std::ostream& operator<<(std::ostream& s, const F16& m)
{
	s << m.toF32();
	return s;
}

} // end namespace anki

#ifndef ANKI_MATH_F16_H
#define ANKI_MATH_F16_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// Half float
class F16
{
	/// @name Friends
	/// @{
	friend F32 operator+(const F32 f, const F16 h);
	friend F32 operator-(const F32 f, const F16 h);
	friend F32 operator*(const F32 f, const F16 h);
	friend F32 operator/(const F32 f, const F16 h);
	/// @}

public:
	/// @name Constructors
	/// @{
	explicit F16()
	{}

	F16(const F16& b)
	{
		data = b.data;
	}

	explicit F16(const F32 f)
	{
		*this = toF16(f);
	}

	explicit F16(const U16 ui)
	{
		data = ui;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	F16& operator=(const F16 b)
	{
		data = b.data;
		return *this;
	}

	F16 operator+(const F16 b) const
	{
		return toF16(toF32() + b.toF32());
	}

	F16& operator+=(const F16 b)
	{
		*this = toF16(toF32() + b.toF32());
		return *this;
	}

	F16 operator-(const F16 b) const
	{
		return toF16(toF32() - b.toF32());
	}

	F16& operator-=(const F16 b)
	{
		*this = toF16(toF32() - b.toF32());
		return *this;
	}

	F16 operator*(const F16 b) const
	{
		return toF16(toF32() * b.toF32());
	}

	F16& operator*=(const F16 b)
	{
		*this = toF16(toF32() * b.toF32());
		return *this;
	}

	F16 operator/(const F16 b) const
	{
		return toF16(toF32() / b.toF32());
	}

	F16& operator/=(const F16 b)
	{
		*this = toF16(toF32() / b.toF32());
		return *this;
	}

	Bool operator==(const F16 b) const
	{
		return data == b.data;
	}

	Bool operator!=(const F16 b) const
	{
		return data != b.data;
	}
	/// @}

	/// @name Operators with F32
	/// @{
	F16& operator=(const F32 b)
	{
		*this = toF16(b);
		return *this;
	}

	F32 operator+(const F32 b) const
	{
		return toF32() + b;
	}

	F16& operator+=(const F32 b)
	{
		*this = toF16(toF32() + b);
		return *this;
	}

	F32 operator-(const F32 b) const
	{
		return toF32() - b;
	}

	F16& operator-=(const F32 b)
	{
		*this = toF16(toF32() - b);
		return *this;
	}

	F32 operator*(const F32 b) const
	{
		return toF32() * b;
	}

	F16& operator*=(const F32 b)
	{
		*this = toF16(toF32() * b);
		return *this;
	}

	F32 operator/(const F32 b) const
	{
		return toF32() / b;
	}

	F16& operator/=(const F32 b)
	{
		*this = toF16(toF32() / b);
		return *this;
	}

	Bool operator==(const F32 b) const
	{
		return toF32() == b;
	}

	Bool operator!=(const F32 b) const
	{
		return toF32() != b;
	}
	/// @}

	/// @name Other
	/// @{
	F32 toF32() const
	{
		return toF32(*this);
	}

	U16 toU16() const
	{
		return data;
	}
	/// @}

private:
	U16 data;

	static F32 toF32(F16 h);
	static F16 toF16(F32 f);
};

/// @name F16 friends
/// @{
inline F32 operator+(const F32 f, const F16 h)
{
	return f + h.toF32();
}

inline F32 operator-(const F32 f, const F16 h)
{
	return f - h.toF32();
}

inline F32 operator*(const F32 f, const F16 h)
{
	return f * h.toF32();
}

inline F32 operator/(const F32 f, const F16 h)
{
	return f / h.toF32();
}
/// @}
/// @}

static_assert(sizeof(F16) == 2, "Incorrect size");

} // end namespace anki

#endif

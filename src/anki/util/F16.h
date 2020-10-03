// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

/// @addtogroup util_other
/// @{

/// Half float
class F16
{
	/// @name Friends with F32
	/// @{
	friend F32 operator+(const F32 f, const F16 h)
	{
		return f + h.toF32();
	}

	friend F32 operator-(const F32 f, const F16 h)
	{
		return f - h.toF32();
	}

	friend F32 operator*(const F32 f, const F16 h)
	{
		return f * h.toF32();
	}

	friend F32 operator/(const F32 f, const F16 h)
	{
		return f / h.toF32();
	}
	/// @}

	/// @name Friends with F64
	/// @{
	friend F64 operator+(const F64 f, const F16 h)
	{
		return f + h.toF32();
	}

	friend F64 operator-(const F64 f, const F16 h)
	{
		return f - h.toF32();
	}

	friend F64 operator*(const F64 f, const F16 h)
	{
		return f * h.toF32();
	}

	friend F64 operator/(const F64 f, const F16 h)
	{
		return f / h.toF32();
	}
	/// @}

public:
	/// @name Constructors
	/// @{
	F16()
	{
		static_assert(sizeof(F16) == 2, "Incorrect size");
	}

	F16(const F16& b)
	{
		m_data = b.m_data;
	}

	explicit F16(const F64 f)
	{
		*this = toF16(F32(f));
	}

	explicit F16(const F32 f)
	{
		*this = toF16(f);
	}

	explicit F16(const U16 ui)
	{
		m_data = ui;
	}
	/// @}

	/// @name Operators with same type
	/// @{
	F16& operator=(const F16 b)
	{
		m_data = b.m_data;
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
		return m_data == b.m_data;
	}

	Bool operator!=(const F16 b) const
	{
		return m_data != b.m_data;
	}
	/// @}

	/// @name Operators with F32
	/// @{
	F32 operator+(const F32 b) const
	{
		return toF32() + b;
	}

	F32 operator-(const F32 b) const
	{
		return toF32() - b;
	}

	F32 operator*(const F32 b) const
	{
		return toF32() * b;
	}

	F32 operator/(const F32 b) const
	{
		return toF32() / b;
	}
	/// @}

	/// @name Operators with F64
	/// @{
	F64 operator+(const F64 b) const
	{
		return toF32() + b;
	}

	F64 operator-(const F64 b) const
	{
		return toF32() - b;
	}

	F64 operator*(const F64 b) const
	{
		return toF32() * b;
	}

	F64 operator/(const F64 b) const
	{
		return toF32() / b;
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
		return m_data;
	}
	/// @}

private:
	U16 m_data;

	static F32 toF32(F16 h);
	static F16 toF16(F32 f);
};
/// @}

} // end namespace anki

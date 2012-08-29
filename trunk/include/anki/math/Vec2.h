#ifndef ANKI_MATH_VEC2_H
#define ANKI_MATH_VEC2_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 2D vector
class Vec2
{
public:
	/// @name Constructors
	/// @{
	explicit Vec2();
	explicit Vec2(const F32 x, const F32 y);
	explicit Vec2(const F32 f);
	explicit Vec2(const F32 arr[]);
	Vec2(const Vec2& b);
	explicit Vec2(const Vec3& v3);
	explicit Vec2(const Vec4& v4);
	/// @}

	/// @name Accessors
	/// @{
	F32& x();
	F32 x() const;
	F32& y();
	F32 y() const;
	F32& operator[](const U i);
	F32 operator[](const U i) const;
	/// @}

	/// @name Operators with same type
	/// @{
	Vec2& operator=(const Vec2& b);
	Vec2 operator+(const Vec2& b) const;
	Vec2& operator+=(const Vec2& b);
	Vec2 operator-(const Vec2& b) const;
	Vec2& operator-=(const Vec2& b);
	Vec2 operator*(const Vec2& b) const;
	Vec2& operator*=(const Vec2& b);
	Vec2 operator/(const Vec2& b) const;
	Vec2& operator/=(const Vec2& b);
	Vec2 operator-() const;
	Bool operator==(const Vec2& b) const;
	Bool operator!=(const Vec2& b) const;
	Bool operator<(const Vec2& b) const;
	Bool operator<=(const Vec2& b) const;
	Bool operator>(const Vec2& b) const;
	Bool operator>=(const Vec2& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Vec2 operator+(const F32 f) const;
	Vec2& operator+=(const F32 f);
	Vec2 operator-(const F32 f) const;
	Vec2& operator-=(const F32 f);
	Vec2 operator*(const F32 f) const;
	Vec2& operator*=(const F32 f);
	Vec2 operator/(const F32 f) const;
	Vec2& operator/=(const F32 f);
	/// @}

	/// @name Other
	/// @{
	F32 getLengthSquared() const;
	F32 getLength() const;
	Vec2 getNormalized() const;
	void normalize();
	F32 dot(const Vec2& b) const;
	/// @}

	/// @name Friends
	friend Vec2 operator+(const F32 f, const Vec2& v2);
	friend Vec2 operator-(const F32 f, const Vec2& v2);
	friend Vec2 operator*(const F32 f, const Vec2& v2);
	friend Vec2 operator/(const F32 f, const Vec2& v2);
	friend std::ostream& operator<<(std::ostream& s, const Vec2& v);
	///@]

private:
	/// @name Data members
	/// @{
	union
	{
		struct
		{
			F32 x, y;
		} vec;

		std::array<F32, 2> arr;
	};
	/// @}
};
/// @

static_assert(sizeof(Vec2) == sizeof(F32) * 2, "Incorrect size");

} // end namespace anki

#include "anki/math/Vec2.inl.h"

#endif

#ifndef ANKI_MATH_VEC4_H
#define ANKI_MATH_VEC4_H

#include "anki/math/MathCommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 4D vector. SSE optimized
class Vec4
{
public:
	/// @name Constructors
	/// @{
	explicit Vec4();
	explicit Vec4(const F32 x, const F32 y, const F32 z,
		const F32 w);
	explicit Vec4(const F32 f);
	explicit Vec4(const F32 arr[]);
	explicit Vec4(const Vec2& v2, const F32 z, const F32 w);
	explicit Vec4(const Vec2& av2, const Vec2& bv2);
	explicit Vec4(const Vec3& v3, const F32 w);
	Vec4(const Vec4& b);
	explicit Vec4(const Quat& q);
#if defined(ANKI_MATH_INTEL_SIMD)
	explicit Vec4(const __m128& mm);
#endif
	/// @}

	/// @name Accessors
	/// @{
	F32& x();
	F32 x() const;
	F32& y();
	F32 y() const;
	F32& z();
	F32 z() const;
	F32& w();
	F32 w() const;
	F32& operator[](const U i);
	F32 operator[](const U i) const;
#if defined(ANKI_MATH_INTEL_SIMD)
	__m128& getMm();
	const __m128& getMm() const;
#endif
	Vec2 xy() const;
	Vec3 xyz() const;
	/// @}

	/// @name Operators with same type
	/// @{
	Vec4& operator=(const Vec4& b);
	Vec4 operator+(const Vec4& b) const;
	Vec4& operator+=(const Vec4& b);
	Vec4 operator-(const Vec4& b) const;
	Vec4& operator-=(const Vec4& b);
	Vec4 operator*(const Vec4& b) const;
	Vec4& operator*=(const Vec4& b);
	Vec4 operator/(const Vec4& b) const;
	Vec4& operator/=(const Vec4& b);
	Vec4 operator-() const;
	Bool operator==(const Vec4& b) const;
	Bool operator!=(const Vec4& b) const;
	Bool operator<(const Vec4& b) const;
	Bool operator<=(const Vec4& b) const;
	Bool operator>(const Vec4& b) const;
	Bool operator>=(const Vec4& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Vec4 operator+(const F32 f) const;
	Vec4& operator+=(const F32 f);
	Vec4 operator-(const F32 f) const;
	Vec4& operator-=(const F32 f);
	Vec4 operator*(const F32 f) const;
	Vec4& operator*=(const F32 f);
	Vec4 operator/(const F32 f) const;
	Vec4& operator/=(const F32 f);
	/// @}

	/// @name Operators with other
	/// @{
	Vec4 operator*(const Mat4& m4) const;
	/// @}

	/// @name Other
	/// @{
	F32 getLength() const;
	Vec4 getNormalized() const;
	void normalize();
	F32 dot(const Vec4& b) const;
	/// @}

	/// @name Friends
	/// @{
	friend Vec4 operator+(const F32 f, const Vec4& v4);
	friend Vec4 operator-(const F32 f, const Vec4& v4);
	friend Vec4 operator*(const F32 f, const Vec4& v4);
	friend Vec4 operator/(const F32 f, const Vec4& v4);
	friend std::ostream& operator<<(std::ostream& s, const Vec4& v);
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			F32 x, y, z, w;
		} vec;

		std::array<F32, 4> arr;

#if defined(ANKI_MATH_INTEL_SIMD)
		__m128 mm;
#endif
	};
	/// @}
};
/// @}

static_assert(sizeof(Vec4) == sizeof(F32) * 4, "Incorrect size");

} // end namespace

#include "anki/math/Vec4.inl.h"

#endif

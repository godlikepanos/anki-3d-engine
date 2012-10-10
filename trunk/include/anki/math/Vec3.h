#ifndef ANKI_MATH_VEC3_H
#define ANKI_MATH_VEC3_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 3D vector. One of the most used classes
class Vec3
{
public:
	/// @name Constructors
	/// @{
	explicit Vec3();
	explicit Vec3(const F32 x, const F32 y, const F32 z);
	explicit Vec3(const F32 f);
	explicit Vec3(const F32 arr[]);
	explicit Vec3(const Vec2& v2, const F32 z);
	Vec3(const Vec3& b);
	explicit Vec3(const Vec4& v4);
	explicit Vec3(const Quat& q);
	/// @}

	/// @name Accessors
	/// @{
	F32& x();
	F32 x() const;
	F32& y();
	F32 y() const;
	F32& z();
	F32 z() const;
	F32& operator[](const U i);
	F32 operator[](const U i) const;
	Vec2 xy() const;
	/// @}

	/// @name Operators with same type
	/// @{
	Vec3& operator=(const Vec3& b);
	Vec3 operator+(const Vec3& b) const;
	Vec3& operator+=(const Vec3& b);
	Vec3 operator-(const Vec3& b) const;
	Vec3& operator-=(const Vec3& b);
	Vec3 operator*(const Vec3& b) const;
	Vec3& operator*=(const Vec3& b);
	Vec3 operator/(const Vec3& b) const;
	Vec3& operator/=(const Vec3& b);
	Vec3 operator-() const;
	Bool operator==(const Vec3& b) const;
	Bool operator!=(const Vec3& b) const;
	Bool operator<(const Vec3& b) const;
	Bool operator<=(const Vec3& b) const;
	Bool operator>(const Vec3& b) const;
	Bool operator>=(const Vec3& b) const;
	/// @}

	/// @name Operators with F32
	/// @{
	Vec3 operator+(const F32 f) const;
	Vec3& operator+=(const F32 f);
	Vec3 operator-(const F32 f) const;
	Vec3& operator-=(const F32 f);
	Vec3 operator*(const F32 f) const;
	Vec3& operator*=(const F32 f);
	Vec3 operator/(const F32 f) const;
	Vec3& operator/=(const F32 f);
	/// @}

	/// @name Operators with other types
	/// @{
	Vec3 operator*(const Mat3& m3) const;
	/// @}

	/// @name Other
	/// @{
	F32 dot(const Vec3& b) const; ///< 3 muls, 2 adds
	Vec3 cross(const Vec3& b) const; ///< 6 muls, 3 adds
	F32 getLength() const;
	F32 getLengthSquared() const;
	F32 getDistanceSquared(const Vec3& b) const;
	void normalize();
	Vec3 getNormalized() const;
	Vec3 getProjection(const Vec3& toThis) const;
	/// Returns q * this * q.Conjucated() aka returns a rotated this.
	/// 18 muls, 12 adds
	Vec3 getRotated(const Quat& q) const;
	void rotate(const Quat& q);
	Vec3 lerp(const Vec3& v1, F32 t) const; ///< Return lerp(this, v1, t)
	/// @}

	/// @name Transformations
	/// The faster way is by far the Mat4 * Vec3 or the
	/// getTransformed(const Vec3&, const Mat3&)
	/// @{
	Vec3 getTransformed(const Vec3& translate, const Mat3& rotate,
		F32 scale) const;
	void transform(const Vec3& translate, const Mat3& rotate, F32 scale);
	Vec3 getTransformed(const Vec3& translate, const Mat3& rotate) const;
	void transform(const Vec3& translate, const Mat3& rotate);
	Vec3 getTransformed(const Vec3& translate, const Quat& rotate,
		F32 scale) const;
	void transform(const Vec3& translate, const Quat& rotate, F32 scale);
	Vec3 getTransformed(const Vec3& translate, const Quat& rotate) const;
	void transform(const Vec3& translate, const Quat& rotate);
	Vec3 getTransformed(const Mat4& transform) const;  ///< 9 muls, 9 adds
	void transform(const Mat4& transform);
	Vec3 getTransformed(const Transform& transform) const;
	void transform(const Transform& transform);
	/// @}

	/// @name Friends
	/// @{
	friend Vec3 operator+(const F32 f, const Vec3& v);
	friend Vec3 operator-(const F32 f, const Vec3& v);
	friend Vec3 operator*(const F32 f, const Vec3& v);
	friend Vec3 operator/(const F32 f, const Vec3& v);
	friend std::ostream& operator<<(std::ostream& s, const Vec3& v);
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			F32 x, y, z;
		} vec;

		Array<F32, 3> arr;
	};
	/// @}
};
/// @}

static_assert(sizeof(Vec3) == sizeof(F32) * 3, "Incorrect size");

} // end namespace

#include "anki/math/Vec3.inl.h"

#endif

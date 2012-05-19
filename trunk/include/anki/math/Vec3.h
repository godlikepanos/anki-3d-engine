#ifndef ANKI_MATH_VEC3_H
#define ANKI_MATH_VEC3_H

#include "anki/math/MathCommonIncludes.h"


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
	explicit Vec3(const float x, const float y, const float z);
	explicit Vec3(const float f);
	explicit Vec3(const float arr[]);
	explicit Vec3(const Vec2& v2, const float z);
	Vec3(const Vec3& b);
	explicit Vec3(const Vec4& v4);
	explicit Vec3(const Quat& q);
	/// @}

	/// @name Accessors
	/// @{
	float& x();
	float x() const;
	float& y();
	float y() const;
	float& z();
	float z() const;
	float& operator[](const size_t i);
	float operator[](const size_t i) const;
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
	bool operator==(const Vec3& b) const;
	bool operator!=(const Vec3& b) const;
	bool operator<(const Vec3& b) const;
	bool operator<=(const Vec3& b) const;
	bool operator>(const Vec3& b) const;
	bool operator>=(const Vec3& b) const;
	/// @}

	/// @name Operators with float
	/// @{
	Vec3 operator+(const float f) const;
	Vec3& operator+=(const float f);
	Vec3 operator-(const float f) const;
	Vec3& operator-=(const float f);
	Vec3 operator*(const float f) const;
	Vec3& operator*=(const float f);
	Vec3 operator/(const float f) const;
	Vec3& operator/=(const float f);
	/// @}

	/// @name Operators with other types
	/// @{
	Vec3 operator*(const Mat3& m3) const;
	/// @}

	/// @name Other
	/// @{
	float dot(const Vec3& b) const; ///< 3 muls, 2 adds
	Vec3 cross(const Vec3& b) const; ///< 6 muls, 3 adds
	float getLength() const;
	float getLengthSquared() const;
	float getDistanceSquared(const Vec3& b) const;
	void normalize();
	Vec3 getNormalized() const;
	Vec3 getProjection(const Vec3& toThis) const;
	/// Returns q * this * q.Conjucated() aka returns a rotated this.
	/// 18 muls, 12 adds
	Vec3 getRotated(const Quat& q) const;
	void rotate(const Quat& q);
	Vec3 lerp(const Vec3& v1, float t) const; ///< Return lerp(this, v1, t)
	/// @}

	/// @name Transformations
	/// The faster way is by far the Mat4 * Vec3 or the
	/// getTransformed(const Vec3&, const Mat3&)
	/// @{
	Vec3 getTransformed(const Vec3& translate, const Mat3& rotate,
		float scale) const;
	void transform(const Vec3& translate, const Mat3& rotate, float scale);
	Vec3 getTransformed(const Vec3& translate, const Mat3& rotate) const;
	void transform(const Vec3& translate, const Mat3& rotate);
	Vec3 getTransformed(const Vec3& translate, const Quat& rotate,
		float scale) const;
	void transform(const Vec3& translate, const Quat& rotate, float scale);
	Vec3 getTransformed(const Vec3& translate, const Quat& rotate) const;
	void transform(const Vec3& translate, const Quat& rotate);
	Vec3 getTransformed(const Mat4& transform) const;  ///< 9 muls, 9 adds
	void transform(const Mat4& transform);
	Vec3 getTransformed(const Transform& transform) const;
	void transform(const Transform& transform);
	/// @}

	/// @name Friends
	/// @{
	friend Vec3 operator+(const float f, const Vec3& v);
	friend Vec3 operator-(const float f, const Vec3& v);
	friend Vec3 operator*(const float f, const Vec3& v);
	friend Vec3 operator/(const float f, const Vec3& v);
	friend std::ostream& operator<<(std::ostream& s, const Vec3& v);
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			float x, y, z;
		} vec;

		std::array<float, 3> arr;
	};
	/// @}
};
/// @}


} // end namespace


#include "anki/math/Vec3.inl.h"


#endif

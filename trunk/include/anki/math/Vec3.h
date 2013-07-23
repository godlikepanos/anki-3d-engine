#ifndef ANKI_MATH_VEC3_H
#define ANKI_MATH_VEC3_H

#include "anki/math/CommonIncludes.h"

namespace anki {

/// @addtogroup Math
/// @{

/// 3D vector template. One of the most used classes
template<typename T>
class TVec3
{
public:
	/// @name Constructors
	/// @{
	explicit TVec3()
	{}

	explicit TVec3(const T x_, const T y_, const T z_)
	{
		x() = x_;
		y() = y_;
		z() = z_;
	}

	explicit TVec3(const T f)
	{
		arr[0] = arr[1] = arr[2] = f;
	}

	explicit TVec3(const T arr_[])
	{
		arr[0] = arr_[0];
		arr[1] = arr_[1];
		arr[2] = arr_[2];
	}

	explicit TVec3(const TVec2<T>& v2, const T z_)
	{
		x() = v2.x();
		y() = v2.y();
		z() = z_;
	}

	TVec3(const TVec3& b)
	{
		arr[0] = b.arr[0];
		arr[1] = b.arr[1];
		arr[2] = b.arr[2];
	}

	explicit TVec3(const TVec4<T>& v4)
	{
		arr[0] = v4[0];
		arr[1] = v4[1];
		arr[2] = v4[2];
	}

	explicit TVec3(const TQuat<T>& q)
	{
		x() = q.x();
		y() = q.y();
		z() = q.z();
	}
	/// @}

	/// @name Accessors
	/// @{
	T& x()
	{
		return vec.x;
	}

	T x() const
	{
		return vec.x;
	}

	T& y()
	{
		return vec.y;
	}

	T y() const
	{
		return vec.y;
	}

	T& z()
	{
		return vec.z;
	}

	T z() const
	{
		return vec.z;
	}

	T& operator[](const U i)
	{
		return arr[i];
	}

	T operator[](const U i) const
	{
		return arr[i];
	}

	TVec2<T> xy() const
	{
		return TVec2<T>(x(), y());
	}
	/// @}

	/// @name Operators with same type
	/// @{
	TVec3& operator=(const TVec3& b)
	{
		arr[0] = b.arr[0];
		arr[1] = b.arr[1];
		arr[2] = b.arr[2];
		return (*this);
	}

	TVec3 operator+(const TVec3& b) const
	{
		return TVec3(x() + b.x(), y() + b.y(), z() + b.z());
	}

	TVec3& operator+=(const TVec3& b)
	{
		x() += b.x();
		y() += b.y();
		z() += b.z();
		return (*this);
	}

	TVec3 operator-(const TVec3& b) const
	{
		return TVec3(x() - b.x(), y() - b.y(), z() - b.z());
	}

	TVec3& operator-=(const TVec3& b)
	{
		x() -= b.x();
		y() -= b.y();
		z() -= b.z();
		return (*this);
	}

	TVec3 operator*(const TVec3& b) const
	{
		return TVec3(x() * b.x(), y() * b.y(), z() * b.z());
	}

	TVec3& operator*=(const TVec3& b)
	{
		x() *= b.x();
		y() *= b.y();
		z() *= b.z();
		return (*this);
	}

	TVec3 operator/(const TVec3& b) const
	{
		return TVec3(x() / b.x(), y() / b.y(), z() / b.z());
	}

	TVec3& operator/=(const TVec3& b)
	{
		x() /= b.x();
		y() /= b.y();
		z() /= b.z();
		return (*this);
	}

	TVec3 operator-() const
	{
		return TVec3(-x(), -y(), -z());
	}

	Bool operator==(const TVec3& b) const
	{
		return isZero<T>(x() - b.x()) 
			&& isZero<T>(y() - b.y()) 
			&& isZero<T>(z() - b.z());
	}

	Bool operator!=(const TVec3& b) const
	{
		return !operator==(b);
	}

	Bool operator<(const TVec3& b) const
	{
		return x() < b.x() && y() < b.y() && z() < b.z();
	}

	Bool operator<=(const TVec3& b) const
	{
		return x() <= b.x() && y() <= b.y() && z() <= b.z();
	}

	Bool operator>(const TVec3& b) const
	{
		return x() > b.x() && y() > b.y() && z() > b.z();
	}

	Bool operator>=(const TVec3& b) const
	{
		return x() >= b.x() && y() >= b.y() && z() >= b.z();
	}
	/// @}

	/// @name Operators with T
	/// @{
	TVec3 operator+(const T f) const
	{
		return (*this) + TVec3(f);
	}

	TVec3& operator+=(const T f)
	{
		(*this) += TVec3(f);
		return (*this);
	}

	TVec3 operator-(const T f) const
	{
		return (*this) - TVec3(f);
	}

	TVec3& operator-=(const T f)
	{
		(*this) -= TVec3(f);
		return (*this);
	}

	TVec3 operator*(const T f) const
	{
		return (*this) * TVec3(f);
	}

	TVec3& operator*=(const T f)
	{
		(*this) *= TVec3(f);
		return (*this);
	}

	TVec3 operator/(const T f) const
	{
		return (*this) / TVec3(f);
	}

	TVec3& operator/=(const T f)
	{
		(*this) /= TVec3(f);
		return (*this);
	}
	/// @}

	/// @name Operators with other types
	/// @{
	TVec3 operator*(const TMat3<T>& m3) const
	{
		ANKI_ASSERT(0 && "TODO");
		return TVec3(0.0);
	}
	/// @}

	/// @name Other
	/// @{

	/// 3 muls, 2 adds
	T dot(const TVec3& b) const
	{
		return x() * b.x() + y() * b.y() + z() * b.z();
	}

	/// 6 muls, 3 adds
	TVec3 cross(const TVec3& b) const
	{
		return TVec3(y() * b.z() - z() * b.y(),
			z() * b.x() - x() * b.z(),
			x() * b.y() - y() * b.x());
	}

	T getLengthSquared() const
	{
		return x() * x() + y() * y() + z() * z();
	}

	T getLength() const
	{
		return sqrt(getLengthSquared());
	}

	T getDistanceSquared(const TVec3& b) const
	{
		return ((*this) - b).getLengthSquared();
	}

	void normalize()
	{
		(*this) /= getLength();
	}

	TVec3 getNormalized() const
	{
		return (*this) / getLength();
	}

	TVec3 getProjection(const TVec3& toThis) const
	{
		return toThis * ((*this).dot(toThis) / (toThis.dot(toThis)));
	}

	/// Returns q * this * q.Conjucated() aka returns a rotated this.
	/// 18 muls, 12 adds
	TVec3 getRotated(const TQuat<T>& q) const
	{
		ANKI_ASSERT(isZero(1.0 - q.getLength())); // Not normalized quat
		TVec3 qXyz(q);
		return 
			(*this) + qXyz.cross(qXyz.cross((*this)) + (*this) * q.w()) * 2.0;
	}

	void rotate(const TQuat<T>& q)
	{
		(*this) = getRotated(q);
	}

	/// Return lerp(this, v1, t)
	TVec3 lerp(const TVec3& v1, T t) const
	{
		return ((*this) * (1.0 - t)) + (v1 * t);
	}
	/// @}

	/// @name Transformations
	/// The faster way is by far the TMat4 * TVec3 or the
	/// getTransformed(const TVec3&, const TMat3&)
	/// @{
	TVec3 getTransformed(const TVec3& translate, const TMat3<T>& rotate,
		T scale) const
	{
		return (rotate * ((*this) * scale)) + translate;
	}

	void transform(const TVec3& translate, const TMat3<T>& rotate, T scale)
	{
		(*this) = getTransformed(translate, rotate, scale);
	}

	TVec3 getTransformed(const TVec3& translate, const TMat3<T>& rotate) const
	{
		return (rotate * (*this)) + translate;
	}

	void transform(const TVec3& translate, const TMat3<T>& rotate)
	{
		(*this) = getTransformed(translate, rotate);
	}

	TVec3 getTransformed(const TVec3& translate, const TQuat<T>& rotate,
		T scale) const
	{
		return ((*this) * scale).getRotated(rotate) + translate;
	}

	void transform(const TVec3& translate, const TQuat<T>& rotate, T scale)
	{
		(*this) = getTransformed(translate, rotate, scale);
	}

	TVec3 getTransformed(const TVec3& translate, const TQuat<T>& rotate) const
	{
		return getRotated(rotate) + translate;
	}

	void transform(const TVec3& translate, const TQuat<T>& rotate)
	{
		(*this) = getTransformed(translate, rotate);
	}

	/// 9 muls, 9 adds
	TVec3 getTransformed(const TMat4<T>& transform) const
	{
		return TVec3(
			transform(0, 0) * x() + transform(0, 1) * y() 
			+ transform(0, 2) * z() + transform(0, 3),
			transform(1, 0) * x() + transform(1, 1) * y() 
			+ transform(1, 2) * z() + transform(1, 3),
			transform(2, 0) * x() + transform(2, 1) * y() 
			+ transform(2, 2) * z() + transform(2, 3));
	}

	void transform(const TMat4<T>& transform)
	{
		(*this) = getTransformed(transform);
	}

	/// 12 muls, 9 adds
	TVec3 getTransformed(const TTransform<T>& transform) const
	{
		return (transform.getRotation() * ((*this) * transform.getScale())) 
			+ transform.getOrigin();
	}

	void transform(const TTransform<T>& transform)
	{
		(*this) = getTransformed(transform);
	}
	/// @}

	/// @name Friends
	/// @{
	friend TVec3 operator+(const T f, const TVec3& v)
	{
		return v + f;
	}

	friend TVec3 operator-(const T f, const TVec3& v)
	{
		return TVec3(f) - v;
	}

	friend TVec3 operator*(const T f, const TVec3& v)
	{
		return v * f;
	}

	friend TVec3 operator/(const T f, const TVec3& v)
	{
		return TVec3(f) / v;
	}

	friend std::ostream& operator<<(std::ostream& s, const TVec3& v)
	{
		s << v.x() << ' ' << v.y() << ' ' << v.z();
		return s;
	}
	/// @}

private:
	/// @name Data
	/// @{
	union
	{
		struct
		{
			T x, y, z;
		} vec;

		Array<T, 3> arr;
	};
	/// @}
};

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

	Vec3 getTransformed(const Transform& transform) const; ///< 12 muls, 9 adds
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

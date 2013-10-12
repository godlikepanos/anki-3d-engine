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
	/// @name Friends
	/// @{
	template<typename Y>
	friend TVec3<Y> operator+(const Y f, const TVec3<Y>& v);
	template<typename Y>
	friend TVec3<Y> operator-(const Y f, const TVec3<Y>& v);
	template<typename Y>
	friend TVec3<Y> operator*(const Y f, const TVec3<Y>& v);
	template<typename Y>
	friend TVec3<Y> operator/(const Y f, const TVec3<Y>& v);
	/// @}

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
		return *this;
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
		ANKI_ASSERT(isZero<T>(1.0 - q.getLength())); // Not normalized quat
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

	/// Transform the vector
	/// @param transform A transformation matrix
	///
	/// @note 9 muls, 9 adds
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

	std::string toString() const
	{
		return std::to_string(x()) + " " + std::to_string(y()) + " " 
			+ std::to_string(z());
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

template<typename T>
TVec3<T> operator+(const T f, const TVec3<T>& v)
{
	return v + f;
}

template<typename T>
TVec3<T> operator-(const T f, const TVec3<T>& v)
{
	return TVec3<T>(f) - v;
}

template<typename T>
TVec3<T> operator*(const T f, const TVec3<T>& v)
{
	return v * f;
}

template<typename T>
TVec3<T> operator/(const T f, const TVec3<T>& v)
{
	return TVec3<T>(f) / v;
}

/// F32 3D vector
typedef TVec3<F32> Vec3;
static_assert(sizeof(Vec3) == sizeof(F32) * 3, "Incorrect size");

/// Half float 3D vector
typedef TVec3<F16> HVec3;

/// 32bit signed integer 3D vector
typedef TVec3<I32> IVec3;

/// 32bit unsigned integer 3D vector
typedef TVec3<U32> UVec3;
/// @}

} // end namespace anki

#endif

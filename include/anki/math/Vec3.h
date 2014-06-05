// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_MATH_VEC3_H
#define ANKI_MATH_VEC3_H

#include "anki/math/CommonIncludes.h"
#include "anki/math/Vec.h"

namespace anki {

/// @addtogroup math
/// @{

/// 3D vector template. One of the most used classes
template<typename T>
class TVec3: public TVec<T, 3, Array<T, 3>, TVec3<T>>
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
	using Base = TVec<T, 3, Array<T, 3>, TVec3<T>>;

	using Base::x;
	using Base::y;
	using Base::z;
	using Base::operator*;

	/// @name Constructors
	/// @{
	explicit TVec3()
		: Base()
	{}

	TVec3(const TVec3& b)
		: Base(b)
	{}

	explicit TVec3(const T x_, const T y_, const T z_)
		: Base(x_, y_, z_)
	{}

	explicit TVec3(const T f)
		: Base(f)
	{}

	explicit TVec3(const T arr[])
		: Base(arr)
	{}

	explicit TVec3(const TVec2<T>& v, const T z_)
		: Base(v.x(), v.y(), z_)
	{}
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

	/// 6 muls, 3 adds
	TVec3 cross(const TVec3& b) const
	{
		return TVec3(y() * b.z() - z() * b.y(),
			z() * b.x() - x() * b.z(),
			x() * b.y() - y() * b.x());
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
		TVec3 qXyz(q.x(), q.y(), q.z());
		return 
			(*this) + qXyz.cross(qXyz.cross((*this)) + (*this) * q.w()) * 2.0;
	}

	void rotate(const TQuat<T>& q)
	{
		(*this) = getRotated(q);
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
	/// @}
};

/// @memberof TVec3
template<typename T>
TVec3<T> operator+(const T f, const TVec3<T>& v)
{
	return v + f;
}

/// @memberof TVec3
template<typename T>
TVec3<T> operator-(const T f, const TVec3<T>& v)
{
	return TVec3<T>(f) - v;
}

/// @memberof TVec3
template<typename T>
TVec3<T> operator*(const T f, const TVec3<T>& v)
{
	return v * f;
}

/// @memberof TVec3
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

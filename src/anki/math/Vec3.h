// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/math/CommonIncludes.h>
#include <anki/math/Vec.h>

namespace anki
{

/// @addtogroup math
/// @{

/// 3D vector template. One of the most used classes
template<typename T>
class TVec3 : public TVec<T, 3, Array<T, 3>, TVec3<T>>
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
	TVec3()
		: Base()
	{
	}

	TVec3(const TVec3& b)
		: Base(b)
	{
	}

	TVec3(const T x_, const T y_, const T z_)
		: Base(x_, y_, z_)
	{
	}

	explicit TVec3(const T f)
		: Base(f)
	{
	}

	explicit TVec3(const T arr[])
		: Base(arr)
	{
	}

	TVec3(const TVec2<T>& v, const T z_)
		: Base(v.x(), v.y(), z_)
	{
	}
	/// @}

	/// @name Other
	/// @{

	/// 6 muls, 3 adds
	TVec3 cross(const TVec3& b) const
	{
		return TVec3(y() * b.z() - z() * b.y(), z() * b.x() - x() * b.z(), x() * b.y() - y() * b.x());
	}

	TVec3 projectTo(const TVec3& toThis) const
	{
		return toThis * ((*this).dot(toThis) / (toThis.dot(toThis)));
	}

	TVec3 projectTo(const TVec3& rayOrigin, const TVec3& rayDir) const
	{
		const auto& a = *this;
		return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
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
using Vec3 = TVec3<F32>;
static_assert(sizeof(Vec3) == sizeof(F32) * 3, "Incorrect size");

/// Half float 3D vector
using HVec3 = TVec3<F16>;

/// 32bit signed integer 3D vector
using IVec3 = TVec3<I32>;

/// 32bit unsigned integer 3D vector
using UVec3 = TVec3<U32>;

/// 64bit float 3D vector
using DVec3 = TVec3<F64>;
/// @}

} // end namespace anki
